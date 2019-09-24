/* nbThread affichent un message a l'ecran
   Parametre du programme : nbThread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NB_THREADS_MAX  20
#define NB_FOIS         10

/*---------------------------------------------------------------------*/
/* Afficher un message d'erreur en fonction du code erreur obtenu
*/
void thdErreur(int codeErr, char *msgErr, void *codeArret) {
  fprintf(stderr, "%s: %d soit %s \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(codeArret);
}

/*---------------------------------------------------------------------*/
/* Fonction executee par un thread : afficher un message un certain nombre
  de fois nbFois a l'ecran, nbLignes lignes de messages ou nbLignes et
  genere aleatoirement
  Parametre de creation du thread : nbFois, le nombre d'affichages
*/
void *thd_afficher (void *arg) {
  int i, j, nbLignes;

  for (i = 0; i < NB_FOIS; i++) {
    nbLignes = rand()% (NB_FOIS);
    for (j = 0; j < nbLignes; j++) {
      printf("Thread %lu, j'affiche %d-%d \n", pthread_self(), i, j);
      usleep(10);
    } 
  }
  /* Se terminer sans renvoyer de compte-rendu */
  pthread_exit((void *)NULL);
}

/*---------------------------------------------------------------------*/
#define NB_AFFICHAGES 10

int main(int argc, char*argv[]) {
  pthread_t idThdAfficheurs[NB_THREADS_MAX];
  int i, etat, nbThreads;

  if (argc != 2) {
    printf("Usage : %s <Nb de threads>\n", argv[0]);
    exit(1);
  }

  nbThreads = atoi(argv[1]);
  if (nbThreads > NB_THREADS_MAX)
    nbThreads = NB_THREADS_MAX;

  /* Lancer les threads afficheurs */
  for (i = 0; i < nbThreads; i++) {
    if ((etat = pthread_create(&idThdAfficheurs[i], NULL,
                               thd_afficher, NULL)) != 0)
      thdErreur(etat, "Creation afficheurs", NULL);
  }

  /* Attendre la fin des threads afficheur car si le thread principal
    - i.e. le main() - se termine, les threads compagnons crees meurent aussi */
  for (i = 0; i < nbThreads; i++)
    if ((etat = pthread_join(idThdAfficheurs[i], NULL)) != 0)
      thdErreur(etat, "Join threads afficheurs", NULL);

  printf ("\nFin de l'execution du thread principal \n");
}
