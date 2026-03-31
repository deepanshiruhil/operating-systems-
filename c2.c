#include    "utility.h"

pthread_cond_t cond_shm;
pthread_mutex_t mutex_shm;
int n2;
bool task_is_done = false;
bool can_run = false;

// This is our main task thread.
void* task(void* vargp) 
{
// If shared memory is not 1, put task thread to sleep until monitor thread signals to continue.
    pthread_mutex_lock(&mutex_shm);
    if(can_run == false) 
    {
        printf("[C2] PID #%d. Task thread sleeping.\n", getpid());
        fflush(stdout);
        pthread_cond_wait(&cond_shm, &mutex_shm);
    }
    pthread_mutex_unlock(&mutex_shm);

    FILE* file_handle = fopen((char*)vargp, "r");
    if(file_handle == NULL)
    {
        fprintf(stderr, "%s: Error at line %d\n", __FILE__, __LINE__);
        return NULL;
    }
    char* line_pointer = NULL;
    size_t zero = 0;

    for (int i = 1; i <= n2; i++) 
    {
        getline(&line_pointer, &zero, file_handle);
        printf("[C2] PID #%d. Read: %d.\n", getpid(), atoi(line_pointer));
// Putting task thread to sleep after each addition if necessary.
        pthread_mutex_lock(&mutex_shm);
        if(can_run == false) 
        {
            printf("[C2] PID #%d. Task thread sleeping.\n", getpid());
            fflush(stdout);
            pthread_cond_wait(&cond_shm, &mutex_shm);
        }
        pthread_mutex_unlock(&mutex_shm);
    }

	write_to_pipe("/tmp/c2_data", "DONE PRINTING", 14);
    task_is_done = true;

    return NULL;
}

//main also serves as the monitor thread. 
int main(int argc, char** argv)
{
	struct timeval start_c2, end_c2, total_c2;
    gettimeofday(&start_c2, NULL);
    n2 = atoi(argv[1]);
    pthread_mutex_init(&mutex_shm, NULL);
    pthread_cond_init(&cond_shm, NULL);
    pthread_t task_thread;
    pthread_create(&task_thread, NULL, task, argv[2]);

	clock_t t;
    double wait_time=0;
    while(true) 
    {
        // here again the mutex is around the shared memory pointer,
        // and it signals to the cond-wait that there could be a change in the block value.
        can_run = read_from_shared_memory("c2.c");
        while(can_run == false)
        {
        	t = clock();
            can_run = read_from_shared_memory("c2.c");
            usleep(1);
            t = clock() - t;
			double time_taken = ((double)t)/CLOCKS_PER_SEC;
			wait_time+=time_taken;
        }        
        pthread_mutex_lock(&mutex_shm);
        pthread_cond_signal(&cond_shm);
        pthread_mutex_unlock(&mutex_shm);
        if(task_is_done) 
        {
            break;
        }

    }

    pthread_join(task_thread, NULL);
    pthread_mutex_destroy(&mutex_shm);
    pthread_cond_destroy(&cond_shm);

    printf("[C2] PID #%d. Task complete!\n", getpid());
    gettimeofday(&end_c2, NULL);
    total_c2.tv_sec=end_c2.tv_sec - start_c2.tv_sec;
	total_c2.tv_usec=end_c2.tv_usec - start_c2.tv_usec;
	printf("WT FOR CHILD 2 :- %lf seconds\n", wait_time);			
	printf("TAT FOR CHILD 2 :- seconds : %ld\nmicro seconds : %ld\n", total_c2.tv_sec, total_c2.tv_usec); 
    return 0;
}
