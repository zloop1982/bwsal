#pragma once
#include <Task.h>
class TaskStream;
class TaskStreamObserver
{
  public:
    virtual void attached(TaskStream* ts) {}
    virtual void detached(TaskStream* ts) {}
    virtual void newStatus(TaskStream* ts) {}
    virtual void completedTask(TaskStream* ts, const Task &t) {}
    virtual void update(TaskStream* ts) {}
};
