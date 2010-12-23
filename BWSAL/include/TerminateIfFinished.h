#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
#include <BWAPI.h>
class TerminateIfFinished : public TaskStreamObserver
{
  public:
    TerminateIfFinished();
    virtual void onCompletedTask(TaskStream* ts);
};
