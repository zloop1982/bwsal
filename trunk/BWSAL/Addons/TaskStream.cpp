#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/UnitReadyTimeCalculator.h>
#include <BasicWorkerFinder.h>
#include <TerminateIfFinished.h>
#include <BasicTaskExecutor.h>
#include <BFSBuildingPlacer.h>
#include <math.h>
using namespace BWAPI;
using namespace std;
TaskStream::TaskStream(Task* t0, Task* t1, Task* t2, Task* t3)
{
  if (t0)
    queuedTasks.push_back(t0);
  if (t1)
    queuedTasks.push_back(t1);
  if (t2)
    queuedTasks.push_back(t2);
  if (t3)
    queuedTasks.push_back(t3);
  status     = None;
  killSwitch = false;
  plannedAdditionalResources = false;
}
TaskStream::~TaskStream()
{
  //delete observers that we own
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->onDetach(this);
    if (obs.second)
      delete obs.first;
  }
  for each(WorkBench* wb in workBenches)
  {
    delete wb;
  }
  observers.clear();
}
void TaskStream::terminate()
{
  TheMacroManager->killSet.insert(this);
  killSwitch = true;
}
void TaskStream::computeStatus()
{
  /*
  if (task[0].getType()==TaskTypes::Unit && task[0].getTilePosition().isValid() && task[0].getUnit().isBuilding())
  {
    UnitType ut = task[0].getUnit();
    TilePosition tp = task[0].getTilePosition();
    if (ut.isAddon())
    {
      tp.x()+=4;
      tp.y()++;
    }
    if (locationReady)
      Broodwar->drawBoxMap(tp.x()*32,tp.y()*32,tp.x()*32+ut.tileWidth()*32,tp.y()*32+ut.tileHeight()*32,Colors::Green);
    else
      Broodwar->drawBoxMap(tp.x()*32,tp.y()*32,tp.x()*32+ut.tileWidth()*32,tp.y()*32+ut.tileHeight()*32,Colors::Red);
    Broodwar->drawTextMap(tp.x()*32,tp.y()*32,"%s",ut.getName().c_str());
  }
  */
  if (queuedTasks.empty())
  {
    if (executingTasks.empty())
      status = Task_Stream_Finished;
    else
      status = Task_Stream_Queue_Empty;
    return;
  }
  /*
  for(std::list<Task*>::iterator i=queuedTasks.begin();i!=queuedTasks.end();i++)
  {
    if (i>0 && task[i-1].getStartTime()==-1) break;
    if (task[i].getType()==TaskTypes::None) break;
    if (!task[i].hasReservedResourcesThisFrame())
    {
      UnitReadyTimeStatus::Enum reason;
      WorkBench* work_bench_for_this_task = &(*workBenches.begin());
      int min_valid_time = -1;
      for each(WorkBench wb in workBenches)
      {
        int first_valid_frame = UnitReadyTimeCalculator::getFirstFreeTime(wb.getWorker(), task[i], reason,true,true);
        if (first_valid_frame == Broodwar->getFrameCount() && !wb.isWorkerReady())
        {
          if (assumeSufficientWorkers)
          {
            first_valid_frame = Broodwar->getFrameCount()+10;
            reason = UnitReadyTimeStatus::Error_WorkerBench_Not_Ready;
          }
          else
            first_valid_frame = -1;
        }
        if (min_valid_time == -1 || first_valid_time<min_valid_time)
        {
          min_valid_time = first_valid_frame;
          work_bench_for_this_task = &wb;
        }
      }
      Unit* worker = work_bench_for_this_task->getWorker();
      task[i].setStartTime(min_valid_time);
      //if we need to wait to start the first task, compute the correct status
      if (min_valid_time!=-1)
      {
        if (!task[i].hasReservedFinishDataThisFrame())
          TheMacroManager->reserveResources(work_bench_for_this_task,task[i]);
        if (task[i].hasReservedResourcesThisFrame() && !task[i].hasReservedFinishDataThisFrame())
        {
          TheMacroManager->reserveFinishData(task[i]);
          plannedAdditionalResources = true;
        }
      }
      if ( i==0 ) //compute task stream status based on status of first unit
      {
        if (task[0].getStartTime()!=-1 && task[0].getStartTime()<=Broodwar->getFrameCount() && work_bench_for_this_task->isWorkerReady())
        {
          work_bench_for_this_task->setTask(task[0]);
          task.erase(task.begin());
          i--;
        }
        else
        {
          if (reason == UnitReadyTimeStatus::Error_Task_Requires_Addon)
            status = Error_Task_Requires_Addon;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Worker_To_Be_Ready)
            status = Waiting_For_Worker_To_Be_Ready;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Free_Time)
            status = Waiting_For_Free_Time;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Earliest_Start_Time)
            status = Waiting_For_Earliest_Start_Time;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Units)
            status = Waiting_For_Required_Units;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Tech)
            status = Waiting_For_Required_Tech;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Upgrade)
            status = Waiting_For_Required_Upgrade;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Supply)
            status = Waiting_For_Supply;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Gas)
            status = Waiting_For_Gas;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Minerals)
            status = Waiting_For_Minerals;
          else if (reason == UnitReadyTimeStatus::Error_Worker_Not_Specified)
            status = Error_Worker_Not_Specified;
          else
            status = Error_Worker_Not_Owned;
        }
      }
      if (min_valid_time==-1) break;
    }
  }
  for(int i=1;i<(int)(task.size());i++)
  {
    if (task[i-1].getStartTime()==-1)
      task[i].setStartTime(-1);
  }
  */
}
void TaskStream::update()
{
  if (killSwitch) return;
  for each(WorkBench* wb in workBenches)
    wb->update();
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
    obs.first->onFrame(this);
}
bool TaskStream::updateStatus()
{
  plannedAdditionalResources = false;
  if (killSwitch) return false;
  Status lastStatus = status;
  computeStatus();
  if (status != lastStatus)
    notifyNewStatus();
  return plannedAdditionalResources;
}
void TaskStream::attach(TaskStreamObserver* obs, bool owned)
{
  //add observer to our observers set
  observers.insert(std::make_pair(obs, owned));
  //let the observer know they have been attached to us
  obs->onAttach(this);
}
void TaskStream::detach(TaskStreamObserver* obs)
{
  //remove observer from our observers set
  observers.erase(obs);
  //let the observer know they have been detached from us
  obs->onDetach(this);
}

