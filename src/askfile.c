#include "askfile.h"

#include <string.h>

#define log_oom() fprintf(stderr, "Out of memory at %s:%d (%s)\n", __FILE__, __LINE__, __func__)


askfile_t* askfile_new() {
    askfile_t* askfile = malloc(sizeof(askfile_t));
    if (!askfile) {
        log_oom();
        return NULL;
    }
    
    askfile->message = NULL;
    askfile->socket_path = NULL;
    askfile->pid = 0;
    askfile->not_after = 0;
    return askfile;
}


// for internal use
static askfile_t* askfile_load_dict_into(dictionary* dict, askfile_t* askfile) {
    char* message = iniparser_getstring(dict, "Ask:Message", NULL);
    if (message != NULL) {
        message = strdup(message);
        if (message == NULL) {
            log_oom();
            return NULL;
        }
    }
    
    char* socket_path = iniparser_getstring(dict, "Ask:Socket", NULL);
    if (socket_path != NULL) {
        socket_path = strdup(socket_path);
        if (socket_path == NULL) {
            log_oom();
            free(message);
            return NULL;
        }
    }
    
    int pid = iniparser_getint(dict, "Ask:PID", 0);
    
    uint64_t not_after = 0;
    char* not_after_str = iniparser_getstring(dict, "Ask:NotAfter", NULL);
    if (not_after_str) {
        not_after = strtoull(not_after_str, NULL, 0);
    }
    
    askfile->message = message;
    askfile->socket_path = socket_path;
    askfile->pid = pid;
    askfile->not_after = not_after;
    
    return askfile;
}


// for internal use
static askfile_t* askfile_load_dict(dictionary* dict) {
    askfile_t* askfile = askfile_new();
    if (askfile == NULL) {
        return NULL;
    }
    
    if (askfile_load_dict_into(dict, askfile) == NULL) {
        askfile_free(askfile);
        return NULL;
    }
    return askfile;
}


askfile_t* askfile_load(const char* file_path) {
    dictionary* dict = iniparser_load(file_path);
    if (dict == NULL) {
        return NULL;
    }
    
    askfile_t* askfile = askfile_load_dict(dict);    
    iniparser_freedict(dict); 
    return askfile;
}


void askfile_free(askfile_t* askfile) {
    if (askfile == NULL) {
        return;
    }
    
    free(askfile->message);
    askfile->message = NULL;
    
    free(askfile->socket_path);
    askfile->socket_path = NULL;
}
