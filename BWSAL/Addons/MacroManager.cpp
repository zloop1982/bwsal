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
}
void MacroManager::update()
{
  rtl.reset(Resources(Broodwar->self()),TheResourceRates->getGatherRate().getMinerals(),TheResourceRates->getGatherRate().getGas());
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
  Broodwar->drawTextScreen(452,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getMinerals()*23*60));
  Broodwar->drawTextScreen(520,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getGas()*23*60));
  killSet.clear();
  int y=20;
  Broodwar->drawTextScreen(10,0,"Frame: %d",Broodwar->getFrameCount());
  for each(TaskStream* ts in taskStreams)
  {
    //update this task stream
    ts->update();
    ts->printToScreen(10,y);
    y+=20;
  }
}