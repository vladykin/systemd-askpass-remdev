#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "askfile.h"


#define ASK_PASSWORD_DIR "/run/systemd/ask-password"
#define MOUNT_DIR "/run/systemd/ask-password-mnt"

static char* password_device = NULL;
static char* password_device_fs = NULL;
static char* password_file = NULL;


static int read_password(char* buf, int bufsize) {
    int rc = mkdir(MOUNT_DIR, 0700);

    rc = mount(password_device, MOUNT_DIR, password_device_fs, MS_NOATIME | MS_RDONLY, NULL);
    if (rc != 0) {
        fprintf(stderr, "Mount failed: %d\n", errno);
        return 0;
    }

    char* filename = malloc(strlen(MOUNT_DIR) + 1 + strlen(password_file) + 1);
    filename[0] = '\0';
    strcat(filename, MOUNT_DIR "/");
    strcat(filename, password_file);

    struct stat st;
    stat(filename, &st);

    if (bufsize < st.st_size) {
        fprintf(stderr, "Password file is too large\n");
        return 0;
    }

    buf[0] = '+';
    FILE* fd = fopen(filename, "rb");
    int readsize = fread(buf + 1, sizeof (char), st.st_size, fd);
    if (readsize < 0) {
        fprintf(stderr, "Read failed: %d\n", errno);
        return 0;
    }
    fclose(fd);
    buf[readsize + 1] = '\0';

    rc = umount(MOUNT_DIR);
    if (rc != 0) {
        fprintf(stderr, "Umount failed: %d\n", errno);
        return 0;
    }

    return readsize + 2;
}

static void ask_password(const char *filename) {

    askfile_t* askfile = askfile_load(filename);

    printf("Message:  %s\n", askfile->message);
    printf("Socket:   %s\n", askfile->socket_path);
    printf("NotAfter: %" PRIu64 "\n", askfile->not_after);
    printf("PID:      %d\n", askfile->pid);


    int bufsize = 10 * 1024;
    char* buf = malloc(bufsize);
    int passwordsize = read_password(buf, bufsize);
    if (!passwordsize) {
        fprintf(stderr, "Password could not be read\n");
        return;
    }

    printf("Packet: %s\n", buf);

    if (access(askfile->socket_path, W_OK) < 0) {
        fprintf(stderr, "No access to socket\n");
        return;
    }


    int socket_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);

    union {
        struct sockaddr sa;
        struct sockaddr_un un;
    } sa;
    sa.un.sun_family = AF_UNIX;
    strncpy(sa.un.sun_path, askfile->socket_path, sizeof (sa.un.sun_path));

    if (sendto(socket_fd, buf, passwordsize, MSG_NOSIGNAL, &sa.sa, offsetof(struct sockaddr_un, sun_path) + strlen(askfile->socket_path)) < 0) {
        fprintf(stderr, "Failed to send\n");
        return;
    }

    free(buf);

    close(socket_fd);
}

static int ask_passwords(char* askdir) {
    DIR *d;
    struct dirent *de;
    int r = 0;

    if (!(d = opendir(askdir))) {
        if (errno == ENOENT)
            return 0;

        fprintf(stderr, "opendir(): %d\n", errno);
        return -errno;
    }

    while ((de = readdir(d))) {
        printf("Found %s\n", de->d_name);

        if (de->d_type != DT_REG) {
            printf("Skipping: not a regular file\n");
            continue;
        }

        if (strstr(de->d_name, "ask.") != de->d_name) {
            printf("Skipping: name does not match\n");
            continue;
        }

        char* askfile = malloc(strlen(askdir) + strlen(de->d_name) + 2);
        memcpy(askfile, askdir, strlen(askdir));
        askfile[strlen(askdir)] = '/';
        memcpy(askfile + strlen(askdir) + 1, de->d_name, strlen(de->d_name));
        askfile[strlen(askdir) + strlen(de->d_name) + 1] = '\0';

        ask_password(askfile);

        free(askfile);
    }

    if (d)
        closedir(d);

    return r;
}


int main(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"dev", required_argument, 0, 'd'},
        {"fs", required_argument, 0, 't'},
        {"file", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    while (1) {

        int option_index = 0;

        int c = getopt_long(argc, argv, "d:t:f:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'd':
                password_device = strdup(optarg);
                break;

            case 't':
                password_device_fs = strdup(optarg);
                break;

            case 'f':
                password_file = strdup(optarg);
                break;

            case '?':
            default:
                return 1;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "No trailing arguments expected\n");
        return 1;
    }
    
    if (password_device == NULL) {
        fprintf(stderr, "Mandatory --dev parameter not specified\n");
        return 1;
    }

    if (password_device_fs == NULL) {
        fprintf(stderr, "Mandatory --fs parameter not specified\n");
        return 1;
    }

    if (password_file == NULL) {
        fprintf(stderr, "Mandatory --file parameter not specified\n");
        return 1;
    }

    return ask_passwords(ASK_PASSWORD_DIR);
}

