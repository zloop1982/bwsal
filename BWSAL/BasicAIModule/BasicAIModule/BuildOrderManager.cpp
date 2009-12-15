#include <BuildOrderManager.h>
#include <BuildManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <WorkerManager.h>
#include <algorithm>
using namespace BWAPI;
BuildOrderManager::BuildOrderManager(BuildManager* buildManager, TechManager* techManager, UpgradeManager* upgradeManager, WorkerManager* workerManager)
{
  this->buildManager   = buildManager;
  this->techManager    = techManager;
  this->upgradeManager = upgradeManager;
  this->workerManager  = workerManager;
  this->usedMinerals   = 0;
  this->usedGas        = 0;
}
std::map<const Unit*,int> nextAvailableTimeData;
int nextAvailableTime(const Unit* unit)
{
  int time=Broodwar->getFrameCount();
  int ntime=0;
  int mtime=time;
  if (nextAvailableTimeData.find(unit)!=nextAvailableTimeData.end())
    ntime=nextAvailableTimeData[unit];
  if (!unit->isCompleted())
  {
    if (unit->isBeingConstructed())
      return time+unit->getRemainingBuildTime();
    else
      return -1;
  }
  if (unit->isTraining())
    mtime = time+unit->getRemainingTrainTime();
  if (unit->isResearching())
    mtime = time+unit->getRemainingResearchTime();
  if (unit->isUpgrading())
    mtime = time+unit->getRemainingUpgradeTime();
  if (ntime<mtime) return mtime;
  return ntime;
}

