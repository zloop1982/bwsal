#include <UnitCompositionProducer.h>
#include <BasicTaskExecutor.h>
#include <SpiralBuildingPlacer.h>
#include <UnitPump.h>
#include <TerminateIfWorkerLost.h>
#include <TerminateIfEmpty.h>
#include <BasicWorkerFinder.h>
#include <MacroManager/UnitReadyTimeCalculator.h>
using namespace BWAPI;
using namespace std;
std::set<UnitType> requiresAddon;
std::map<UnitType,UnitType> getRequiredAddon;
UnitCompositionProducer::UnitCompositionProducer(BWAPI::UnitType workerType)
{
  this->workerType=workerType;
  if (requiresAddon.empty())
  {
    for each(UnitType t in UnitTypes::allUnitTypes())
    {
      for each(std::pair<UnitType, int> r in t.requiredUnits())
      {
        if (r.first.isAddon() && r.first.whatBuilds().first==t.whatBuilds().first)
        {
          requiresAddon.insert(t);
          getRequiredAddon[t]=r.first;
        }
      }
    }
  }
}
void UnitCompositionProducer::attached(TaskStream* ts)
{
  streams.insert(ts);
}
void UnitCompositionProducer::detached(TaskStream* ts)
{
  streams.erase(ts);
}
void UnitCompositionProducer::update()
{
  std::set<Unit*> units=Broodwar->self()->getUnits();
  for each(Unit* u in units)
  {
    if (u->getType()==workerType && u->exists() && u->isCompleted())
    {
      TaskStream* ts = TheMacroManager->getTaskStream(u);
      if (ts==NULL)
      {
        ts = new TaskStream();
        TheMacroManager->taskStreams.push_back(ts);
        ts->setWorker(u);
        ts->attach(BasicTaskExecutor::getInstance(),false);
        ts->attach(this,false);
        ts->attach(new TerminateIfWorkerLost(),true);
      }
    }
  }

  actualUnitCounts.clear();
  for each(std::pair<UnitType, double> t in unitCompositionWeights)
  {
    actualUnitCounts[t.first]=TheMacroManager->uctl.getFinalCount(t.first);
  }
  for each(TaskStream* ts in streams)
  {
    if (ts->getTask()==NULL)
    {
      UnitType t=getNextUnitType(ts->getWorker());
      ts->setTask(Task(t,ts->getWorker()->getTilePosition()));
      actualUnitCounts[t]++;
    }
    else if (ts->getNextTask()==NULL)
    {
      UnitType t=getNextUnitType(ts->getWorker());
      ts->setNextTask(Task(t,ts->getWorker()->getTilePosition()));
      actualUnitCounts[t]++;
    }
  }
}
UnitType UnitCompositionProducer::getNextUnitType(BWAPI::Unit* worker)
{
  double aTc = 0;
  double cTc = 0;
  for each(std::pair<UnitType, double> t in unitCompositionWeights)
  {
    aTc+=actualUnitCounts[t.first]*t.second;
    cTc+=t.second*t.second;
  }
  double r= aTc/cTc;
  for each(std::pair<UnitType, double> t in unitCompositionWeights)
  {
    desiredUnitCounts[t.first]=t.second*r;
  }
  double maxDiff=-10000;
  UnitType nextType;
  int time = UnitReadyTimeCalculator::getReadyTime(worker,true);
  for each(std::pair<UnitType, double> t in unitCompositionWeights)
  {
    UnitType typ = t.first;
    double d=desiredUnitCounts[t.first]-actualUnitCounts[typ];
    UnitReadyTimeStatus::Enum r;
    int t2=UnitReadyTimeCalculator::getReadyTime(worker,Task(typ),r,false,true);
    if (t2<0 || t2>time)
    {
      if (t2<0 && requiresAddon.find(typ)!=requiresAddon.end())
      {
        typ = getRequiredAddon[typ];
      }
      else
      {
        continue;
      }
    }
    if (d>maxDiff)
    {
      maxDiff  = d;
      nextType = typ;
    }
  }
  return nextType;
}
void UnitCompositionProducer::setUnitWeight(BWAPI::UnitType t, double weight)
{
  unitCompositionWeights[t]=weight;
}