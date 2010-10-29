#pragma once
#include <Task.h>
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>

class TaskStreamObserver;
class TaskStream : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    TaskStream(Task t=Task(), Task nt=Task());
    ~TaskStream();
    void terminate();
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    void clearPlanningData();
    void updateStatus();
    virtual std::string getName() const;
    virtual std::string getShortName() const;

    void attach(TaskStreamObserver* obs, bool owned);
    void detach(TaskStreamObserver* obs);

    void notifyNewStatus();
    void notifyCompletedTask();


    enum Status
    {
      None,
      Error_Task_Not_Specified,
      Error_Worker_Not_Specified,
      Error_Worker_Not_Owned,
      Error_Location_Not_Specified,
      Error_Location_Blocked,
      Error_Task_Requires_Addon,
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

    void setWorker(BWAPI::Unit* w);
    BWAPI::Unit* getWorker() const;

    void setBuildUnit(BWAPI::Unit* b);
    BWAPI::Unit* getBuildUnit() const;

    void setTaskStarted(bool started);
    void completeTask();

    bool isWorkerReady() const;
    bool isLocationReady() const;

    void setTask(Task t);
    Task& getTask();

    void setUrgent(bool isUrgent);
    bool isUrgent() const;
    void setNextTask(Task t);
    Task& getNextTask();

    void setName(std::string s);
    void printToScreen(int x, int y);
  private:
    void computeStatus();
    bool killSwitch;
    bool urgent;
    bool workerReady;
    bool locationReady;
    Task task[2];
    BWAPI::Unit* worker;
    BWAPI::Unit* buildUnit;
    Status status;
    std::string name;
    std::map<TaskStreamObserver*, bool > observers;
};
