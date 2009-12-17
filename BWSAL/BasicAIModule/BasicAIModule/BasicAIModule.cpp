#include "BasicAIModule.h"
using namespace BWAPI;

void BasicAIModule::onStart()
{
  this->showManagerAssignments=false;
  if (Broodwar->isReplay()) return;
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  //Broodwar->enableFlag(Flag::CompleteMapInformation);
  BWTA::readMap();
  BWTA::analyze();
  this->analyzed=true;
  this->buildManager      = new BuildManager(&this->arbitrator);
  this->techManager       = new TechManager(&this->arbitrator);
  this->upgradeManager    = new UpgradeManager(&this->arbitrator);
  this->scoutManager      = new ScoutManager(&this->arbitrator);
  this->workerManager     = new WorkerManager(&this->arbitrator);
  this->buildOrderManager = new BuildOrderManager(this->buildManager,this->techManager,this->upgradeManager,this->workerManager);
  this->baseManager       = new BaseManager();
  this->supplyManager     = new SupplyManager();

  this->supplyManager->setBuildManager(this->buildManager);
  this->supplyManager->setBuildOrderManager(this->buildOrderManager);
  this->techManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->upgradeManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->workerManager->setBaseManager(this->baseManager);
  this->baseManager->setBuildOrderManager(this->buildOrderManager);
  
  BWAPI::Race race = Broodwar->self()->getRace();
  BWAPI::Race enemyRace = Broodwar->enemy()->getRace();
  BWAPI::UnitType workerType=*(race.getWorker());
  this->buildOrderManager->enableDependencyResolver();
  this->buildOrderManager->build(20,workerType,80);
  //make the basic production facility
  if (race == Races::Zerg)
  {
    //send an overlord out if Zerg
    this->scoutManager->setScoutCount(1);
    this->buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,60);
    this->buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,120);
  }
  else if (race == Races::Terran)
  {
    if (enemyRace == Races::Zerg)
    {
      this->buildOrderManager->buildAdditional(2,BWAPI::UnitTypes::Terran_Barracks,40);
      this->buildOrderManager->buildAdditional(20,BWAPI::UnitTypes::Terran_Marine,40);
    }
    else
    {
      this->buildOrderManager->buildAdditional(3,BWAPI::UnitTypes::Terran_Factory,60);
      this->buildOrderManager->buildAdditional(2,BWAPI::UnitTypes::Terran_Machine_Shop,70);
      this->buildOrderManager->buildAdditional(20,BWAPI::UnitTypes::Terran_Vulture,40);
      this->buildOrderManager->buildAdditional(20,BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode,40);
    }
  }
  else if (race == Races::Protoss)
  {
    this->buildOrderManager->buildAdditional(10,UnitTypes::Protoss_Dragoon,70);
    this->buildOrderManager->buildAdditional(10,UnitTypes::Protoss_Zealot,70);
    this->buildOrderManager->buildAdditional(20,UnitTypes::Protoss_Carrier,60);
  }
 
}


void BasicAIModule::onFrame()
{
  if (Broodwar->isReplay()) return;
  if (!this->analyzed) return;
  this->buildOrderManager->update();
  this->buildManager->update();
  this->baseManager->update();
  this->workerManager->update();
  this->techManager->update();
  this->upgradeManager->update();
  this->supplyManager->update();
  this->scoutManager->update();
  this->arbitrator.update();
  if (Broodwar->getFrameCount()>24*50)
    scoutManager->setScoutCount(1);

  std::set<Unit*> units=Broodwar->self()->getUnits();
  if (this->showManagerAssignments)
  {
    for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
    {
      if (this->arbitrator.hasBid(*i))
      {
        int x=(*i)->getPosition().x();
        int y=(*i)->getPosition().y();
        std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bids=this->arbitrator.getAllBidders(*i);
        int y_off=0;
        for(std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end();j++)
        {
          Broodwar->drawText(CoordinateType::Map,x,y+y_off,"%s: %d",j->first->getName().c_str(),(int)j->second);
          y_off+=20;
        }
      }
    }
  }
  /*
  for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
  {
    int x=(*i)->getPosition().x();
    int y=(*i)->getPosition().y();
    Broodwar->drawCircle(CoordinateType::Map,x,y,(*i)->getType().airWeapon()->maxRange(),Colors::Blue,false);
    Broodwar->drawCircle(CoordinateType::Map,x,y,(*i)->getType().groundWeapon()->maxRange(),Colors::Green,false);
  }
  */

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

void BasicAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  this->arbitrator.onRemoveObject(unit);
  this->buildManager->onRemoveUnit(unit);
  this->techManager->onRemoveUnit(unit);
  this->upgradeManager->onRemoveUnit(unit);
  this->workerManager->onRemoveUnit(unit);
  this->scoutManager->onRemoveUnit(unit);
}
bool BasicAIModule::onSendText(std::string text)
{
  UnitType type=UnitTypes::getUnitType(text);
  if (text=="debug")
  {
    this->showManagerAssignments=true;
    return true;
  }
  if (text=="expand")
  {
    double minDist;
    BWTA::BaseLocation* natural=NULL;
    BWTA::BaseLocation* home=BWTA::getStartLocation(Broodwar->self());
    for(std::set<BWTA::BaseLocation*>::const_iterator b=BWTA::getBaseLocations().begin();b!=BWTA::getBaseLocations().end();b++)
    {
      if (*b==home) continue;
      double dist=home->getGroundDistance(*b);
      if (dist>0)
      {
        if (natural==NULL || dist<minDist)
        {
          minDist=dist;
          natural=*b;
        }
      }
    }
    Broodwar->printf("expanding to (%d,%d)",natural->getTilePosition().x(),natural->getTilePosition().y());
    this->baseManager->expand(natural);
  }
  if (type!=UnitTypes::Unknown)
  {
    this->buildOrderManager->buildAdditional(1,type,300);
  }
  else
  {
    TechType type=TechTypes::getTechType(text);
    if (type!=TechTypes::Unknown)
    {
      this->techManager->research(type);
    }
    else
    {
      UpgradeType type=UpgradeTypes::getUpgradeType(text);
      if (type!=UpgradeTypes::Unknown)
      {
        this->upgradeManager->upgrade(type);
      }
      else
        Broodwar->printf("You typed '%s'!",text.c_str());
    }
  }
  return true;
}