void TaskStream::notifyNewStatus()
{
  //notify all observers of our new status
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->onNewStatus(this);
  }
}
void TaskStream::completeTask(WorkBench* wb, Task* task)
{
  executingTasks.erase(task);
  //notify all observers that we have completed a task
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->onCompletedTask(this,wb,task);
  }
}
TaskStream::Status TaskStream::getStatus() const
{
  return status;
}
std::string TaskStream::getStatusString() const
{
  //turn the status into a string
  switch (status)
  {
    case None:
      return "None";
    break;
    case Task_Stream_Queue_Empty:
      return "Task_Stream_Queue_Empty";
    break;
    case Task_Stream_Finished:
      return "Task_Stream_Finished";
    break;
    case Error_No_Work_Benches:
      return "Error_No_Work_Benches";
    break;
    case Waiting_For_Worker_To_Be_Ready:
      return "Waiting_For_Worker_To_Be_Ready";
    break;
    case Waiting_For_Free_Time:
      return "Waiting_For_Free_Time";
    break;
    case Waiting_For_Earliest_Start_Time:
      return "Waiting_For_Earliest_Start_Time";
    break;
    case Waiting_For_Required_Units:
      return "Waiting_For_Required_Units";
    break;
    case Waiting_For_Required_Tech:
      return "Waiting_For_Required_Tech";
    break;
    case Waiting_For_Required_Upgrade:
      return "Waiting_For_Required_Upgrade";
    break;
    case Waiting_For_Supply:
      return "Waiting_For_Supply";
    break;
    case Waiting_For_Gas:
      return "Waiting_For_Gas";
    break;
    case Waiting_For_Minerals:
      return "Waiting_For_Minerals";
    break;
    case Executing_Task:
      return "Executing_Task";
    break;
    default:
      return "Unknown";
  }
  return "Unknown";
}
void TaskStream::printToScreen(int x, int y)
{
  /*
  Broodwar->drawTextScreen(x,y,"[ ] %s - w=%x bu=%x",
    getStatusString().c_str(),
    getWorker(),
    getBuildUnit());
  Broodwar->drawTextScreen(x+200,y,"%s %d",
    task[0].getName().c_str(),
    task[0].getStartTime());
  Broodwar->drawTextScreen(x+300,y,"%s %d",
    task[1].getName().c_str(),
    task[1].getStartTime());
  Broodwar->drawTextScreen(x+400,y,"%s %d",
    task[2].getName().c_str(),
    task[2].getStartTime());
  Broodwar->drawTextScreen(x+500,y,"%s %d",
    task[3].getName().c_str(),
    task[3].getStartTime());
    */
}

