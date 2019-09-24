#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int i;

void *my_thread_process(void *arg)
{
    for (int j = 0; j < 1000000; j++)
    {
        i++;
        //printf("Thread %s: %d\n", (char *)arg, i);
    }
    pthread_exit(0);
}

int main(int ac, char **av)
{
    pthread_t th1, th2;
    void *ret;
    if (pthread_create(&th1, NULL, my_thread_process, "1") < 0)
    {
        fprintf(stderr, "pthread_create error for thread 1\n");
        exit(1);
    }
    if (pthread_create(&th2, NULL, my_thread_process, "2") < 0)
    {
        fprintf(stderr, "pthread_create error for thread 2\n");
        exit(1);
    }
    (void)pthread_join(th1, &ret);
    (void)pthread_join(th2, &ret);
    printf("Valeur de i finale: %d\n", i); //Devrait être 2M mais no car pas d'exclusion mutuelle sur i
}