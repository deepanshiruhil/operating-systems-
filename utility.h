#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdbool.h>
#include    <stdint.h>
#include    <limits.h>
#include    <time.h>
#include    <errno.h>
#include    <signal.h>

#include    <fcntl.h>
#include    <unistd.h>
#include    <pthread.h>
#include    <sys/stat.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <sys/time.h>
#include    <sys/ipc.h>
#include    <sys/shm.h>

#define     BLOCK_SIZE  1

int get_shared_block(char* filename, int size);
bool* attach_memory_block(char* filename, int size);
void detach_memory_block(bool* block);
void destroy_memory_block(char* filename);
bool read_from_shared_memory(char* filename);
void write_to_shared_memory(char* filename, bool status);
void write_to_pipe(char* filename, char* str, int str_size);
void* read_from_pipe(void* filename);
