#pragma once
class Task;
class TaskStream;
class WorkBench;
class TaskStreamObserver
{
  public:
    virtual void onAttach(TaskStream* ts) {}
    virtual void onDetach(TaskStream* ts) {}

    virtual void onWorkBenchCreate(TaskStream* ts, WorkBench* wb) {}
    virtual void onWorkBenchDestroy(TaskStream* ts, WorkBench* wb) {}

    virtual void onNewStatus(TaskStream* ts) {}
    virtual void onFrame(TaskStream* ts) {}

    virtual void onStartedTask(TaskStream* ts, WorkBench* wb, Task* t) {}
    virtual void onCompletedTask(TaskStream* ts, WorkBench* wb, Task* t) {}

};
