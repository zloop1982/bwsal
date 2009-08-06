#include "ConstructionManager.h"

ConstructionManager::ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer)
{
  this->arbitrator = arbitrator;
  this->placer     = placer;
}

void ConstructionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  std::set<Building*> buildingsWithNewBuilders;
  for(std::set<Building*>::iterator b = buildingsNeedingBuilders.begin(); b != buildingsNeedingBuilders.end(); b++)
  {
    double min_dist = 1000000;
    BWAPI::Unit* builder = NULL;
    for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
    {
      double dist = (*u)->getPosition().getDistance((*b)->position);
      if (dist < min_dist)
      {
        min_dist = dist;
        builder  = *u;
      }
    }
    if (builder != NULL)
    {
      arbitrator->accept(this, builder);
      arbitrator->setBid(this, builder, 100.0);
      builders.insert(std::make_pair(builder, *b));
      (*b)->builderUnit = builder;
      units.erase(builder);
      buildingsWithNewBuilders.insert(*b);
    }
  }
  for(std::set<Building*>::iterator b = buildingsWithNewBuilders.begin(); b != buildingsWithNewBuilders.end(); b++)
    buildingsNeedingBuilders.erase(*b);
  for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
    arbitrator->decline(this, *u, 0);
}

void ConstructionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  this->onRemoveUnit(unit);
}

void ConstructionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  if (builders.find(unit) != builders.end())
  {
    Building* building = builders.find(unit)->second;
    if (building != NULL)
      building->builderUnit = NULL;
    builders.erase(unit);
  }
}

