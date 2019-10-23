/* 
 * Lecteur - Redacteur - Moniteur - Gestion FIFO
 * @Author: Kévin Delcourt
 * */

#include "LRMonitor.h"
#include <string.h>

/*--------------------------------------------------*/
LRMonitor::LRMonitor()
{
    nbEcrituresEnCours = 0;
    nbLecturesEnCours = 0;
    condition = newCondition();
}

void LRMonitor::debutLire()
{
    enter();
    while (nbEcrituresEnCours > 0)
    {
        condition->wait();
    }
    nbLecturesEnCours++;
    condition->signal();
    leave();
}

void LRMonitor::debutEcrire()
{
    enter();
    while (nbLecturesEnCours > 0 || nbEcrituresEnCours > 0)
    {
        condition->wait();
    }
    nbEcrituresEnCours++;
    leave();
}

void LRMonitor::finLire()
{
    enter();
    nbLecturesEnCours--;
    if (nbLecturesEnCours == 0)
    {
        condition->signal();
    }
    leave();
}

void LRMonitor::finEcrire()
{
    enter();
    nbEcrituresEnCours--;
    condition->signal();
    leave();
}