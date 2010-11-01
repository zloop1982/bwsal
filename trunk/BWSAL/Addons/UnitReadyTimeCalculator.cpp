#include <MacroManager/UnitReadyTimeCalculator.h>
#include <MacroManager.h>
using namespace BWAPI;
using namespace std;
map<BWAPI::UnitType, set<BWAPI::UnitType> > makes;
map<BWAPI::UnitType, set<BWAPI::TechType> > researches;
map<BWAPI::UnitType, set<BWAPI::UpgradeType> > upgrades;

void UnitReadyTimeCalculator::init()
{
  for(set<BWAPI::UnitType>::iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
  {
    makes[(*i).whatBuilds().first].insert(*i);
  }
  for(set<BWAPI::TechType>::iterator i=TechTypes::allTechTypes().begin();i!=TechTypes::allTechTypes().end();i++)
  {
    researches[i->whatResearches()].insert(*i);
  }
  for(set<BWAPI::UpgradeType>::iterator i=UpgradeTypes::allUpgradeTypes().begin();i!=UpgradeTypes::allUpgradeTypes().end();i++)
  {
    upgrades[i->whatUpgrades()].insert(*i);
  }
}
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
      if (res_error == ResourceTimeline::Insufficient_Supply)
        reason = UnitReadyTimeStatus::Waiting_For_Supply;
      if (res_error == ResourceTimeline::Insufficient_Gas)
        reason = UnitReadyTimeStatus::Waiting_For_Gas;
      if (res_error == ResourceTimeline::Insufficient_Minerals)
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
  if (task.getType()==TaskTypes::Unit && task.getUnit()==UnitTypes::Zerg_Lurker)
  {
    int t2=TheMacroManager->ttl.getFinishTime(TechTypes::Lurker_Aspect);
    if (t2==-1 || t2>t)
    {
      reason = UnitReadyTimeStatus::Waiting_For_Required_Tech;
      t=t2;
    }
    if (t==-1) return -1;
  }
  if (task.getType()==TaskTypes::Upgrade)
  {
    int t2=TheMacroManager->utl.getFirstTime(task.getUpgrade(),task.getLevel()-1);
    if (t2==-1 || t2>t)
    {
      reason = UnitReadyTimeStatus::Waiting_For_Required_Upgrade;
      t=t2;
    }
    if (t==-1) return -1;
  }
  return t;
}
std::set<BWAPI::UnitType> UnitReadyTimeCalculator::getPossibleUnitTypes(BWAPI::Unit* unit, int time, bool considerResources, bool considerTasks)
{
  set<UnitType> unitTypes;
  int t = getReadyTime(unit,considerTasks);
  if (t==-1 || t>time) return unitTypes;
  UnitType workerType = unit->getType();
  if (unit->isMorphing())
    workerType=unit->getBuildType();
  UnitReadyTimeStatus::Enum r;
  for each(UnitType type in makes[workerType])
  {
    if (getReadyTime(unit,Task(type),r,considerResources,considerTasks)<time)
    {
      unitTypes.insert(type);
    }
  }
  return unitTypes;
}