void ConstructionManager::update()
{
  if (!this->buildingsNeedingBuilders.empty())
  {
    std::set<BWAPI::Unit*> myPlayerUnits = BWAPI::Broodwar->self()->getUnits();
    for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
      if ((*u)->isCompleted() && (*u)->getType().isWorker() && this->builders.find(*u)==this->builders.end())
      {
        double min_dist=1000000;
        for(std::set<Building*>::iterator b = buildingsNeedingBuilders.begin(); b != buildingsNeedingBuilders.end(); b++)
        {
          double dist = (*u)->getPosition().getDistance((*b)->position);
          if (dist < min_dist)
            min_dist = dist;
        }
        if (min_dist < 10)
          min_dist = 10;
        if (min_dist > 256*32 + 10)
          min_dist = 256*32 + 10;
        double bid = 80 - (min_dist - 10)/(256*32)*60;
        arbitrator->setBid(this, *u, bid);
      }
  }
  std::list<Building*>::iterator i_next;
  int index=0;
  for(std::list<Building*>::iterator i=this->incompleteBuildings.begin();i!=this->incompleteBuildings.end();i=i_next)
  {
    index++;
    i_next=i;
    i_next++;
    Building* b = *i;
    if (b->tilePosition==BWAPI::TilePositions::None)
    {
      if ((BWAPI::Broodwar->getFrameCount()+index)%25==0 && BWAPI::Broodwar->canMake(NULL,b->type))
      {
        b->tilePosition = this->placer->getBuildLocationNear(BWAPI::Broodwar->self()->getStartLocation(), b->type);
        if (b->tilePosition!=BWAPI::TilePositions::None)
        {
          b->position = BWAPI::Position(b->tilePosition.x()*32 + b->type.tileWidth()*16, b->tilePosition.y()*32 + b->type.tileHeight()*16);
          this->placer->reserveTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
        }
      }
      if (b->tilePosition==BWAPI::TilePositions::None)
        continue;
    }

    if (b->builderUnit!=NULL && !b->builderUnit->exists())
      b->builderUnit=NULL;
    if (b->buildingUnit!=NULL && (!b->buildingUnit->exists() || b->buildingUnit->getType()!=b->type))
      b->buildingUnit=NULL;

    if (b->buildingUnit == NULL)
    {
      std::set<BWAPI::Unit*> unitsOnTile = BWAPI::Broodwar->unitsOnTile(b->tilePosition.x(), b->tilePosition.y());
      for(std::set<BWAPI::Unit*>::iterator t = unitsOnTile.begin(); t != unitsOnTile.end(); t++)
        if ((*t)->getType().isBuilding() && (*t)->getType() == b->type && !(*t)->isCompleted() && !(*t)->isLifted())
        {
          b->buildingUnit = *t;
          break;
        }
      if (b->buildingUnit == NULL && b->builderUnit!=NULL && b->builderUnit->getType().isBuilding())
      {
        b->buildingUnit = b->builderUnit;
      }
    }
    BWAPI::Unit* u = b->builderUnit;
    BWAPI::Unit* s = b->buildingUnit;
    if (s != NULL && s->isCompleted())
    {
      //If the building is complete, we can forget about it.
      this->incompleteBuildings.erase(i);
      if (u != NULL)
      {
        this->builders.erase(u);
        arbitrator->setBid(this,u,0);
      }
      this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
    }
    else
    {
      if (s == NULL)
      {
        if (BWAPI::Broodwar->canMake(NULL,b->type))
        {
          if (u == NULL)
            buildingsNeedingBuilders.insert(b);
          else
          {
            if (!u->isConstructing())
            {
              double distance = u->getPosition().getDistance(b->position);
              if (distance > 100)
                u->rightClick(b->position);
              else
                if (BWAPI::Broodwar->canBuildHere(u, b->tilePosition, b->type))
                {
                  if (BWAPI::Broodwar->canMake(u, b->type))
                    u->build(b->tilePosition, b->type);
                }
                else
                {
                  this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
                  b->tilePosition = BWAPI::TilePositions::None;
                  b->position = BWAPI::Positions::None;
                }
            }
          }
        }
      }
      else
      {
        if (b->type.getRace() != BWAPI::Races::Terran)
        {
          if (u != NULL)
          {
            this->builders.erase(u);
            arbitrator->setBid(this, u, 0);
            b->builderUnit = NULL;  
            u = b->builderUnit;
          }
        }
        else
        {
          if (u == NULL)
            buildingsNeedingBuilders.insert(b);
          else
          {
            if (BWAPI::Broodwar->getFrameCount()%(4*BWAPI::Broodwar->getLatency())==0)
            {
              /*
              int sx=s->getPosition().x();
              int sy=s->getPosition().y();
              int ux=u->getPosition().x();
              int uy=u->getPosition().y();
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,sx,sy,"%s",s->getOrder().getName().c_str());
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,sx,sy+20,"%s",s->getType().getName().c_str());
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,sx,sy+40,"isCompleted: %s",s->isCompleted() ? "true" : "false");
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,sx,sy+60,"isBeingConstructed: %s",s->isBeingConstructed() ? "true" : "false");
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,ux,uy,"%s",u->getOrder().getName().c_str());
              BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,ux,uy+20,"isConstructing: %s",u->isConstructing() ? "true" : "false");
              */
              if (!u->isConstructing() || !s->isBeingConstructed())
              {
                /*
                BWAPI::Broodwar->drawBox(BWAPI::CoordinateType::Map,sx-32,sy-32,sx+32,sy+32,BWAPI::Colors::Green,false);
                BWAPI::Broodwar->drawBox(BWAPI::CoordinateType::Map,ux-32,uy-32,ux+32,uy+32,BWAPI::Colors::Blue,false);
                BWAPI::Broodwar->printf("rightClick: u: 0x%x, s: 0x%x",u,s);
                */
                u->rightClick(s);
              }
            }
          }
        }
      }
      i++;
    }
  }
}

bool ConstructionManager::build(BWAPI::UnitType type)
{
  if (!type.isBuilding()) return false;
  Building* newBuilding     = new Building();
  newBuilding->type         = type;
  newBuilding->tilePosition = BWAPI::TilePositions::None;
  newBuilding->builderUnit  = NULL;
  newBuilding->buildingUnit = NULL;
  newBuilding->position     = BWAPI::Positions::None;
  this->incompleteBuildings.push_back(newBuilding);
  return true;
}

std::string ConstructionManager::getName()
{
  return "Construction Manager";
}
