/* 
 * Lecteur - Redacteur - Moniteur - Specification - Gestion FIFO
 * @Author: Kévin Delcourt
 * */

#ifndef LR_H
#define LR_H

#include "MONITEURS_HOARE/HoareMonitor.h"
#include "MONITEURS_HOARE/HoareCondition.h"

class LRMonitor : public HoareMonitor
{

protected:
    int nbLecturesEnCours;
    int nbEcrituresEnCours;

    HoareCondition *condition;

public:
    LRMonitor();

    void debutLire();
    void debutEcrire();
    void finLire();
    void finEcrire();
};

#endif
