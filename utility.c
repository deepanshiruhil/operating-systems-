#include    "utility.h"

#define     FAILURE     -1

int get_shared_block(char *filename, int size) 
{
//request a key
//the key is linked to a filename, so that other programs can access it.
//rhe file MUST exist+ also be accessible.
    key_t key = ftok(filename, 0);
    if (key == FAILURE) 
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);
    }
//get shared memory block or create it if it doesn't exist.
    return shmget(key, size, 0644 | IPC_CREAT);
}

bool* attach_memory_block(char* filename, int size) 
{
    int shared_block_id = get_shared_block(filename, size);
    bool* result;
    if (shared_block_id == FAILURE)
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);    
        return NULL;
    }    
    //map the shared block into this process's memory and get a pointer to it.
    result = shmat(shared_block_id, NULL, 0);
    if (result == (void*)FAILURE) 
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);        
        return NULL;
    }
    return result;
}

void detach_memory_block(bool* block)
{
    if(shmdt(block) == FAILURE)
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);    
        return;
    }
    return;
}

void destroy_memory_block(char *filename)
{
    int shared_block_id = get_shared_block(filename, 0);
    if((shared_block_id == FAILURE) || (shmctl(shared_block_id,IPC_RMID,NULL) == FAILURE))
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);            
        return;
    }
    return;
}

bool read_from_shared_memory(char* filename) 
{
    bool* block = attach_memory_block(filename, BLOCK_SIZE);
    if (block == NULL) 
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);
        return NULL;
    }
    bool retval = block[0];
    detach_memory_block(block);
    return retval;
}

void write_to_shared_memory(char* filename, bool status) 
{
    bool* block = attach_memory_block(filename, BLOCK_SIZE);
    if (block == NULL) 
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);
        return;
    }

    block[0] = status;
    detach_memory_block(block);
    return;
}

void write_to_pipe(char* filename, char* str, int str_size)
{
    	int fd;
    	char * myfifo = "/tmp/c1Data";
    	mkfifo(filename, 0666);
        fd = open(filename, O_WRONLY);
        write(fd, str, str_size);
        close(fd);
}

void* read_from_pipe(void* filename)
{

    char str[32];
    int fd = open((char*)filename, O_RDONLY);
    read(fd, str, sizeof(str));
    printf("Pipe spits out %s\n", str);
    close(fd);
    return NULL;
}
