/* UPS
   Lecteurs-redacteurs partageant un fichier dans lequel
   les premiers lisent (a plusieurs) et les seconds ecrivent
   (en exclusion mutuelle)
   @Author : Kévin Delcourt
*/
#include <stdio.h>
#include <string.h> /* strerror */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include "LRMonitor.h"

LRMonitor *moniteurLR;

#define NB_MAX_LECTEURS 10
#define NB_MAX_REDACTEURS 10
#define NB_LECTURES 5
#define NB_ECRITURES 6

int fd;

// Descriptif partage par tous les threads
// => les lecteurs partagent ce descriptif et donc la position
// de lecture dans le fichier
// Si on veut des lectures ind�pendantes, ce descriptif doit �tre local
// au thread qui fera sa propre ouverture et sa propre fermeture du fichier

/*---------------------------------------------------------------------*/
void thdErreur(int codeErr, char *msgErr, void *codeArret)
{
    /*---------------------------------------------------------------------*/
    fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
    pthread_exit(codeArret);
}

/*---------------------------------------------------------------------*/
void *threadLecteur(void *arg)
{
    /*---------------------------------------------------------------------*/
    int nbLus, i;
    int *monNum = (int *)arg;
    char unCar;

    for (i = 0; i < NB_LECTURES; i++)
    {
        printf("Lecteur %d: \033[0;35mJE VEUX LIRE\033[0m \n", *monNum);
        moniteurLR->debutLire();

        // Se positionner au debut de fichier
        if (lseek(fd, 0, SEEK_SET) < 0)
            thdErreur(errno, "Lseek debut fichier", monNum);
        printf("Lecteur %d: \033[0;36mLecture\033[0m \n", *monNum);
        // Lire caractere par caractere jusqu'� fin fichier
        // en affichant les caracteres lus
        do
        {
            if ((nbLus = read(fd, &unCar, sizeof(char))) < 0)
                thdErreur(errno, "Read fichier lecture", monNum);

            // Temporiser eventuellement avec usleep() pour perdre du temps
        } while (nbLus > 0);

        moniteurLR->finLire();
    }
    printf("Lecteur %d, j'ai fini mon execution \n", *monNum);
    pthread_exit(NULL);
}

/*---------------------------------------------------------------------*/
void *threadRedacteur(void *arg)
{
    /*---------------------------------------------------------------------*/
    int i, nbEcrits;
    int *monNum = (int *)arg;
    char monCar = *monNum + 65;

    for (i = 0; i < NB_ECRITURES; i++)
    {
        printf("Redacteur %d: \033[0;33mJE VEUX ECRIRE\033[0m \n", *monNum);
        moniteurLR->debutEcrire();

        // S'appreter a ecrire en fin de fichier
        if (lseek(fd, 0, SEEK_END) < 0)
            thdErreur(errno, "Lseek fin fichier", monNum);

        if ((nbEcrits = write(fd, &monCar, 1)) < 0)
            thdErreur(errno, "Write fichier", monNum);
        printf("Redacteur %d: \033[0;34mEcriture\033[0m de %c \n", *monNum, monCar);

        moniteurLR->finEcrire();
    }
    printf("Redacteur %d, j'ai fini mon execution \n", *monNum);
    pthread_exit(NULL);
}

/*---------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    /*---------------------------------------------------------------------*/
    pthread_t lesLecteurs[NB_MAX_LECTEURS];
    pthread_t lesRedacteurs[NB_MAX_REDACTEURS];
    int rangLecteurs[NB_MAX_LECTEURS];
    int rangRedacteurs[NB_MAX_REDACTEURS];
    int nbLecteurs, nbRedacteurs;
    int i, etat;
    moniteurLR = new LRMonitor();
    if (argc != 3)
    {
        printf("Usage: %s <Nb lecteurs <= %d> <Nb redacteurs <= %d> \n",
               argv[0], NB_MAX_LECTEURS, NB_MAX_REDACTEURS);
        exit(1);
    }

    nbLecteurs = atoi(argv[1]);
    if (nbLecteurs > NB_MAX_LECTEURS)
        nbLecteurs = NB_MAX_LECTEURS;

    nbRedacteurs = atoi(argv[2]);
    if (nbRedacteurs > NB_MAX_REDACTEURS)
        nbRedacteurs = NB_MAX_REDACTEURS;

    /* Creation du fichier partage */
    if ((fd = open("LectRed_shared", O_RDWR | O_CREAT, 0666)) < 0)
    {
        printf("Erreur ouverture fichier %s\n", strerror(errno));
        exit(1);
    }

    /*  A completer pour assurer la synchronisation souhaitee */

    /* Lancement des threads lecteurs et redacteurs */
    for (i = 0; i < nbLecteurs; i++)
    {
        rangLecteurs[i] = i;
        if ((etat = pthread_create(&lesLecteurs[i], NULL,
                                   threadLecteur, &rangLecteurs[i])) != 0)
            thdErreur(etat, "Creation lecteurs", NULL);
    }

    for (i = 0; i < nbRedacteurs; i++)
    {
        rangRedacteurs[i] = i;
        if ((etat = pthread_create(&lesRedacteurs[i], NULL,
                                   threadRedacteur, &rangRedacteurs[i])) != 0)
            thdErreur(etat, "Creation redacteurs", NULL);
    }

    /* Attente de la fin des threads */
    for (i = 0; i < nbLecteurs; i++)
        if ((etat = pthread_join(lesLecteurs[i], NULL)) != 0)
            thdErreur(etat, "Join lecteurs", NULL);

    for (i = 0; i < nbRedacteurs; i++)
        if ((etat = pthread_join(lesRedacteurs[i], NULL)) != 0)
            thdErreur(etat, "Join lecteurs", NULL);

    /* Fermeture du fichier partage */
    close(fd);

    moniteurLR->saveActivities();
    printf("\nFin de l'execution du main \n");
}