void BuildOrderManager::reserveResources(Unit* builder, UnitType unitType)
{
  int t=Broodwar->getFrameCount();
  if (builder)
    t=nextAvailableTime(builder);
  this->reservedMinerals[t]+=unitType.mineralPrice();
  this->reservedGas[t]+=unitType.gasPrice();  
}
void BuildOrderManager::unreserveResources(Unit* builder, UnitType unitType)
{
  int t=Broodwar->getFrameCount();
  if (builder)
    t=nextAvailableTime(builder);
  this->reservedMinerals[t]-=unitType.mineralPrice();
  this->reservedGas[t]-=unitType.gasPrice();  
}
bool unitCompare(const Unit* a, const Unit* b)
{
  return nextAvailableTime(a)<nextAvailableTime(b);
}
void BuildOrderManager::update()
{
  if (items.empty()) return;
  std::map< int, PriorityLevel >::iterator l=items.end();
  l--;
  while (l->second.techs.empty() &&
         l->second.upgrades.empty() &&
         l->second.buildings.empty() &&
         l->second.units.empty())
  {
    items.erase(l);
    if (items.empty()) return;
    l=items.end();
    l--;
  }
  this->reservedMinerals.clear();
  this->reservedGas.clear();
  int y=5;
  BWAPI::Broodwar->drawTextScreen(5,y,"Mineral Rate: %f",this->workerManager->getMineralRate());y+=20;
  BWAPI::Broodwar->drawTextScreen(5,y,"Gas Rate: %f",this->workerManager->getGasRate());y+=20;

  //Iterate through priority levels in decreasing order
  for(;l!=items.end();l--)
  {
    BWAPI::Broodwar->drawTextScreen(5,y,"Priority: %d",l->first);y+=20;
    //First consider all techs for this priority level
    for(std::list<BuildItem>::iterator i=l->second.techs.begin();i!=l->second.techs.end();i++)
    {
    }
    //Next consider all upgrades for this priority level
    for(std::list<BuildItem>::iterator i=l->second.upgrades.begin();i!=l->second.upgrades.end();i++)
    {
    }
    //Next consider all buildings for this priority level
    std::map<UnitType, std::map<UnitType,int> > buildings;
    for(std::list<BuildItem>::iterator i=l->second.buildings.begin();i!=l->second.buildings.end();i++)
    {
      bool ok=true;
      for(std::map<const UnitType*,int>::const_iterator j=i->unitType.requiredUnits().begin();j!=i->unitType.requiredUnits().end();j++)
      {
        if (Broodwar->self()->completedUnitCount(*j->first)==0)
        {
          ok=false;
          break;
        }
      }
      if (ok)
        buildings[*i->unitType.whatBuilds().first][i->unitType]+=i->count;
    }
    for(std::map<UnitType, std::map<UnitType,int> >::iterator i=buildings.begin();i!=buildings.end();i++)
    {
      std::map<UnitType,int>::iterator j2;
      for(std::map<UnitType,int>::iterator j=i->second.begin();j!=i->second.end();j=j2)
      {
        BWAPI::Broodwar->drawTextScreen(5,y,"considering %s",j->first.getName().c_str());y+=20;
        j2=j;
        j2++;
        if (hasResources(j->first))
        {
          for(std::list<BuildItem>::iterator k=l->second.buildings.begin();k!=l->second.buildings.end();k++)
          {
            if (k->unitType==j->first)
            {
              if (k->isAdditional)
              {
                k->count--;
                if (k->count==0)
                {
                  l->second.buildings.erase(k);
                }
              }
              break;
            }
          }
          this->spendResources(j->first);
          this->buildManager->build(j->first);
          j->second--;
          if (j->second==0)
          {
            i->second.erase(j);
          }
        }
        else
        {
          BWAPI::Broodwar->drawTextScreen(5,y,"resource-limited");y+=20;
          return;
        }
      }
    }

    //Finally consider all units for this priority level
    std::map<UnitType, std::map<UnitType,int> > units;
    for(std::list<BuildItem>::iterator i=l->second.units.begin();i!=l->second.units.end();i++)
    {
      bool ok=true;
      for(std::map<const UnitType*,int>::const_iterator j=i->unitType.requiredUnits().begin();j!=i->unitType.requiredUnits().end();j++)
      {
        if (Broodwar->self()->completedUnitCount(*j->first)==0)
        {
          ok=false;
          break;
        }
      }
      if (*i->unitType.requiredTech()!=TechTypes::None)
      {
        if (!Broodwar->self()->hasResearched(*i->unitType.requiredTech()))
          ok=false;
      }
      if (ok)
        units[*i->unitType.whatBuilds().first][i->unitType]+=i->count;
    }
    for(std::map<UnitType, std::map<UnitType,int> >::iterator i=units.begin();i!=units.end();i++)
    {
      std::set<Unit*> allUnits = Broodwar->self()->getUnits();
      std::vector<Unit*> factories;
      for(std::set<Unit*>::iterator j=allUnits.begin();j!=allUnits.end();j++)
      {
        if ((*j)->getType()==i->first && nextAvailableTime(*j)>-1)
        {
          factories.push_back(*j);
        }
      }
      std::sort(factories.begin(),factories.end(),unitCompare);
      
      for(int m=0;m<factories.size();m++)
      {
        BWAPI::Broodwar->drawTextScreen(5,y,"%s[%x]: %d",factories[m]->getType().getName().c_str(),factories[m],nextAvailableTime(factories[m]));y+=20;
      }
      bool done=false;
      while (!done)
      {
        int most=-1;
        UnitType t=UnitTypes::None;
        for(std::map<UnitType,int>::iterator j=i->second.begin();j!=i->second.end();j++)
        {
          BWAPI::Broodwar->drawTextScreen(5,y,"%s[%d]",j->first.getName().c_str(),j->second);y+=20;
          if (most<j->second)
          {
            most=j->second;
            t=j->first;
          }
        }
        if (t!=UnitTypes::None)
        {
          if (hasResources(t))
          {
            if (factories.empty())
            {
              BWAPI::Broodwar->drawTextScreen(5,y,"unit-limited for %s",t.getName().c_str());y+=20;
              done=true;//go to next builder type
            }
            else
            {
              if (nextAvailableTime(factories.front())==Broodwar->getFrameCount())
              {
                for(std::list<BuildItem>::iterator k=l->second.units.begin();k!=l->second.units.end();k++)
                {
                  if (k->unitType==t)
                  {
                    if (k->isAdditional)
                    {
                      k->count--;
                      if (k->count==0)
                      {
                        l->second.units.erase(k);
                      }
                    }
                    break;
                  }
                }
                this->spendResources(t);
                this->buildManager->build(t);
                nextAvailableTimeData[factories.front()]=Broodwar->getFrameCount()+20;
                i->second[t]--;
                if (i->second[t]==0)
                {
                  i->second.erase(t);
                }
              }
              else
              {
                this->reserveResources(factories.front(),t);
              }
              factories.erase(factories.begin());
            }
          }
          else
          {
            BWAPI::Broodwar->drawTextScreen(5,y,"resource-limited");y+=20;
            return;//out of resources, finish for this frame
          }
        }
        else
        {
          done=true;
        }
      }
    }
  }
  BWAPI::Broodwar->drawTextScreen(5,y,"unit-limited");y+=20;
  /*
  int o=5;
  std::list<BuildItem>::iterator i_next;
  //todo: redesign priority system
  for(std::list<BuildItem>::iterator i=l->second.begin();i!=l->second.end();i=i_next)
  {
    i_next=i;
    i_next++;
    if (i->unitType!=BWAPI::UnitTypes::None)
    {
      if (i->isAdditional)
      {
        BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Screen,5,o,"buildAdditional(%d, \"%s\", %d)",i->count,i->unitType.getName().c_str(),l->first);
        if (i->count>0)
        {
          if (this->hasResources(i->unitType))
          {
            this->buildManager->build(i->unitType,i->seedPosition);
            this->spendResources(i->unitType);
            i->count--;
          }
        }
        else
        {
          l->second.erase(i);
        }
      }
      else
      {
        BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Screen,5,o,"build(%d, \"%s\", %d)",i->count,i->unitType.getName().c_str(),l->first);
        if (this->buildManager->getPlannedCount(i->unitType)>=i->count)
          l->second.erase(i);
        else
          if (this->hasResources(i->unitType))
          {
            this->buildManager->build(i->unitType,i->seedPosition);
            this->spendResources(i->unitType);
          }
      }
    }
    else if (i->techType!=BWAPI::TechTypes::None)
    {
      BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Screen,5,o,"research(\"%s\", %d)",i->techType.getName().c_str(),l->first);
      if (this->techManager->planned(i->techType))
        l->second.erase(i);
      else
        if (this->hasResources(i->techType))
        {
          this->techManager->research(i->techType);
          this->spendResources(i->techType);
        }
    }
    else
    {
      BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Screen,5,o,"upgrade(%d, \"%s\", %d)",i->count,i->upgradeType.getName().c_str(),l->first);
      if (this->upgradeManager->getPlannedLevel(i->upgradeType)>=i->count)
        l->second.erase(i);
      else
        if (!BWAPI::Broodwar->self()->isUpgrading(i->upgradeType) && this->hasResources(i->upgradeType))
        {
          this->upgradeManager->upgrade(i->upgradeType);
          this->spendResources(i->upgradeType);
        }
    }
    o+=20;
  }
  */
}

