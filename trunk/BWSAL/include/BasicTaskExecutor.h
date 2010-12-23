#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
class BasicTaskExecutor : public TaskStreamObserver
{
  public:
    static BasicTaskExecutor* getInstance();

    virtual void onAttach(TaskStream* ts);
    virtual void onDetach(TaskStream* ts);

    virtual void onNewStatus(TaskStream* ts);
    virtual void onFrame(TaskStream* ts);

    virtual void onCompletedTask(TaskStream* ts, const Task &t);

    void computeIsExecuting(TaskStream* ts);
    void computeIsCompleted(TaskStream* ts);
    void computeIsReady(TaskStream* ts);
    void getReady(TaskStream* ts);
    void execute(TaskStream* ts);

  private:
    struct data
    {
      bool isReady;
      int targetLevel;
    };
    std::map< TaskStream*, std::map<WorkBench*, data > > wbData;
};
