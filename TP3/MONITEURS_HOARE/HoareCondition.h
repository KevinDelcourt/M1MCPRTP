#ifndef HOARE_CONDITION_H

#define HOARE_CONDITION_H

#include <pthread.h>

#include "sem.h"

class HoareMonitor;

class HoareConditionBase {
  friend class HoareMonitor;
  protected:
    HoareMonitor &myHoareMonitor;
    unsigned nbWaiting;
    
  public:
    HoareConditionBase(HoareMonitor &itsHoareMonitor);
    virtual ~HoareConditionBase() {};
  
    virtual void signal() = 0;
    unsigned length() { return nbWaiting; }
};

class HoareCondition : public HoareConditionBase {
  friend class HoareMonitor;
  protected:
    Semaphore semCondition;
    
  public:
    HoareCondition(HoareMonitor &itsHoareMonitor);
    HoareCondition(const HoareCondition &oneHoareCondition);
    virtual ~HoareCondition();
    
    void wait();
    void signal();
};

class HoareConditionWithPriority : public HoareConditionBase {
  friend class HoareMonitor;
  protected:
    Semaphore *semCondition;
    unsigned *nbWaitingByPriority;
    unsigned nbWaiting;
    unsigned nbPriorities;
    
  public:
    HoareConditionWithPriority(HoareMonitor &itsHoareMonitor, unsigned nbPriorities);
    HoareConditionWithPriority(const HoareConditionWithPriority &oneHoareConditionWithPriority);
    virtual ~HoareConditionWithPriority();

    void wait(unsigned priority);
    void signal();
};
#endif
