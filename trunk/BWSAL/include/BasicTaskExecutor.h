#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
class BasicTaskExecutor : public TaskStreamObserver
{
  public:
    static BasicTaskExecutor* getInstance();
    virtual void attached(TaskStream* ts);
    virtual void detached(TaskStream* ts);
    virtual void newStatus(TaskStream* ts);
    virtual void completedTask(TaskStream* ts, const Task &t);
    virtual void update(TaskStream* ts);
  private:
    struct data
    {
      BWAPI::Unit* buildUnit;
      int targetLevel;
      bool started;
    };
    std::map< TaskStream*, data > taskStreams;
};
