#include <MacroManager.h>
#include <BWAPI.h>
using namespace BWAPI;
MacroManager* TheMacroManager;
Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;

MacroManager::MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  TheArbitrator = arbitrator;
  TheMacroManager = this;
}
void MacroManager::update()
{
  rtl.reset(Resources(Broodwar->self()),0,0);
  int y=0;
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
  killSet.clear();
  for each(TaskStream* ts in taskStreams)
  {
    //update this task stream
    ts->update();
    ts->printToScreen(10,y);
    y+=20;
  }
}