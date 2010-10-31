#include <MacroSupplyManager.h>
#include <MacroManager.h>
#include <BWAPI.h>
#include <ResourceRates.h>
#include <math.h>
#include <BasicTaskExecutor.h>
#include <SpiralBuildingPlacer.h>
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
      int productionCapacity       = 0;
      int supplyBuildTime = BWAPI::Broodwar->self()->getRace().getSupplyProvider().buildTime();
      int time = BWAPI::Broodwar->getFrameCount() + supplyBuildTime;
      for each(UnitType t in factoryTypes)
      {
        productionCapacity+=TheMacroManager->uctl.getFinalCount(t)*4;
      }

      if (productionCapacity>=TheMacroManager->rtl.getFinalSupply())
      {
        TaskStream* ts = new TaskStream(Task(Broodwar->self()->getRace().getSupplyProvider()));
        TheMacroManager->taskStreams.push_front(ts);
        ts->attach(new BasicWorkerFinder(),true);
        ts->attach(BasicTaskExecutor::getInstance(),false);
        ts->attach(new TerminateIfEmpty(),true);
        ts->attach(SpiralBuildingPlacer::getInstance(),false);
      }
    }
  }
}