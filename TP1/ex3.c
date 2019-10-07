/*
 * Producteur-consommateur, variante 3: retraits à la demande
 * 
 * Compilation : gcc m1_ProdConso_base.c -lpthread [-DOptionDeTrace ...] -o prodconso
 *
 * Options de trace lors de la compilation (-D) : 
 * TRACE_BUF : tracer le contenu du buffer
 * TRACE_THD : tracer la creation des threads
 * TRACE_SOUHAIT : tracer ce que veulent faire les producteurs/consommateurs
 * 
 * @Author : Kévin Delcourt
 * */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NB_PROD_MAX 20
#define NB_CONSO_MAX 20

#define NB_CASES_MAX 20

#define NB_DEPOT_MAX 10
#define NB_RETRAIT_MAX 10

typedef struct
{
    char info[80];
    int type;     // Message de 2 types (0/1 par exemple)
    int rangProd; // Qui a produit le message
} TypeMessage;

typedef struct
{
    TypeMessage buffer[NB_CASES_MAX]; // Buffer
    int iDepot;                       // Indice prochain depot
    int iRetrait;                     // Indice prochain retrait
    int nbVide;                       // Nombre de cases vides
    int nbConsoEnAttente[2];          // Nombre de consommateurs en attente dans la condition de type 0 ou 1
} RessourceCritique;                  // A completer eventuellement pour la synchro

// Variables partagees entre tous
RessourceCritique resCritiques; // Modifications donc conflits possibles
int nbCases;                    // Taille effective du buffer,
                                // Pas de modif donc pas de conflit
int nbDepots;
int nbRetraits;

typedef struct
{                // Parametre des threads
    int rang;    // - rang de creation
    int typeMsg; // - type de message a deposer/retirer (si besoin)
} Parametres;

pthread_cond_t condDepot;
pthread_cond_t condRetrait[2];
pthread_mutex_t exclusionMutuelleMoniteur = PTHREAD_MUTEX_INITIALIZER;

/*---------------------------------------------------------------------*/
/* codeErr : code retournee par une primitive
 * msgErr  : message d'erreur personnalise
 * valErr  : valeur retournee par le thread
 */
void thdErreur(int codeErr, char *msgErr, int valeurErr)
{
    int *retour = malloc(sizeof(int));
    *retour = valeurErr;
    fprintf(stderr, "\033[1;31m ERREUR %s \033[0m %d - %s \n", msgErr, codeErr, strerror(codeErr));
    pthread_exit(retour);
}

/*--------------------------------------------------*/
void initialiserVarPartagees(void)
{
    int i;

    /* Le buffer, les indices et le nombre de cases pleines */
    resCritiques.iDepot = 0;
    resCritiques.iRetrait = 0;
    resCritiques.nbVide = nbCases;
    resCritiques.nbConsoEnAttente[0] = 0;
    resCritiques.nbConsoEnAttente[1] = 0;
    for (int i = 0; i < nbCases; i++)
    {
        strcpy(resCritiques.buffer[i].info, "Message vide");
        resCritiques.buffer[i].type = 0;
        resCritiques.buffer[i].rangProd = -1;
    }
}

/*--------------------------------------------------*/
char *afficherType(int type)
{
    static char affichage[22];

    if (type == 0)
        sprintf(affichage, "\033[0;36m[T%d]\033[0m", type);
    else
        sprintf(affichage, "\033[0;35m[T%d]\033[0m", type);

    return affichage;
}

void afficherBuffer(void)
{
    int i;

    printf("\n[ \n");
    for (i = 0; i < nbCases; i++)
    {
        printf("\tCase %d : %s %s (de %d)",
               i,
               afficherType(resCritiques.buffer[i].type),
               resCritiques.buffer[i].info,
               resCritiques.buffer[i].rangProd);

        if (i == resCritiques.iDepot)
            printf("\033[0;33m < iDepot \033[0m");
        if (i == resCritiques.iRetrait)
            printf("\033[0;33m < iRetrait \033[0m");
        printf("\n");
    }
    printf("]\033[0;33m%d cases vides\033[0m\n\n", resCritiques.nbVide);
}

int typeProchainRetrait()
{
    return resCritiques.buffer[resCritiques.iDepot].type;
}

/*--------------------------------------------------*/
void depot(const TypeMessage *leMessage)
{
    strcpy(resCritiques.buffer[resCritiques.iDepot].info, leMessage->info);
    resCritiques.buffer[resCritiques.iDepot].type = leMessage->type;
    resCritiques.buffer[resCritiques.iDepot].rangProd = leMessage->rangProd;
    resCritiques.iDepot = (resCritiques.iDepot + 1) % nbCases;
    resCritiques.nbVide--;
#ifdef TRACE_BUF
    afficherBuffer();
#endif
}

