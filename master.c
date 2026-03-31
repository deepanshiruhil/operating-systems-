#include    "utility.h"

bool        subprocessed_exited = false;
char*       pipe_filenames[] = {"/tmp/c1_data", "/tmp/c2_data", "/tmp/c3_data"};
int         children[3];

void* pied_piper(void* vargp)
{
    int status;
    int pid;
    for(int i = 0; i < 3; i++)
    {
        pid = wait(&status);
        if(pid == children[0])
        {
            children[0] = -1;
        }
        else if(pid == children[1])
        {
            children[1] = -1;
        }
        else if(pid == children[2])
        {
            children[2] = -1;
        }                
        printf("[M] Child #%d has exited.\n", pid);
    }
    subprocessed_exited = true;
    return NULL;
}

int main(int argc, char** argv)
{
    clock_t t;
    t = clock();
    double time_taken = ((double)t) / CLOCKS_PER_SEC;
    printf("Master thread starts at %f seconds \n", time_taken);
	char n1[16], n2[16], c2_file[32], n3[16], c3_file[32];
	char str[80];
    int time_quantum;
	printf("N1: ");
	scanf("%s", n1);
	printf("N2: ");
	scanf("%s", n2);
    printf("Filename for C2: ");
    scanf("%s", c2_file);
	printf("N3: ");
	scanf("%s", n3);
    printf("Filename for C3: ");
    scanf("%s", c3_file);    	
    printf("FCFS/RR [1 for FCFS, 2 for RR]: ");
    scanf("%d", &time_quantum);
    if(time_quantum == 1)
    {
        printf("Stub. Unimplemented. Shoo. Go away.\n");
        exit(1);
    }
    printf("Time Quantum for RR in microseconds: ");
    scanf("%d", &time_quantum);

    pthread_t pied_piper_thread, c1_output_thread, c2_output_thread, c3_output_thread;
    pthread_create(&c1_output_thread, NULL, read_from_pipe, pipe_filenames[0]);
    pthread_create(&c2_output_thread, NULL, read_from_pipe, pipe_filenames[1]);
    pthread_create(&c3_output_thread, NULL, read_from_pipe, pipe_filenames[2]);       

    write_to_shared_memory("c1.c", false);
    write_to_shared_memory("c2.c", false);
    write_to_shared_memory("c3.c", false);      
	t = clock();
    time_taken = ((double)t) / CLOCKS_PER_SEC;
    printf("Child 1 starts at %f seconds \n", time_taken);
	children[0] = fork();
 
    if (children[0] == 0)
    {
// spawning C1
        execlp("./c1", "./c1", n1, NULL);
    }
    else
    {
    t = clock();
    time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("Child 2 starts at %f seconds \n", time_taken);
    	
        children[1] = fork();

        if (children[1] == 0)
        {
// spawning C2
            execlp("./c2", "./c2", n2, c2_file, NULL);
        }
        else
        {
        t = clock();
    	time_taken = ((double)t)/CLOCKS_PER_SEC;
    	printf("Child 3 starts at %f seconds \n", time_taken);
            children[2] = fork();
        
            if (children[2] == 0)
            {
// spawning C3
                execlp("./c3", "./c3", n3, c3_file, NULL);
            }  
            else
            {
// parent can now setup scheduling.
                pthread_create(&pied_piper_thread, NULL, pied_piper, NULL);

                for(uint64_t i = 0; subprocessed_exited == false; i++)
                {
                    if(children[(i % 3)] == -1)
                    {
                        continue;
                    }
                    write_to_shared_memory("c1.c", ((i % 3) == 0));
                    write_to_shared_memory("c2.c", ((i % 3) == 1));
                    write_to_shared_memory("c3.c", ((i % 3) == 2));   
                    usleep(time_quantum);                      
                }

                pthread_join(pied_piper_thread, NULL);
                pthread_join(c1_output_thread, NULL);
                pthread_join(c2_output_thread, NULL);
                pthread_join(c3_output_thread, NULL);                                
            } 
        }
    }
    return 0;
}
