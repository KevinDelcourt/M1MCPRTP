/* UPS
 * Lecteur - Redacteur - Moniteur - Gestion FIFO
 * @Author: Kévin Delcourt
 * */

#include "IsoloirMonitor.h"
#include <string.h>

IsoloirMonitor::IsoloirMonitor(int nbIsoloirs)
{
    nbIso = nbIsoloirs;
    condition = newConditionWithPriority(2);
}

void IsoloirMonitor::entrerIsoloir(int priorite)
{
    enter();
    if (isoloirs.size() >= nbIso)
    {
        condition->wait(priorite);
    }
    isoloirs.push_back(pthread_self());
    leave();
}

void IsoloirMonitor::sortirIsoloir()
{
    enter();
    isoloirs.remove(pthread_self());
    condition->signal();
    leave();
}
