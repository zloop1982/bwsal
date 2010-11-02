#include "MacroAIModule.h"
#include <BasicTaskExecutor.h>
#include <SpiralBuildingPlacer.h>
#include <UnitPump.h>
#include <TerminateIfWorkerLost.h>
#include <TerminateIfEmpty.h>
#include <BasicWorkerFinder.h>
#include <UnitCompositionProducer.h>
#include <MacroManager.h>
#include <ResourceRates.h>
#include <MacroSupplyManager.h>

using namespace BWAPI;
int drag_index = -1;
bool lastMouseClick = false;
UnitCompositionProducer* infantryProducer = NULL;
MacroAIModule::MacroAIModule()
{
}
MacroAIModule::~MacroAIModule()
{
  if (TheMacroManager != NULL)
    delete TheMacroManager;
  if (TheMacroSupplyManager != NULL)
    delete TheMacroSupplyManager;
  if (TheResourceRates != NULL)
    delete TheResourceRates;
}
void MacroAIModule::onStart()
{
  Broodwar->enableFlag(Flag::UserInput);
  MacroManager::create(&arbitrator);
  MacroSupplyManager::create();
  ResourceRates::create();

  TaskStream* ts = new TaskStream();
  TheMacroManager->taskStreams.push_back(ts);
  Unit* worker = NULL;
  for each(Unit* u in Broodwar->self()->getUnits())
  {
    if (u->getType()==UnitTypes::Terran_Command_Center)
      worker = u;
  }
  ts->setWorker(worker);
  ts->attach(BasicTaskExecutor::getInstance(),false);
  ts->attach(new UnitPump(UnitTypes::Terran_SCV),true);
  ts->attach(new TerminateIfWorkerLost(),true);
  infantryProducer = new UnitCompositionProducer(UnitTypes::Terran_Barracks);
  infantryProducer->setUnitWeight(UnitTypes::Terran_Marine,2.0);
  infantryProducer->setUnitWeight(UnitTypes::Terran_Medic,1.0);
  infantryProducer->setUnitWeight(UnitTypes::Terran_Firebat,0.5);
}
void MacroAIModule::onEnd(bool isWinner)
{
}
void MacroAIModule::onFrame()
{
  TheArbitrator->update();
  infantryProducer->update();
  TheMacroSupplyManager->update();
  TheMacroManager->update();
  TheResourceRates->update();
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
  if (drag_index<0)
  {
    if (Broodwar->getMouseState(M_LEFT) && !lastMouseClick)
    {
      drag_index = (Broodwar->getMouseY()-25)/20;
      if (drag_index<0) drag_index = 0;
    }
    if (drag_index>=(int)TheMacroManager->taskStreams.size())
      drag_index=(int)TheMacroManager->taskStreams.size()-1;
  }
  if (drag_index>=0)
  {
    int land_index = (Broodwar->getMouseY()-25)/20;
    if (land_index<0) land_index = 0;
    if (land_index>=(int)TheMacroManager->taskStreams.size())
      land_index=(int)TheMacroManager->taskStreams.size()-1;
    if (land_index!=drag_index)
    {
      std::list<TaskStream*>::iterator td=TheMacroManager->taskStreams.end();
      std::list<TaskStream*>::iterator tl=TheMacroManager->taskStreams.end();
      TaskStream* tm=NULL;
      int j=0;
      for(std::list<TaskStream*>::iterator i=TheMacroManager->taskStreams.begin();i!=TheMacroManager->taskStreams.end();i++)
      {
        if (j==drag_index)
          td=i;
        if (j==land_index)
          tl=i;
        j++;
      }
      if (td!=TheMacroManager->taskStreams.end() && tl!=TheMacroManager->taskStreams.end())
      {
        tm=*td;
        *td=*tl;
        *tl=tm;
      }
      drag_index = land_index;
    }
    if (!Broodwar->getMouseState(M_LEFT) && lastMouseClick)
    {
      drag_index=-1;
    }
  }
  lastMouseClick = Broodwar->getMouseState(M_LEFT);
}
void MacroAIModule::onSendText(std::string text)
{
  Broodwar->sendText(text.c_str());
  
  UnitType type=UnitTypes::getUnitType(text);
  if (type!=UnitTypes::Unknown)
  {
    TaskStream* ts = new TaskStream(Task(type));
    TheMacroManager->taskStreams.push_back(ts);
    ts->attach(new BasicWorkerFinder(),true);
    ts->attach(BasicTaskExecutor::getInstance(),false);
    ts->attach(new TerminateIfEmpty(),true);
    ts->attach(SpiralBuildingPlacer::getInstance(),false);
  }
  else
  {
    TechType type=TechTypes::getTechType(text);
    if (type!=TechTypes::Unknown)
    {
      TaskStream* ts = new TaskStream(Task(type));
      TheMacroManager->taskStreams.push_back(ts);
      ts->attach(new BasicWorkerFinder(),true);
      ts->attach(BasicTaskExecutor::getInstance(),false);
      ts->attach(new TerminateIfEmpty(),true);
      ts->attach(SpiralBuildingPlacer::getInstance(),false);
    }
    else
    {
      UpgradeType type=UpgradeTypes::getUpgradeType(text);
      if (type!=UpgradeTypes::Unknown)
      {
        TaskStream* ts = new TaskStream(Task(type));
        TheMacroManager->taskStreams.push_back(ts);
        ts->attach(new BasicWorkerFinder(),true);
        ts->attach(BasicTaskExecutor::getInstance(),false);
        ts->attach(new TerminateIfEmpty(),true);
        ts->attach(SpiralBuildingPlacer::getInstance(),false);
      }
      else
        Broodwar->printf("You typed '%s'!",text.c_str());
    }
  }
}
void MacroAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  TheArbitrator->onRemoveObject(unit);
}
