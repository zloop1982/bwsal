#include <MacroManager.h>
#include <BWAPI.h>
#include <ResourceRates.h>
using namespace BWAPI;
MacroManager* TheMacroManager = NULL;
Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator = NULL;

MacroManager* MacroManager::create(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  if (TheMacroManager) return TheMacroManager;
  return new MacroManager(arbitrator);
}

MacroManager::MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  TheArbitrator = arbitrator;
  TheMacroManager = this;
}
MacroManager::~MacroManager()
{
  TheMacroManager = NULL;
  for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
    delete *i;
}
void MacroManager::update()
{
  spentResources.setSupply(0);//don't keep track of spent supply
  Resources r = CumulativeResources(Broodwar->self())-spentResources;
  rtl.reset(r,TheResourceRates->getGatherRate().getMinerals(),TheResourceRates->getGatherRate().getGas());
  uctl.reset();
  for each(TaskStream* ts in killSet)
  {
    for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
    {
      if (*i == ts)
      {
        taskStreams.erase(i);
        break;
      }
    }
    delete ts;
  }
  for each(TaskStream* ts in taskStreams)
    ts->clearPlanningData();
  Broodwar->drawTextScreen(452,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getMinerals()*23*60));
  Broodwar->drawTextScreen(520,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getGas()*23*60));
  killSet.clear();
  int y=25;
  Broodwar->drawTextScreen(10,5,"Frame: %d",Broodwar->getFrameCount());
  bool plannedAdditionalResources = true;
  while(plannedAdditionalResources)
  {
    plannedAdditionalResources = false;
    for each(TaskStream* ts in taskStreams)
    {
      if (ts->updateStatus())
      {
        plannedAdditionalResources = true;
        break;
      }
    }
  }
  for each(TaskStream* ts in taskStreams)
    ts->update();

  for each(TaskStream* ts in taskStreams)
  {
    ts->printToScreen(10,y);
    y+=20;
  }
  for(std::map<int, Resources>::iterator i=rtl.resourceEvents.begin();i!=rtl.resourceEvents.end();i++)
  {
    Broodwar->drawTextScreen(10,y,"%d: (m:%f, g:%f, s:%f)",(*i).first,(*i).second.getMinerals(),(*i).second.getGas(),(*i).second.getSupply());
    y+=20;
  }
}