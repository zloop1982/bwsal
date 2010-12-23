#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
#include <BWAPI.h>
class TerminateIfNoWorkBenches : public TaskStreamObserver
{
  public:
    TerminateIfNoWorkBenches();
    virtual void onFrame(TaskStream* ts);
};
