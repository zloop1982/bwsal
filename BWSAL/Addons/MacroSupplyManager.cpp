#include <MacroSupplyManager.h>
#include <MacroManager.h>
#include <BWAPI.h>
#include <ResourceRates.h>
#include <math.h>
#include <BasicTaskExecutor.h>
#include <SpiralBuildingPlacer.h>
#include <BFSBuildingPlacer.h>
#include <UnitPump.h>
#include <TerminateIfWorkerLost.h>
#include <TerminateIfEmpty.h>
#include <BasicWorkerFinder.h>
#include <UnitCompositionProducer.h>
using namespace BWAPI;
using namespace std;
set<UnitType> factoryTypes;
MacroSupplyManager* TheMacroSupplyManager = NULL;
MacroSupplyManager* MacroSupplyManager::create()
{
  if (TheMacroSupplyManager) return TheMacroSupplyManager;
  return new MacroSupplyManager();
}
MacroSupplyManager::MacroSupplyManager()
{
  TheMacroSupplyManager=this;
  for each(UnitType t in UnitTypes::allUnitTypes())
  {
    if (t.canProduce())
    {
      factoryTypes.insert(t);
    }
  }
  initialSupplyTotal = Broodwar->self()->supplyTotal();
  lastFrameCheck=0;
}
MacroSupplyManager::~MacroSupplyManager()
{
  TheMacroSupplyManager = NULL;
}
void MacroSupplyManager::update()
{
  if (TheResourceRates->getGatherRate().getMinerals()>0)
  {
    if (Broodwar->getFrameCount()>lastFrameCheck+25)
    {
      lastFrameCheck=Broodwar->getFrameCount();
      if (TheMacroManager->uctl.getFinalCount(BWAPI::Broodwar->self()->getRace().getSupplyProvider())==0)
      {
        if (Broodwar->self()->supplyUsed()>=initialSupplyTotal-2)
        {
          Task s(Broodwar->self()->getRace().getSupplyProvider());
          TaskStream* ts = new TaskStream(s);
          TheMacroManager->taskStreams.push_front(ts);
          ts->attach(new BasicWorkerFinder(),true);
          ts->attach(BasicTaskExecutor::getInstance(),false);
          ts->attach(new TerminateIfEmpty(),true);
          ts->attach(BFSBuildingPlacer::getInstance(),false);
        }
      }
      else
      {
        int supplyBuildTime = BWAPI::Broodwar->self()->getRace().getSupplyProvider().buildTime();
        if (TheMacroManager->rtl.getAvailableResourcesAtTime(Broodwar->getFrameCount()+supplyBuildTime*3).getSupply()<=0)
        {
          if (TheMacroManager->rtl.getFinalSupplyTotal()<400)
          {
            Task s(Broodwar->self()->getRace().getSupplyProvider());
            int frame = TheMacroManager->rtl.getFirstTimeWhenSupplyIsNoGreaterThan(0);
            s.setEarliestStartTime(frame-29*10-supplyBuildTime);
            TaskStream* ts = new TaskStream(s);
            TheMacroManager->taskStreams.push_front(ts);
            ts->attach(new BasicWorkerFinder(),true);
            ts->attach(BasicTaskExecutor::getInstance(),false);
            ts->attach(new TerminateIfEmpty(),true);
            ts->attach(BFSBuildingPlacer::getInstance(),false);
          }
        }
      }
    }
  }
}