void TaskStream::clearPlanningData()
{
  /*
  //clear reserved resources and reserved finish data
  for(int i=0;i<(int)(task.size());i++)
  {
    task[i].setReservedFinishDataThisFrame(task[i].isCompleted());
    task[i].setReservedResourcesThisFrame(task[i].hasSpentResources());
  }
  for each(WorkBench wb in workBenches)
  {
    if (wb.getTask().hasSpentResources())
    {
      if (wb.getTask().getType()==TaskTypes::Unit)
      {
        if (wb.getTask().getUnit().supplyProvided()>0)
          TheMacroManager->rtl.registerSupplyIncrease(wb.getTask().getFinishTime(), wb.getTask().getUnit().supplyProvided());
        int count = 1;
        if (wb.getTask().getUnit().isTwoUnitsInOneEgg())
          count = 2;
        TheMacroManager->uctl.registerUnitCountChange(wb.getTask().getFinishTime(), wb.getTask().getUnit(), count);
      }
      if (wb.getTask().getType()==TaskTypes::Tech)
        TheMacroManager->ttl.registerTechFinish(wb.getTask().getFinishTime(),wb.getTask().getTech());
      if (wb.getTask().getType()==TaskTypes::Upgrade)
        TheMacroManager->utl.registerUpgradeLevelIncrease(wb.getTask().getFinishTime(),wb.getTask().getUpgrade());
    }
    wb.getTask().setReservedFinishDataThisFrame(true);
  }
  */
  plannedAdditionalResources = false;
}
int TaskStream::getStartTime() const
{
  //start time of task stream is start time of first task in stream
  //return task[0].getStartTime();
  return -1;
}
int TaskStream::getFinishTime() const
{
  /*
  //finish time of task stream is finish time of last task in stream
  if (task[0]==NULL)
    return Broodwar->getFrameCount();
  for(int i=0;i<(int)(task.size());i++)
    if (task[i].getFinishTime() == -1)
      return -1; //just to be safe, return never if any of the tasks will never finish
  return task[task.size()-1].getFinishTime();
  */
  return -1;
}

int TaskStream::getFinishTime(BWAPI::UnitType t) const
{
  /*
  //returns the first time that a task of the given unit type will finish
  for(int i=0;i<(int)(task.size());i++)
    if (task[i].getType()==TaskTypes::Unit && task[i].getUnit()==t)
      return task[i].getFinishTime();
  //or returns never if the task stream will never finish a task of the given unit type
  */
  return -1;
}
void TaskStream::makeWorkBench(Unit* worker)
{
  workBenches.insert(new WorkBench(this,worker));
}
void TaskStream::makeWorkBenches(int count)
{
  for(int i=0;i<count;i++)
    workBenches.insert(new WorkBench(this));
}