std::string BuildOrderManager::getName() const
{
  return "Build Order Manager";
}
void BuildOrderManager::build(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
  if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
  if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
    seedPosition=BWAPI::Broodwar->self()->getStartLocation();

  BuildItem newItem;
  newItem.unitType=t;
  newItem.techType=BWAPI::TechTypes::None;
  newItem.upgradeType=BWAPI::UpgradeTypes::None;
  newItem.count=count;
  newItem.seedPosition=seedPosition;
  newItem.isAdditional=false;
  if (t.isBuilding())
    items[priority].buildings.push_back(newItem);
  else
    items[priority].units.push_back(newItem);
}

void BuildOrderManager::buildAdditional(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
  if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
  if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
    seedPosition=BWAPI::Broodwar->self()->getStartLocation();

  BuildItem newItem;
  newItem.unitType=t;
  newItem.techType=BWAPI::TechTypes::None;
  newItem.upgradeType=BWAPI::UpgradeTypes::None;
  newItem.count=count;
  newItem.seedPosition=seedPosition;
  newItem.isAdditional=true;
  if (t.isBuilding())
    items[priority].buildings.push_back(newItem);
  else
    items[priority].units.push_back(newItem);
}

void BuildOrderManager::research(BWAPI::TechType t, int priority)
{
  if (t==BWAPI::TechTypes::None || t==BWAPI::TechTypes::Unknown) return;
  BuildItem newItem;
  newItem.unitType=BWAPI::UnitTypes::None;
  newItem.techType=t;
  newItem.upgradeType=BWAPI::UpgradeTypes::None;
  newItem.count=1;
  newItem.isAdditional=false;
  items[priority].techs.push_back(newItem);
}

