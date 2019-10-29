/* 
 * Lecteur - Redacteur - Moniteur - Specification - Gestion FIFO
 * @Author: Kévin Delcourt
 * */

#ifndef LR_H
#define LR_H

#include "MONITEURS_HOARE/HoareMonitor.h"
#include "MONITEURS_HOARE/HoareCondition.h"
#include <list>
#include <iterator>

class IsoloirMonitor : public HoareMonitor
{
private:
    int nbIso;
    list<pthread_t> isoloirs;
    HoareConditionWithPriority *condition;

public:
    IsoloirMonitor(int nbIsoloirs);
    void entrerIsoloir(int priorite);
    void sortirIsoloir();
};

#endif