/*--------------------------------------------------*/
void retrait(TypeMessage *leMessage)
{
    strcpy(leMessage->info, resCritiques.buffer[resCritiques.iRetrait].info);
    leMessage->type = resCritiques.buffer[resCritiques.iRetrait].type;
    leMessage->rangProd = resCritiques.buffer[resCritiques.iRetrait].rangProd;
    resCritiques.iRetrait = (resCritiques.iRetrait + 1) % nbCases;
    resCritiques.nbVide++;
#ifdef TRACE_BUF
    afficherBuffer();
#endif
}

/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void deposer(TypeMessage leMessage, int rangProd)
{
    int etat;
    while (resCritiques.nbVide == 0)
    {
#ifdef TRACE_SOUHAIT
        printf("\t\t\tProd %d: \033[0;33m En attente\033[0m dans la condition condDepot\n",
               rangProd);
#endif

        if (0 != (etat = pthread_cond_wait(&condDepot, &exclusionMutuelleMoniteur)))
            thdErreur(etat, "Wait condition depot", etat);
#ifdef TRACE_SOUHAIT
        printf("\t\t\tProd %d: \033[0;32m Sort\033[0m de la condition condDepot\n",
               rangProd);
#endif
    }

    depot(&leMessage);
    printf("\tProd %d : Message a ete depose = %s %s (de %d)\n", rangProd, afficherType(leMessage.type), leMessage.info, leMessage.rangProd);

    if (0 != (etat = pthread_cond_signal(&condRetrait[typeProchainRetrait()])))
        thdErreur(etat, "Signal condition depot", etat);
}

/*--------------------------------------------------
 * Correspondra au service du moniteur vu en TD
 * La synchronisation sera ajoutee dans cette operation
 * */
void retirer(TypeMessage *unMessage, int rangConso, int typeVoulu)
{
    int etat;
    while (resCritiques.nbVide == nbCases || typeProchainRetrait() != typeVoulu)
    {

#ifdef TRACE_SOUHAIT
        printf("\t\t\tConso %d: \033[0;33m En attente\033[0m dans la condition condRetrait\n",
               rangConso);
#endif
        resCritiques.nbConsoEnAttente[typeVoulu]++;
        if (0 != (etat = pthread_cond_wait(&condRetrait[typeVoulu], &exclusionMutuelleMoniteur)))
            thdErreur(etat, "Wait condition retrait", etat);
        resCritiques.nbConsoEnAttente[typeVoulu]--;
#ifdef TRACE_SOUHAIT
        printf("\t\t\tConso %d: \033[0;32m Sort\033[0m de la condition condRetrait\n",
               rangConso);
#endif
    }

    retrait(unMessage);
    printf("\t\tConso %d : Message a ete lu = %s %s (de %d)\n", rangConso, afficherType(unMessage->type), unMessage->info, unMessage->rangProd);

    if (resCritiques.nbVide < nbCases && resCritiques.nbConsoEnAttente[typeProchainRetrait()] > 0)
    {
        if (0 != (etat = pthread_cond_signal(&condRetrait[typeProchainRetrait()])))
            thdErreur(etat, "Signal condition retrait", etat);
    }
    else
    {
        if (0 != (etat = pthread_cond_signal(&condDepot)))
            thdErreur(etat, "Signal condition depot", etat);
    }
}

/*--------------------------------------------------*/
void *producteur(void *arg)
{
    int i, etat;
    TypeMessage leMessage;
    Parametres param = *(Parametres *)arg;

    srand(pthread_self());

    for (i = 0; i < nbDepots; i++)
    {
        sprintf(leMessage.info, "%s %d", "bonjour num ", i);
        leMessage.type = param.typeMsg;
        leMessage.rangProd = param.rang;

#ifdef TRACE_SOUHAIT
        printf("\t\t\tProd %d : Je veux deposer = [T%d] %s (de %d)\n",
               param.rang, leMessage.type, leMessage.info, leMessage.rangProd);
#endif
        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", etat);

        deposer(leMessage, param.rang);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", etat);

        //usleep(rand() % (100 * param.rang + 100));
    }
    pthread_exit(NULL);
}