void BuildOrderManager::upgrade(int level, BWAPI::UpgradeType t, int priority)
{
  if (t==BWAPI::UpgradeTypes::None || t==BWAPI::UpgradeTypes::Unknown) return;
  BuildItem newItem;
  newItem.unitType=BWAPI::UnitTypes::None;
  newItem.techType=BWAPI::TechTypes::None;
  newItem.upgradeType=t;
  newItem.count=level;
  newItem.isAdditional=false;
  items[priority].upgrades.push_back(newItem);
}

bool BuildOrderManager::hasResources(BWAPI::UnitType t)
{
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals<t.mineralPrice())
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<t.gasPrice())
    return false;
  double m=BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals-t.mineralPrice();
  for(std::map<int, int>::iterator i=this->reservedMinerals.begin();i!=this->reservedMinerals.end();i++)
  {
    double t=i->first-Broodwar->getFrameCount();
    if (m+t*this->workerManager->getMineralRate()<i->second)
      return false;
    m-=i->second;
  }
  double g=BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas-t.gasPrice();
  for(std::map<int, int>::iterator i=this->reservedGas.begin();i!=this->reservedGas.end();i++)
  {
    double t=i->first-Broodwar->getFrameCount();
    if (g+t*this->workerManager->getGasRate()<i->second)
      return false;
    g-=i->second;
  }
  return true;
}

bool BuildOrderManager::hasResources(BWAPI::TechType t)
{
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals<t.mineralPrice())
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<t.gasPrice())
    return false;
  return true;
}

bool BuildOrderManager::hasResources(BWAPI::UpgradeType t)
{
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals< t.mineralPriceBase()+t.mineralPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1))
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<t.gasPriceBase()+t.gasPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1))
    return false;
  return true;
}

void BuildOrderManager::spendResources(BWAPI::UnitType t)
{
  this->usedMinerals+=t.mineralPrice();
  this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::TechType t)
{
  this->usedMinerals+=t.mineralPrice();
  this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::UpgradeType t)
{
  this->usedMinerals+=t.mineralPriceBase()+t.mineralPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
  this->usedGas+=t.gasPriceBase()+t.gasPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
}

int BuildOrderManager::getPlannedCount(BWAPI::UnitType t)
{
  int c=this->buildManager->getPlannedCount(t);
  if (t.isBuilding())
  {
    for(std::map<int, PriorityLevel>::iterator i=items.begin();i!=items.end();i++)
    {
      for(std::list<BuildItem>::iterator j=i->second.buildings.begin();j!=i->second.buildings.end();j++)
      {
        if (j->unitType==t)
        {
          if (j->isAdditional)
            c+=j->count;
          else
            c=c<j->count ? j->count : c;
        }
      }
    }
  }
  else
  {
    for(std::map<int, PriorityLevel>::iterator i=items.begin();i!=items.end();i++)
    {
      for(std::list<BuildItem>::iterator j=i->second.units.begin();j!=i->second.units.end();j++)
      {
        if (j->unitType==t)
        {
          if (j->isAdditional)
            c+=j->count;
          else
            c=c<j->count ? j->count : c;
        }
      }
    }
  }
  return c;
}