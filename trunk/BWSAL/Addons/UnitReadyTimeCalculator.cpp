#include <MacroManager/UnitReadyTimeCalculator.h>
#include <MacroManager.h>
using namespace BWAPI;
using namespace std;

//returns the frame when the unit will be ready to do another task
int UnitReadyTimeCalculator::getReadyTime(BWAPI::Unit* unit, bool considerTasks)
{
  if (unit==NULL || unit->exists()==false) return -1;
  int t=Broodwar->getFrameCount();
  t=max(t,t+unit->getRemainingResearchTime());
  t=max(t,t+unit->getRemainingUpgradeTime());
  t=max(t,t+unit->getRemainingBuildTime());
  t=max(t,t+unit->getRemainingTrainTime());
  if (considerTasks)
  {
    TaskStream* ts = TheMacroManager->getTaskStream(unit);
    if (ts)
    {
      int t2=ts->getFinishTime();
      if (t2==-1)
        return -1;
      t=max(t,t2);
    }
  }
  return t;
}

//returns the frame when the unit will be ready to do the given task
int UnitReadyTimeCalculator::getReadyTime(BWAPI::Unit* unit, const Task &task, UnitReadyTimeStatus::Enum &reason, bool considerResources, bool considerTasks)
{
  //WARNING: does not yet take into account terran addons correctly!
  reason = UnitReadyTimeStatus::Waiting_For_Worker_To_Be_Ready;
  int t = getReadyTime(unit,considerTasks);
  if (t==-1) return -1;
  if (considerResources)
  {
    int t2=TheMacroManager->rtl.getFirstValidTime(task.getResources());
    if (t2==-1 || t2>t)
    {
      ResourceTimeline::ErrorCode res_error = TheMacroManager->rtl.getLastError();
      if (res_error = ResourceTimeline::Insufficient_Supply)
        reason = UnitReadyTimeStatus::Waiting_For_Supply;
      if (res_error = ResourceTimeline::Insufficient_Gas)
        reason = UnitReadyTimeStatus::Waiting_For_Gas;
      if (res_error = ResourceTimeline::Insufficient_Minerals)
        reason = UnitReadyTimeStatus::Waiting_For_Minerals;
      t=t2;
    }
    if (t==-1) return -1;
  }
  std::map<UnitType, int> req_units = task.getRequiredUnits();
  for each(std::pair<UnitType, int> r in req_units)
  {
    int t2=TheMacroManager->uctl.getFirstTime(r.first,r.second);
    if (t2==-1 || t2>t)
    {
      reason = UnitReadyTimeStatus::Waiting_For_Required_Units;
      t=t2;
    }
    if (t==-1) return -1;
  }
  return t;
}