#include "BasicAIModule.h"
using namespace BWAPI;

void BasicAIModule::onStart()
{
  if (Broodwar->inReplay()) return;
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  Broodwar->enableFlag(Flag::CompleteMapInformation);
  BWTA::analyze();
  this->constructionManager=new ConstructionManager(&this->arbitrator,&placer);
  this->supplyManager=new SupplyManager(this->constructionManager);
  this->baseManager=new BaseManager(this->constructionManager);
  this->baseManager->addBase(BWTA::getStartLocation(BWAPI::Broodwar->self()));
  this->workerManager=new WorkerManager(&this->arbitrator,baseManager);

  Broodwar->printf("Hello world!");
  Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
}
void BasicAIModule::onFrame()
{
  if (Broodwar->inReplay()) return;

  baseManager->update();
  workerManager->update();
  constructionManager->update();
  supplyManager->update();
  arbitrator.update();
  std::set<Unit*> units=Broodwar->self()->getUnits();

  //Basic worker pump
  for(std::set<Unit*>::iterator u=units.begin();u!=units.end();u++)
  {
    if ((*u)->getType()==*(Broodwar->self()->getRace().getWorker()->whatBuilds().first))
    {
      if (!(*u)->isTraining())
      {
        (*u)->train(*(Broodwar->self()->getRace().getWorker()));
      }
    }
  }
  //we will iterate through all the base locations, and draw their outlines.
  for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
  {
    TilePosition p=(*i)->getTilePosition();
    Position c=(*i)->getPosition();

    //draw outline of center location
    Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

    //draw a circle at each mineral patch
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getMinerals().begin();j!=(*i)->getMinerals().end();j++)
    {
      Position q=(*j)->getPosition();
      Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
    }

    //draw the outlines of vespene geysers
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
    {
      TilePosition q=(*j)->getTilePosition();
      Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
    }

    //if this is an island expansion, draw a yellow circle around the base location
    if ((*i)->isIsland())
    {
      Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
    }
  }
    
  //we will iterate through all the regions and draw the polygon outline of it in green.
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    BWTA::Polygon p=(*r)->getPolygon();
    for(int j=0;j<(int)p.size();j++)
    {
      Position point1=p[j];
      Position point2=p[(j+1) % p.size()];
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
    }
  }

  //we will visualize the chokepoints with red lines
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
    {
      Position point1=(*c)->getSides().first;
      Position point2=(*c)->getSides().second;
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
    }
  }
}

void BasicAIModule::onRemoveUnit(BWAPI::Unit* unit)
{
  this->arbitrator.onRemoveObject(unit);
  this->constructionManager->onRemoveUnit(unit);
  this->supplyManager->onRemoveUnit(unit);
}
bool BasicAIModule::onSendText(std::string text)
{
  UnitType type=UnitTypes::getUnitType(text);
  if (type!=UnitTypes::Unknown)
  {
    this->constructionManager->build(type);
  }
  else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
  }
  return true;
}
