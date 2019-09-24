#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int i;

int main(int ac, char **av)
{
    int pid;
    i = 1;
    if ((pid = fork()) == 0)
    {
        /* Dans le fils */
        printf("Je suis le fils, pid = %d\n", getpid());
        i++;
        printf("Fin du fils, i = %d !\n", i);
        exit(0);
    }
    else if (pid > 0)
    {
        /* Dans le pere */
        printf("Je suis le pere, pid = %d\n", getpid());
        sleep(1);
        //i++;
        printf("Fin du pere, i = %d !\n", i);
        exit(0);
    }
    else
    {
        /* Erreur */
        perror("fork");
        exit(1);
    }
}