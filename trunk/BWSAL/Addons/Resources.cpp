#include <Resources.h>

Resources::Resources(BWAPI::Player* player)
{
  if (player==NULL)
  {
    minerals = 0;
    gas      = 0;
    supply   = 0;
  }
  else
  {
    minerals = player->minerals();
    gas      = player->gas();
    supply   = player->supplyTotal() - player->supplyUsed();
  }
}
Resources::Resources(BWAPI::UnitType type)
{
  minerals = type.mineralPrice();
  gas      = type.gasPrice();
  supply   = type.supplyRequired();
}
Resources::Resources(BWAPI::TechType type)
{
  minerals = type.mineralPrice();
  gas      = type.gasPrice();
  supply   = 0;
}
Resources::Resources(BWAPI::UpgradeType type, int level)
{
  minerals = type.mineralPriceBase() + type.mineralPriceFactor()*(level-1);
  gas      = type.gasPriceBase() + type.gasPriceFactor()*(level-1);
  supply   = 0;
}
Resources& Resources::operator=(const Resources &r)
{
  minerals = r.minerals;
  gas      = r.gas;
  supply   = r.supply;
  return *this;
}
Resources& Resources::set(int m, int g, int s)
{
  minerals = m;
  gas      = g;
  supply   = s;
  return *this;
}
Resources& Resources::setMinerals(int m)
{
  minerals = m;
  return *this;
}
Resources& Resources::setGas(int g)
{
  gas = g;
  return *this;
}
Resources& Resources::setSupply(int s)
{
  supply = s;
  return *this;
}
Resources& Resources::addMinerals(int m)
{
  minerals += m;
  return *this;
}
Resources& Resources::addGas(int g)
{
  gas += g;
  return *this;
}
Resources& Resources::addSupply(int s)
{
  supply += s;
  return *this;
}
Resources& Resources::operator+=(const Resources &r)
{
  minerals += r.minerals;
  gas      += r.gas;
  supply   += r.supply;
  return *this;
}
Resources& Resources::operator-=(const Resources &r)
{
  minerals -= r.minerals;
  gas      -= r.gas;
  supply   -= r.supply;
  return *this;
}
int Resources::getMinerals() const
{
  return minerals;
}
int Resources::getGas() const
{
  return gas;
}
int Resources::getSupply() const
{
  return supply;
}
bool Resources::operator==(const Resources &r) const
{
  return minerals == r.minerals && gas == r.gas && supply == r.supply;
}
bool Resources::operator<(const Resources &r) const
{
  return minerals < r.minerals || (minerals == r.minerals && gas < r.gas) || (minerals == r.minerals && gas == r.gas && supply < r.supply);
}
bool Resources::isValid() const
{
  return minerals>=0 && gas>=0 && supply>=0;
}
Resources Resources::operator+(const Resources &r) const
{
  return Resources(minerals+r.minerals,gas+r.gas,supply+r.supply);
}
Resources Resources::operator-(const Resources &r) const
{
  return Resources(minerals-r.minerals,gas-r.gas,supply-r.supply);
}
