/* UPS
 * Lecteur - Redacteur - Moniteur - Gestion FIFO
 * @Author: Kévin Delcourt
 * */

#include "LRMonitor.h"
#include <string.h>

LRMonitor::LRMonitor()
{
    ecritureEnCours = false;
    nbLecturesEnCours = 0;
    condition = newConditionWithPriority(2);
    horloge = clock();
}

/*
    NB: Prière de ne pas faire attention aux appel de pushActivity
    Ces derniers servent à réaliser un compte-rendu de l'activité du moniteur et n'interviennent pas dans la synchronisation
*/
void LRMonitor::debutLire()
{
    pushActivity("Début Lire - Avant Enter");
    enter();
    pushActivity("Début Lire - Après Enter");
    if (ecritureEnCours || condition->length() > 0)
    {
        pushActivity("Début Lire - Avant Wait");
        condition->wait(1);
        pushActivity("Début Lire - Après Wait");
    }
    nbLecturesEnCours++;
    pushActivity("Début Lire - Avant Signal");
    condition->signal();
    pushActivity("Début Lire - Après Signal - Avant Leave");
    leave();
    pushActivity("Début Lire - Après Leave");
}

void LRMonitor::debutEcrire()
{
    pushActivity("Début Écrire - Avant Enter");
    enter();
    pushActivity("Début Écrire - Après Enter");
    if (nbLecturesEnCours > 0 || ecritureEnCours)
    {
        pushActivity("Début Écrire - Avant Wait 1");
        condition->wait(1);
        pushActivity("Début Écrire - Après Wait 1");
    }
    if (nbLecturesEnCours > 0)
    {
        pushActivity("Début Écrire - Avant Wait 2");
        condition->wait(0);
        pushActivity("Début Écrire - Après Wait 2");
    }
    ecritureEnCours = true;
    pushActivity("Début Écrire - Avant Leave");
    leave();
    pushActivity("Début Écrire - Après Leave");
}

void LRMonitor::finLire()
{
    pushActivity("Fin Lire - Avant Enter");
    enter();
    pushActivity("Fin Lire - Après Enter");
    nbLecturesEnCours--;
    if (nbLecturesEnCours == 0)
    {
        pushActivity("Fin Lire - Avant Signal");
        condition->signal();
        pushActivity("Fin Lire - Après Signal");
    }
    pushActivity("Fin Lire - Avant Leave");
    leave();
    pushActivity("Fin Lire - Après Leave");
}

void LRMonitor::finEcrire()
{
    pushActivity("Fin Écrire - Avant Enter");
    enter();
    pushActivity("Fin Écrire - Après Enter");
    ecritureEnCours = false;
    pushActivity("Fin Écrire - Avant Signal");
    condition->signal();
    pushActivity("Fin Écrire - Après signal - Avant Leave");
    leave();
    pushActivity("Fin Écrire - Après Leave");
}

//Private
double LRMonitor::get_clock_time_in_ms()
{
    return (clock() - horloge) / (double)CLOCKS_PER_SEC * 1000;
}

//Private
void LRMonitor::pushActivity(string action)
{
    Activity a;
    a.thread = pthread_self();
    a.time = get_clock_time_in_ms();
    a.action = action;
    activities.push_back(a);
}
/*
    Sauvegarde les actions faites sur le moniteur dans l'ordre chronologique dans un fichier activities.csv, à ouvrir avec votre tableur favori.
    Exemple de sortie pour 3 lecteurs / 2 rédacteurs: https://docs.google.com/spreadsheets/d/1_18OiXH2ZOsFTYT1Wm_FbPey5EE-zX6GpqPrmRsQG3U/edit?usp=sharing
    NB: Ne rentre pas en compte dans la résolution de l'exercice
*/
void LRMonitor::saveActivities()
{
    enter();
    FILE *f_out;
    list<pthread_t> pthreads;
    list<pthread_t>::iterator pthreadsIterator;
    list<Activity>::iterator activitiesIterator;
    for (activitiesIterator = activities.begin(); activitiesIterator != activities.end(); ++activitiesIterator)
    {
        bool knownThread = false;
        for (pthreadsIterator = pthreads.begin(); pthreadsIterator != pthreads.end(); ++pthreadsIterator)
        {
            knownThread = knownThread || *pthreadsIterator == activitiesIterator->thread;
        }
        if (!knownThread)
        {
            pthreads.push_back(activitiesIterator->thread);
        }
    }

    f_out = fopen("activities.csv", "w");
    fprintf(f_out, ",Thread\nt (ms)");
    for (pthreadsIterator = pthreads.begin(); pthreadsIterator != pthreads.end(); ++pthreadsIterator)
    {
        fprintf(f_out, ",%X", *pthreadsIterator);
    }
    fprintf(f_out, "\n");
    for (activitiesIterator = activities.begin(); activitiesIterator != activities.end(); ++activitiesIterator)
    {
        fprintf(f_out, "%.7f", activitiesIterator->time);
        for (pthreadsIterator = pthreads.begin(); pthreadsIterator != pthreads.end(); ++pthreadsIterator)
        {
            if (*pthreadsIterator == activitiesIterator->thread)
            {
                fprintf(f_out, ",%s", activitiesIterator->action.c_str());
            }
            else
            {
                fprintf(f_out, ",-");
            }
        }
        fprintf(f_out, "\n");
    }
    fclose(f_out);
    leave();
}