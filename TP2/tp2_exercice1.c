/* UPS/IRIT
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

#define NB_MAX_LECTEURS 10
#define NB_MAX_REDACTEURS 10
#define NB_LECTURES 5
#define NB_ECRITURES 6
#define LIBRE 0
#define LECTURE_EN_COURS 1
#define ECRITURE_EN_COURS 2
/* A compl�ter pour assurer la synchronisation voulue */

typedef struct
{
    int fd;
    int etatFichier;
    int nbLecteursEnCours;
    int nbRedacteursEnAttente;
} RessourceCritique;

RessourceCritique resCritique;

pthread_cond_t condRedacteurs = PTHREAD_COND_INITIALIZER;
pthread_cond_t condLecteurs = PTHREAD_COND_INITIALIZER;
pthread_mutex_t exclusionMutuelleMoniteur = PTHREAD_MUTEX_INITIALIZER;
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
void debutLecture(int monNum)
{
    int etat;
    while (resCritique.nbRedacteursEnAttente > 0 || resCritique.etatFichier == ECRITURE_EN_COURS)
    {
#ifdef VERBOSE
        printf("\tLecteur %d: \033[0;31mEn attente\033[0m\n", monNum);
#endif
        if (0 != (etat = pthread_cond_wait(&condLecteurs, &exclusionMutuelleMoniteur)))
            thdErreur(etat, "Wait condition lecteur", NULL);
#ifdef VERBOSE
        printf("\tLecteur %d: \033[0;32mRéveil\033[0m\n", monNum);
#endif
    }
    resCritique.etatFichier = LECTURE_EN_COURS;
    resCritique.nbLecteursEnCours++;
    if (0 != (etat = pthread_cond_signal(&condLecteurs)))
        thdErreur(etat, "Signal condition lecteur", NULL);
#ifdef VERBOSE
    printf("\tLecteur %d: \033[0;32mDébut lecture\033[0m\n", monNum);
#endif
}

/*---------------------------------------------------------------------*/
void finLecture(int monNum)
{
    int etat;
    resCritique.nbLecteursEnCours--;
    if (resCritique.nbLecteursEnCours == 0)
    {
        resCritique.etatFichier = LIBRE;
        if (0 != (etat = pthread_cond_signal(&condRedacteurs)))
            thdErreur(etat, "Signal condition redacteur", NULL);
    }
#ifdef VERBOSE
    printf("\tLecteur %d: \033[0;34mFin lecture\033[0m\n", monNum);
#endif
}

/*---------------------------------------------------------------------*/
void debutEcriture(int monNum)
{
    int etat;
    while (resCritique.etatFichier != LIBRE)
    {
#ifdef VERBOSE
        printf("\tRedacteur %d: \033[0;31mEn attente\033[0m\n", monNum);
#endif
        resCritique.nbRedacteursEnAttente++;
        if (0 != (etat = pthread_cond_wait(&condRedacteurs, &exclusionMutuelleMoniteur)))
            thdErreur(etat, "Wait condition redacteur", NULL);
        resCritique.nbRedacteursEnAttente--;
#ifdef VERBOSE
        printf("\tRedacteur %d: \033[0;32mRéveil\033[0m\n", monNum);
#endif
    }
    resCritique.etatFichier = ECRITURE_EN_COURS;
#ifdef VERBOSE
    printf("\tRedacteur %d: \033[0;36mDébut écriture\033[0m\n", monNum);
#endif
}

void finEcriture(int monNum)
{
    int etat;
    resCritique.etatFichier = LIBRE;
    if (resCritique.nbRedacteursEnAttente > 0)
    {
        if (0 != (etat = pthread_cond_signal(&condRedacteurs)))
            thdErreur(etat, "Signal condition redacteur", NULL);
    }
    else
    {
        if (0 != (etat = pthread_cond_signal(&condLecteurs)))
            thdErreur(etat, "Signal condition lecteur", NULL);
    }
#ifdef VERBOSE
    printf("\tRedacteur %d: \033[0;35mFin écriture\033[0m\n", monNum);
#endif
}

/*---------------------------------------------------------------------*/
void *threadLecteur(void *arg)
{
    /*---------------------------------------------------------------------*/
    int nbLus, i, etat;
    int *monNum = (int *)arg;
    char unCar;

    for (i = 0; i < NB_LECTURES; i++)
    {
        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", NULL);

        debutLecture(*monNum);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", NULL);

        // Se positionner au debut de fichier
        if (lseek(resCritique.fd, 0, SEEK_SET) < 0)
            thdErreur(errno, "Lseek debut fichier", monNum);
#ifndef VERBOSE
        printf("Lecteur %d: Lecture de :\n", *monNum);
#endif
        // Lire caractere par caractere jusqu'� fin fichier
        // en affichant les caracteres lus
        do
        {
            if ((nbLus = read(resCritique.fd, &unCar, sizeof(char))) < 0)
                thdErreur(errno, "Read fichier lecture", monNum);
#ifndef VERBOSE
            if (nbLus > 0)
                printf("%c", unCar);
#endif

            // Temporiser eventuellement avec usleep() pour perdre du temps
        } while (nbLus > 0);
#ifndef VERBOSE
        printf("\n");
#endif
        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", NULL);

        finLecture(*monNum);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", NULL);
    }
    printf("Lecteur %d, j'ai fini mon execution \n", *monNum);
    pthread_exit(NULL);
}

/*---------------------------------------------------------------------*/
void *threadRedacteur(void *arg)
{
    /*---------------------------------------------------------------------*/
    int i, nbEcrits, etat;
    int *monNum = (int *)arg;
    char monCar = *monNum + 65;

    for (i = 0; i < NB_ECRITURES; i++)
    {
        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", NULL);

        debutEcriture(*monNum);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", NULL);

        // S'appreter a ecrire en fin de fichier
        if (lseek(resCritique.fd, 0, SEEK_END) < 0)
            thdErreur(errno, "Lseek fin fichier", monNum);

        if ((nbEcrits = write(resCritique.fd, &monCar, 1)) < 0)
            thdErreur(errno, "Write fichier", monNum);
#ifndef VERBOSE
        printf("Redacteur %d: Ecriture de %c \n", *monNum, monCar);
#endif

        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", NULL);

        finEcriture(*monNum);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", NULL);
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
    resCritique.etatFichier = LIBRE;
    resCritique.nbLecteursEnCours = 0;
    resCritique.nbRedacteursEnAttente = 0;

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
    if ((resCritique.fd = open("LectRed_shared", O_RDWR | O_CREAT, 0666)) < 0)
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

    if (0 != (etat = pthread_mutex_destroy(&exclusionMutuelleMoniteur)))
        thdErreur(etat, "Destroy mutex global", NULL);

    if (0 != (etat = pthread_cond_destroy(&condRedacteurs)))
        thdErreur(etat, "Destroy condition redacteurs", NULL);

    if (0 != (etat = pthread_cond_destroy(&condLecteurs)))
        thdErreur(etat, "Destroy condition lecteurs", NULL);

    /* Fermeture du fichier partage */
    close(resCritique.fd);

    printf("\nFin de l'execution du main \n");
}
