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

struct Activity
{
    pthread_t thread;
    double time;
    string action;
};

class LRMonitor : public HoareMonitor
{
private:
    double get_clock_time_in_ms();
    void pushActivity(string action);
    bool ecritureEnCours;
    int nbLecturesEnCours;
    HoareConditionWithPriority *condition;
    clock_t horloge;
    list<Activity> activities;

public:
    LRMonitor();

    void debutLire();
    void debutEcrire();
    void finLire();
    void finEcrire();

    void saveActivities();
};

#endif
