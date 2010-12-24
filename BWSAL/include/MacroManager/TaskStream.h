#pragma once
#include <Task.h>
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>

class TaskStreamObserver;
class TaskStream
{
  public:
    TaskStream(Task* t0 = NULL, Task* t1 = NULL, Task* t2 = NULL, Task* t3=NULL);
    ~TaskStream();
    void terminate();
    void update();
    void clearPlanningData();
    bool updateStatus(); //returns true if planned additional resources (units, supply)

    void attach(TaskStreamObserver* obs, bool owned);
    void detach(TaskStreamObserver* obs);

    enum Status
    {
      None,
      Task_Stream_Queue_Empty,
      Task_Stream_Finished,
      Error_No_Work_Benches,
      Waiting_For_Worker_To_Be_Ready,
      Waiting_For_Free_Time,
      Waiting_For_Earliest_Start_Time,
      Waiting_For_Required_Units,
      Waiting_For_Required_Tech,
      Waiting_For_Required_Upgrade,
      Waiting_For_Supply,
      Waiting_For_Gas,
      Waiting_For_Minerals,
      Executing_Task,
    };
    Status getStatus() const;
    std::string getStatusString() const;
    int getStartTime() const;
    int getFinishTime() const;
    int getFinishTime(BWAPI::UnitType t) const;

    void setTaskStarted(bool started);
    void completeTask();

    void printToScreen(int x, int y);

    void makeWorkBench(BWAPI::Unit* worker = NULL);
    void makeWorkBenches(int count);

    std::set<WorkBench*> workBenches;
    bool assumeSufficientWorkers;
    std::list<Task*> queuedTasks;
    std::set<Task*> executingTasks;
    bool plannedAdditionalResources;
    void completeTask(WorkBench* wb, Task* t);
  private:
    void notifyNewStatus();
    //void notifyCompletedTask();
    void computeStatus();
    bool killSwitch;
    bool locationReady;
    Status status;
    std::map<TaskStreamObserver*, bool > observers;
};
