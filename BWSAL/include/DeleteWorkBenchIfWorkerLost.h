#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
#include <BWAPI.h>
class DeleteWorkBenchIfWorkerLost : public TaskStreamObserver
{
  public:
    DeleteWorkBenchIfWorkerLost();
    virtual void onFrame(TaskStream* ts);
};
