#ifndef HOARE_BASE_MONITOR_H

#define HOARE_BASE_MONITOR_H

#include <pthread.h>

#include "sem.h"
#include "List.h"

class HoareCondition;
class HoareConditionWithPriority;
class List;
class HoareMonitor {
  
  friend class HoareCondition;
  friend class HoareConditionWithPriority;

  protected:
    Mutex mutex;
    Semaphore urgent;
    unsigned cptUrgent;
    List listCondition;
    
  protected:
    HoareMonitor();
    HoareMonitor(const HoareMonitor &oneMonitor);
    ~HoareMonitor();
    
    void enter();
    void leave();
    
    HoareCondition *newCondition();
    HoareConditionWithPriority *newConditionWithPriority(unsigned nbPriorities);
};

#endif
