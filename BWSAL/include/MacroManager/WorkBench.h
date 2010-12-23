#pragma once
#include <Task.h>
#include <Arbitrator.h>
#include <BWAPI.h>
class TaskStream;
class TaskStreamObserver;
class WorkBench
{
  public:
    WorkBench(TaskStream* ts, BWAPI::Unit* worker = NULL);
    void setCurrentTask(Task* t);
    Task* getCurrentTask();
    void setLikelyNextTask(Task* t);
    Task* getLikelyNextTask();

    void setWorker(BWAPI::Unit* w);
    BWAPI::Unit* getWorker() const;
    void onRevoke(BWAPI::Unit* unit);
    void setBuildUnit(BWAPI::Unit* b);
    BWAPI::Unit* getBuildUnit() const;

    void computeBuildUnit();
    bool isWorkerReady();
    bool isLocationReady();
    TaskStream* getTaskStream();

  private:
    Task* currentTask;
    Task* likelyNextTask;
    BWAPI::Unit* worker;
    BWAPI::Unit* buildUnit;
    TaskStream* ts;
};