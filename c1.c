#include    "utility.h"

pthread_cond_t cond_shm;
pthread_mutex_t mutex_shm;
int n1;
bool task_is_done = false;
bool can_run = false;

// This is our main task thread.
void* task(void* vargp) 
{
// If shared memory is not 1, put task thread to sleep until monitor thread signals to continue.
    pthread_mutex_lock(&mutex_shm);
    if(can_run == false) 
    {
        printf("[C1] PID #%d. Task thread sleeping.\n", getpid());
        fflush(stdout);
        pthread_cond_wait(&cond_shm, &mutex_shm);
    }
    pthread_mutex_unlock(&mutex_shm);

    unsigned long long int sum = 0;
    for (int i = 1; i <= n1; i++) 
    {
        sum += i;

// Putting task thread to sleep after each addition if necessary.
        pthread_mutex_lock(&mutex_shm);
        if(can_run == false) 
        {
            printf("[C1] PID #%d. Task thread sleeping.\n", getpid());
            fflush(stdout);
            pthread_cond_wait(&cond_shm, &mutex_shm);
        }
        pthread_mutex_unlock(&mutex_shm);
    }

        char numval[80];
        sprintf(numval, "%llu", sum);
        //uncomment this pipe line @Kevin to begin the writing
        write_to_pipe("/tmp/c1_data", numval, strlen(numval) + 1);
        
    fflush(stdout);
    task_is_done = true;

    return NULL;
}

//main also serves as the monitor thread. 
int main(int argc, char** argv)
{
    struct timeval start_c1, end_c1, total_c1;
    gettimeofday(&start_c1, NULL);

    n1 = atoi(argv[1]);
    pthread_mutex_init(&mutex_shm, NULL);
    pthread_cond_init(&cond_shm, NULL);
    pthread_t task_thread;
    pthread_create(&task_thread, NULL, task, NULL);

    clock_t t;
    double wait_time=0;
    while(true) 
    {
        // here again the mutex is around the shared memory pointer,
        // and it signals to the cond-wait that there could be a change in the block value.
        can_run = read_from_shared_memory("c1.c");
        while(can_run == false)
        {
        	t = clock();
            can_run = read_from_shared_memory("c1.c");
            usleep(1);
			t = clock() - t;
			double time_taken = ((double)t) / CLOCKS_PER_SEC;
			wait_time += time_taken;
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

    printf("[C1] PID #%d. Task complete!\n", getpid());
    gettimeofday(&end_c1, NULL);
    total_c1.tv_sec=end_c1.tv_sec - start_c1.tv_sec;
	total_c1.tv_usec=end_c1.tv_usec - start_c1.tv_usec;
	printf("WT FOR CHILD 1 :- %lf seconds\n", wait_time);			
	printf("TAT FOR CHILD 1 :- seconds : %ld\nmicro seconds : %ld\n", total_c1.tv_sec, total_c1.tv_usec); 			
    return 0;
}
