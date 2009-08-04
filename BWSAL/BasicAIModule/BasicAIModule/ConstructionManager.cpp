#include "ConstructionManager.h"

ConstructionManager::ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer)
{
  this->arbitrator=arbitrator;
  this->placer=placer;
}
void ConstructionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  std::set<Building*> buildingsWithNewBuilders;
  for(std::set<Building*>::iterator b=buildingsNeedingBuilders.begin();b!=buildingsNeedingBuilders.end();b++)
  {
    double min_dist=1000000;
    BWAPI::Unit* builder=NULL;
    for(std::set<BWAPI::Unit*>::iterator u=units.begin();u!=units.end();u++)
    {
      double dist=(*u)->getPosition().getDistance((*b)->position);
      if (dist<min_dist)
      {
        min_dist=dist;
        builder=*u;
      }
    }
    if (builder!=NULL)
    {
      arbitrator->accept(this,builder);
      arbitrator->setBid(this,builder,100.0);
      builders.insert(std::make_pair(builder,*b));
      (*b)->builderUnit=builder;
      units.erase(builder);
      buildingsWithNewBuilders.insert(*b);
    }
  }
  for(std::set<Building*>::iterator b=buildingsWithNewBuilders.begin();b!=buildingsWithNewBuilders.end();b++)
  {
    buildingsNeedingBuilders.erase(*b);
  }
  for(std::set<BWAPI::Unit*>::iterator u=units.begin();u!=units.end();u++)
  {
    arbitrator->decline(this,*u,0);
  }
}
void ConstructionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  if (builders.find(unit)!=builders.end())
  {
    Building* building=builders.find(unit)->second;
    if (building!=NULL)
    {
      building->builderUnit=NULL;
    }
    builders.erase(unit);
  }
}
void ConstructionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  if (builders.find(unit)!=builders.end())
  {
    Building* building=builders.find(unit)->second;
    if (building!=NULL)
    {
      building->builderUnit=NULL;
    }
    builders.erase(unit);
  }
  for(int i=0;i<(int)this->incompleteBuildings.size();i++)
  {
    if (unit==incompleteBuildings[i]->buildingUnit)
    {
      incompleteBuildings[i]->buildingUnit=NULL;
    }
  }
}

void ConstructionManager::update()
{
  if (!this->buildingsNeedingBuilders.empty())
  {
    std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
    for(std::set<BWAPI::Unit*>::iterator u=myPlayerUnits.begin();u!=myPlayerUnits.end();u++)
    {
      if ((*u)->getType().isWorker())
      {
        double min_dist=1000000;
        for(std::set<Building*>::iterator b=buildingsNeedingBuilders.begin();b!=buildingsNeedingBuilders.end();b++)
        {
          double dist=(*u)->getPosition().getDistance((*b)->position);
          if (dist<min_dist)
          {
            min_dist=dist;
          }
        }
        if (min_dist<10) min_dist=10;
        if (min_dist>256*32+10) min_dist=256*32+10;
        double bid=80-(min_dist-10)/(256*32)*60;
        arbitrator->setBid(this,*u,bid);
      }
    }
  }
  for(int i=0;i<(int)this->incompleteBuildings.size();)
  {
    Building* b=this->incompleteBuildings[i];
    BWAPI::Unit* u=b->builderUnit;
    BWAPI::Unit* s=b->buildingUnit;
    if (s!=NULL && s->isCompleted())
    {
      //If the building is complete, we can forget about it.
      this->incompleteBuildings.erase(this->incompleteBuildings.begin()+i);
      if (u!=NULL)
      {
        this->builders.erase(u);
        arbitrator->setBid(this,u,0);
      }
      this->placer->freeTiles(b->tilePosition,b->type.tileWidth(),b->type.tileHeight());
    }
    else
    {
      if (s==NULL)
      {
        std::set<BWAPI::Unit*> unitsOnTile=BWAPI::Broodwar->unitsOnTile(b->tilePosition.x(),b->tilePosition.y());
        for(std::set<BWAPI::Unit*>::iterator t=unitsOnTile.begin();t!=unitsOnTile.end();t++)
        {
          if ((*t)->getType().isBuilding() && (*t)->getType()==b->type && !(*t)->isCompleted())
          {
            b->buildingUnit=*t;
            s=b->buildingUnit;
            break;
          }
        }
      }
      if (s==NULL)
      {
        if (u==NULL)
        {
          buildingsNeedingBuilders.insert(b);
        }
        else
        {
          if (u->getType().isBuilding())
          {
            b->buildingUnit=u;
            s=b->buildingUnit;
          }
          else
          {
            if (!u->isConstructing())
            {
              double distance=u->getPosition().getDistance(b->position);
              if (distance>100)
              {
                u->rightClick(b->position);
              }
              else
              {
                if (BWAPI::Broodwar->getFrameCount()%4)
                {
                  if (BWAPI::Broodwar->canBuildHere(u,b->tilePosition,b->type))
                  {
                    if (BWAPI::Broodwar->self()->minerals()>=b->type.mineralPrice() &&
                        BWAPI::Broodwar->self()->gas()>=b->type.gasPrice())
                    {
                      u->build(b->tilePosition,b->type);
                    }
                  }
                  else
                  {
                    this->placer->freeTiles(b->tilePosition,b->type.tileWidth(),b->type.tileHeight());
                    b->tilePosition=this->placer->getBuildLocationNear(b->tilePosition,b->type);
                    b->position=BWAPI::Position(b->tilePosition.x()*32+b->type.tileWidth()*16,b->tilePosition.y()*32+b->type.tileHeight()*16);
                    this->placer->reserveTiles(b->tilePosition,b->type.tileWidth(),b->type.tileHeight());
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        if (s->getType().getRace()!=BWAPI::Races::Terran)
        {
          if (u!=NULL)
          {
            this->builders.erase(u);
            arbitrator->setBid(this,u,0);
            b->builderUnit=NULL;           
          }
        }
        else
        {
          if (u==NULL)
          {
            buildingsNeedingBuilders.insert(b);
          }
          else
          {
            if (!u->isConstructing() || u->getBuildUnit()!=s)
            {
              if (u->getBuildUnit()!=s)
              {
                u->stop();
              }
              u->rightClick(s);
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
  if (type.getRace()!=BWAPI::Broodwar->self()->getRace())
  {
    return false;
  }
  BWAPI::TilePosition buildLocation=this->placer->getBuildLocationNear(BWAPI::Broodwar->self()->getStartLocation(),type);
  if (buildLocation==BWAPI::TilePositions::None)
  {
    return false;
  }
  Building* newBuilding=new Building();
  newBuilding->type=type;
  newBuilding->tilePosition=buildLocation;
  newBuilding->builderUnit=NULL;
  newBuilding->buildingUnit=NULL;
  newBuilding->position=BWAPI::Position(newBuilding->tilePosition.x()*32+type.tileWidth()*16,newBuilding->tilePosition.y()*32+type.tileHeight()*16);
  this->placer->reserveTiles(newBuilding->tilePosition,type.tileWidth(),type.tileHeight());
  this->incompleteBuildings.push_back(newBuilding);
  return true;
}
std::string ConstructionManager::getName()
{
  return "Construction Manager";
}