/*--------------------------------------------------*/
void *consommateur(void *arg)
{
    int i, etat, typeVoulu;
    TypeMessage unMessage;
    Parametres *param = (Parametres *)arg;

    srand(pthread_self());

    for (i = 0; i < nbRetraits; i++)
    {
        typeVoulu = rand() % 2;
#ifdef TRACE_SOUHAIT
        printf("\t\t\tConso %d : Je veux retirer un message de type %d\n", param->rang, typeVoulu);
#endif
        if (0 != (etat = pthread_mutex_lock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Lock mutex global", etat);

        retirer(&unMessage, param->rang, typeVoulu);

        if (0 != (etat = pthread_mutex_unlock(&exclusionMutuelleMoniteur)))
            thdErreur(etat, "Unlock mutex global", etat);

        //usleep(rand() % (100 * param->rang + 100));
    }
    pthread_exit(NULL);
}

int minArg(char *arg, int max)
{
    if (atoi(arg) > max)
        return max;
    return atoi(arg);
}

/*--------------------------------------------------*/
int main(int argc, char *argv[])
{
    int i, etat;
    int nbThds, nbProd, nbConso;
    Parametres paramThds[NB_PROD_MAX + NB_CONSO_MAX];
    pthread_t idThdProd[NB_PROD_MAX], idThdConso[NB_CONSO_MAX];

    if (argc <= 5)
    {
        printf("Usage: %s <Nb Prod <= %d> <Nb Conso <= %d> <Nb Cases <= %d> <Nb Depots <= %d> <Nb retraits <= %d> \n",
               argv[0], NB_PROD_MAX, NB_CONSO_MAX, NB_CASES_MAX, NB_DEPOT_MAX, NB_RETRAIT_MAX);
        exit(2);
    }

    nbProd = minArg(argv[1], NB_PROD_MAX);
    nbConso = minArg(argv[2], NB_CONSO_MAX);
    nbThds = nbProd + nbConso;
    nbCases = minArg(argv[3], NB_CASES_MAX);
    nbDepots = minArg(argv[4], NB_DEPOT_MAX);
    nbRetraits = minArg(argv[5], NB_RETRAIT_MAX);

#ifdef TRACE_THD
    printf("\033[0;32m Paramètres: \033[0m\n - %d producteurs \n - %d consommateurs \n - %d cases dans le buffer \n - %d depots pour chaque producteur \n - %d retraits pour chaque consommateur\n",
           nbProd, nbConso, nbCases, nbDepots, nbRetraits);
#endif

    if (0 != (etat = pthread_cond_init(&condDepot, NULL)))
        thdErreur(etat, "Init condition depot", etat);

    if (0 != (etat = pthread_cond_init(&condRetrait[0], NULL)))
        thdErreur(etat, "Init condition depot 0", etat);

    if (0 != (etat = pthread_cond_init(&condRetrait[1], NULL)))
        thdErreur(etat, "Init condition depot 1", etat);

    initialiserVarPartagees();

    /* Creation des nbProd producteurs et nbConso consommateurs */
    for (i = 0; i < nbThds; i++)
    {
        if (i < nbProd)
        {
            paramThds[i].typeMsg = i % 2;
            paramThds[i].rang = i;
            if ((etat = pthread_create(&idThdProd[i], NULL, producteur, &paramThds[i])) != 0)
                thdErreur(etat, "Creation producteur", etat);
#ifdef TRACE_THD
            printf("Creation thread prod %lu de rang %d -> %d/%d\n", idThdProd[i], i, paramThds[i].rang, nbProd);
#endif
        }
        else
        {
            paramThds[i].typeMsg = i % 2;
            paramThds[i].rang = i - nbProd;
            if ((etat = pthread_create(&idThdConso[i - nbProd], NULL, consommateur, &paramThds[i])) != 0)
                thdErreur(etat, "Creation consommateur", etat);
#ifdef TRACE_THD
            printf("Creation thread conso %lu de rang %d -> %d/%d\n", idThdConso[i - nbProd], i, paramThds[i].rang, nbConso);
#endif
        }
    }

    /* Attente de la fin des threads */
    for (i = 0; i < nbProd; i++)
    {
        if ((etat = pthread_join(idThdProd[i], NULL)) != 0)
            thdErreur(etat, "Join threads producteurs", etat);
#ifdef TRACE_THD
        printf("Fin thread producteur de rang %d\n", i);
#endif
    }

    for (i = 0; i < nbConso; i++)
    {
        if ((etat = pthread_join(idThdConso[i], NULL)) != 0)
            thdErreur(etat, "Join threads consommateurs", etat);
#ifdef TRACE_THD
        printf("Fin thread consommateur de rang %d\n", i);
#endif
    }

    if (0 != (etat = pthread_mutex_destroy(&exclusionMutuelleMoniteur)))
        thdErreur(etat, "Destroy mutex global", etat);

    if (0 != (etat = pthread_cond_destroy(&condDepot)))
        thdErreur(etat, "Destroy condition depot", etat);

    if (0 != (etat = pthread_cond_destroy(&condRetrait[0])))
        thdErreur(etat, "Destroy condition retrait", etat);

    if (0 != (etat = pthread_cond_destroy(&condRetrait[1])))
        thdErreur(etat, "Destroy condition retrait", etat);

#ifdef TRACE_THD
    printf("\nFin de l'execution du main \n");
#endif

    return 0;
}
