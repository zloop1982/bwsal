#include <Resources.h>

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
Resources Resources::operator+(const Resources &r) const
{
  return Resources(minerals+r.minerals,gas+r.gas,supply+r.supply);
}
Resources Resources::operator-(const Resources &r) const
{
  return Resources(minerals-r.minerals,gas-r.gas,supply-r.supply);
}