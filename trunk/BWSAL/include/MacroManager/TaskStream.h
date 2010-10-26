#pragma once
#include <Task.h>
#include <Arbitrator.h>
#include <BWAPI.h>
class TaskStreamObserver;
class TaskStream : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    TaskStream(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    virtual std::string getName() const;
    virtual std::string getShortName() const;

    void attach(TaskStreamObserver* obs);
    void detach(TaskStreamObserver* obs);

    void notifyObservers();

    enum Status
    {
      Error_Worker_Not_Specified,
      Error_Task_Not_Specified,
      Error_Location_Not_Specified,
      Error_Location_Unreachable,
      Error_Location_Blocked,
      Error_Task_Requires_Addon,
      Waiting_For_Required_Units,
      Waiting_For_Required_Tech,
      Waiting_For_Required_Upgrade,
      Waiting_For_Worker,
      Waiting_For_Supply,
      Waiting_For_Gas,
      Waiting_For_Minerals,
      Executing_Task,
    };
    Status getStatus() const;

    void setWorker(BWAPI::Unit* w);
    BWAPI::Unit* getWorker() const;

    void setTask(Task t);
    Task getTask() const;

    void setUrgent(bool isUrgent);
    bool isUrgent() const;
    void setNextTask(Task t);
    Task getNextTask() const;

    void setName(std::string s);

  private:
    bool urgent;
    Task task;
    Task nextTask;
    BWAPI::Unit* worker;
    Status status;
    std::string name;
    std::set<TaskStreamObserver* > observers;
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
};
