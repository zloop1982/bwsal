#pragma once
class Resources
{
  public:
    Resources(int m = 0, int g = 0, int s = 0) : minerals(m),gas(g),supply(0) {}

    Resources& operator=(const Resources &r);
    Resources& set(int m, int g = 0, int s = 0);
    Resources& setMinerals(int m);
    Resources& setGas(int g);
    Resources& setSupply(int s);
    Resources& addMinerals(int m);
    Resources& addGas(int g);
    Resources& addSupply(int s);
    Resources& operator+=(const Resources &r);
    Resources& operator-=(const Resources &r);

    int getMinerals() const;
    int getGas() const;
    int getSupply() const;

    bool operator==(const Resources &r) const;
    bool operator<(const Resources &r) const;

    Resources operator+(const Resources &r) const;
    Resources operator-(const Resources &r) const;

  private:
    int minerals;
    int gas;
    int supply;
};