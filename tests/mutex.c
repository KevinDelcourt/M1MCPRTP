#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t my_mutex;
static int tab[5];
int c;

void *inc_c_process(void *arg)
{
    pthread_mutex_lock(&my_mutex);
    for (int j = 0; j < 1000000; j++)
    {
        c++;
        //printf("Thread %s: %d\n", (char *)arg, i);
    }
    pthread_mutex_unlock(&my_mutex);
    pthread_exit(0);
}

int main(int ac, char **av)
{
    pthread_t th1, th2;
    void *ret;
    pthread_mutex_init(&my_mutex, NULL);
    if (pthread_create(&th1, NULL, inc_c_process, NULL) < 0)
    {
        fprintf(stderr, "pthread_create error for thread 1\n");
        exit(1);
    }
    if (pthread_create(&th2, NULL, inc_c_process, NULL) < 0)
    {
        fprintf(stderr, "pthread_create error for thread 2\n");
        exit(1);
    }
    (void)pthread_join(th1, &ret);
    (void)pthread_join(th2, &ret);
    printf("Valeur finale: %d\n", c);
}