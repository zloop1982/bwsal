#include "MacroAIModule.h"
#include <BasicTaskExecutor.h>
#include <UnitPump.h>
#include <TerminateIfWorkerLost.h>
using namespace BWAPI;
MacroAIModule::MacroAIModule()
{
  macroManager = NULL;
}
MacroAIModule::~MacroAIModule()
{
  if (macroManager != NULL)
    delete macroManager;
  macroManager = NULL;
}
void MacroAIModule::onStart()
{
  Broodwar->enableFlag(Flag::UserInput);
  macroManager = new MacroManager(&arbitrator);
  TaskStream* ts = new TaskStream();
  macroManager->taskStreams.push_back(ts);
  Unit* worker = NULL;
  for each(Unit* u in Broodwar->self()->getUnits())
  {
    if (u->getType()==UnitTypes::Terran_Command_Center)
      worker = u;
  }
  ts->setWorker(worker);
  ts->attach(new BasicTaskExecutor(),false);
  ts->attach(new UnitPump(UnitTypes::Terran_SCV),true);
  ts->attach(new TerminateIfWorkerLost(),true);
}
void MacroAIModule::onEnd(bool isWinner)
{
}
void MacroAIModule::onFrame()
{
  macroManager->update();
  arbitrator.update();
  std::set<Unit*> units=Broodwar->self()->getUnits();
  for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
  {
    if (this->arbitrator.hasBid(*i))
    {
      int x=(*i)->getPosition().x();
      int y=(*i)->getPosition().y();
      std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bids=this->arbitrator.getAllBidders(*i);
      int y_off=0;
      bool first = false;
      const char activeColor = '\x07', inactiveColor = '\x16';
      char color = activeColor;
      for(std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end();j++)
      {
        Broodwar->drawTextMap(x,y+y_off,"%c%s: %d",color,j->first->getShortName().c_str(),(int)j->second);
        y_off+=15;
        color = inactiveColor;
      }
    }
  }
}
void MacroAIModule::onSendText(std::string text)
{
  Broodwar->sendText(text.c_str());
  if (text=="supply")
  {
    TaskStream* ts = new TaskStream(Task(UnitTypes::Terran_Supply_Depot));
    TilePosition tp=Broodwar->self()->getStartLocation();
    if (tp.x()<50)
      tp.x()+=7;
    else
      tp.x()-=7;
    ts->getTask().setTilePosition(tp);
    macroManager->taskStreams.push_back(ts);
    Unit* worker = NULL;
    for each(Unit* u in Broodwar->self()->getUnits())
    {
      if (u->getType()==UnitTypes::Terran_SCV)
        worker = u;
    }
    ts->setWorker(worker);
    ts->attach(new BasicTaskExecutor(),false);
  }
}