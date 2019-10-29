// La classe prioritaire passe en cas de conflit d'acces
// Option de compilation :
// -DRETARDER_PRIO_HAUTE : pour que les prios hautes demandent apres les basses
// -DDELAY : pour perdre du temps dans l'isoloir
//
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "IsoloirMonitor.h"

#define NB_ELECTEURS_MAX 40
#define NB_ISOLOIRS_MAX 10
#define PRIO_BASSE 1
#define PRIO_HAUTE 0

IsoloirMonitor *moniteurIsoloir;

/* codeErr : code retournee par une primitive
 * msgErr  : message d'erreur personnalise
 * valErr  : valeur retournee par le thread
 */
void thdErreur(const char *msgErr, int codeErr, int valeurErr)
{
    int *retour = (int *)malloc(sizeof(int));
    *retour = valeurErr;
    fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
    pthread_exit(retour);
}

//------------------------------------------------
typedef struct
{
    int monNum;
    int maPrio;
} Params;

//------------------------------------------------
// Completer avec les appels aux operations du moniteur
// pour assurer la synchronisation des acces aux isoloirs
//------------------------------------------------
void *thdElecteur(void *arg)
{
    Params *param = (Params *)arg;
    time_t t;

    //srand(pthread_self());

#ifdef RETARDER_PRIO_HAUTE
    if (param->maPrio == PRIO_HAUTE)
        usleep(rand() % 1000 * (param->maPrio + 1));
#endif

    char car = (param->maPrio == PRIO_BASSE) ? 'B' : 'H';

    printf("[%X - n:%d - p:%c] Je me presente pour voter a %s", pthread_self(), param->monNum, car, (time(&t), ctime(&t)));
    moniteurIsoloir->entrerIsoloir(param->maPrio);
    printf("[%X - n:%d - p:%c] Je prepare mon enveloppe dans l'isoloir a %s", pthread_self(), param->monNum, car, (time(&t), ctime(&t)));
#ifdef DELAY
    usleep(rand() % 100000 * (param->maPrio + 1));
#endif
    printf("[%X - n:%d - p:%c] Je finis a %s", pthread_self(), param->monNum, car, (time(&t), ctime(&t)));
    moniteurIsoloir->sortirIsoloir();
    pthread_exit(NULL);
}

//------------------------------------------------
int main(int argc, char *argv[])
{
    int nbElecteursPrioHaute, nbElecteursPrioBasse, nbIsoloirs, nbPriorites, i, etat, *cr;
    pthread_t idElecteurs[NB_ELECTEURS_MAX];
    Params param[NB_ELECTEURS_MAX];

    if (argc != 4)
    {
        printf("Usage : %s NbIsoloirs NbElecteursPrioHaute NbElecteursPrioBasse\n", argv[0]);
        exit(1);
    }
    nbIsoloirs = atoi(argv[1]);
    nbElecteursPrioHaute = atoi(argv[2]);
    nbElecteursPrioBasse = atoi(argv[3]);
    if (nbElecteursPrioHaute + nbElecteursPrioBasse > NB_ELECTEURS_MAX)
    {
        nbElecteursPrioHaute = NB_ELECTEURS_MAX / 2;
        nbElecteursPrioBasse = NB_ELECTEURS_MAX / 2;
    }
    if (nbIsoloirs > NB_ISOLOIRS_MAX)
    {
        nbIsoloirs = NB_ISOLOIRS_MAX;
    }
    nbPriorites = 2;
    moniteurIsoloir = new IsoloirMonitor(nbIsoloirs);
    for (i = 0; i < nbElecteursPrioHaute; i++)
    {
        param[i].monNum = i;
        param[i].maPrio = PRIO_HAUTE; //(i % 2);
        etat = pthread_create(&idElecteurs[i], NULL, thdElecteur, &param[i]);
        if (etat != 0)
            thdErreur("Creation thds prio haute", etat, 7);
    }

    for (i = 0; i < nbElecteursPrioBasse; i++)
    {
        param[i + nbElecteursPrioHaute].monNum = i + nbElecteursPrioHaute;
        param[i + nbElecteursPrioHaute].maPrio = PRIO_BASSE; //(i % 2);
        etat = pthread_create(&idElecteurs[i + nbElecteursPrioHaute], NULL, thdElecteur, &param[i + nbElecteursPrioHaute]);
        if (etat != 0)
            thdErreur("Creation thds prio basse", etat, 7);
    }

    for (i = 0; i < nbElecteursPrioHaute + nbElecteursPrioBasse; i++)
    {
        etat = pthread_join(idElecteurs[i], (void **)&cr);
        if (etat != 0)
            thdErreur("Join thds", etat, 8);
        free(cr);
    }

    return 0;
}
