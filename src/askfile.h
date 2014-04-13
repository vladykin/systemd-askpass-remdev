#ifndef ASKFILE_H
#define ASKFILE_H

#include <inttypes.h>

#include <iniparser.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char* message;
    char* socket_path;
    int pid;
    uint64_t not_after;
} askfile_t;


askfile_t* askfile_new();


askfile_t* askfile_load(const char* file_path);


void askfile_free(askfile_t* askfile);


#ifdef __cplusplus
}
#endif

#endif /* ASKFILE_H */
