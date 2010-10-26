#include <MacroManager.h>
#include <BWAPI.h>
using namespace BWAPI;
MacroManager* TheMacroManager;
Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;

MacroManager::MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  TheArbitrator = arbitrator;
  TheMacroManager = this;
  gatherRateStepSize = 10;
}
void MacroManager::update()
{
  computeGatherRate();
  rtl.reset(Resources(Broodwar->self()),gatherRate.getMinerals(),gatherRate.getGas());
  for each(TaskStream* ts in killSet)
  {
    for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
    {
      if (*i == ts)
      {
        taskStreams.erase(i);
        break;
      }
    }
    delete ts;
  }
  Broodwar->drawTextScreen(452,16,"\x07%d",(int)(gatherRate.getMinerals()*23*60));
  Broodwar->drawTextScreen(520,16,"\x07%d",(int)(gatherRate.getGas()*23*60));
  int x=0;
  for(std::list<Resources>::iterator i=resourceHistory.begin();i!=resourceHistory.end();i++)
  {
    int y=(int)((*i).getMinerals());
    Broodwar->drawDotScreen(x,(int)(CumulativeResources(Broodwar->self()).getMinerals())-y,Colors::Cyan);
    int y2=(int)(gatherRateA.getMinerals()+gatherRateB.getMinerals()*x*gatherRateStepSize+gatherRateC.getMinerals()*x*gatherRateStepSize*x*gatherRateStepSize);
    Broodwar->drawDotScreen(x,(int)(CumulativeResources(Broodwar->self()).getMinerals())-y2,Colors::Green);
    x++;
  }
  killSet.clear();
  int y=0;
  for each(TaskStream* ts in taskStreams)
  {
    //update this task stream
    ts->update();
    ts->printToScreen(10,y);
    y+=20;
  }
}
void MacroManager::computeGatherRate()
{
  if (Broodwar->getFrameCount()%gatherRateStepSize==0)
  {
    resourceHistory.push_back(CumulativeResources(Broodwar->self()));
    if (resourceHistory.size()>101)
      resourceHistory.pop_front();
  }
  int n = resourceHistory.size();
  double rate_m=0;
  double rate_g=0;
  double MIN_a=0;
  double MIN_b=0;
  double MIN_c=0;
  double GAS_a=0;
  double GAS_b=0;
  double GAS_c=0;
  if (n>1)
  {
    double nom_m    = 0.0;
    double nom_g    = 0.0;
    double div      = 0.0;
    double x        = 0.0;
    double S1       = n;
    double SX       = 0.0;
    double SXX      = 0.0;
    double SXXX     = 0.0;
    double SXXXX    = 0.0;
    double MIN_SY   = 0.0;
    double MIN_SXY  = 0.0;
    double MIN_SXXY = 0.0;
    double GAS_SY   = 0.0;
    double GAS_SXY  = 0.0;
    double GAS_SXXY = 0.0;
    double total_ym = 0.0;
    double total_yg = 0.0;
    for(std::list<Resources>::iterator i=resourceHistory.begin();i!=resourceHistory.end();i++)
    {
      double ym=(*i).getMinerals();
      double yg=(*i).getGas();
      nom_m    += x*ym;
      nom_g    += x*yg;
      total_ym += ym;
      total_yg += yg;
      SX       += x;
      SXX      += x*x;
      SXXX     += x*x*x;
      SXXXX    += x*x*x*x;
      MIN_SY   += ym;
      MIN_SXY  += x*ym;
      MIN_SXXY += x*x*ym;
      GAS_SY   += ym;
      GAS_SXY  += x*ym;
      GAS_SXXY += x*x*ym;
      x+=gatherRateStepSize;
    }
    x-=gatherRateStepSize;
    double MIN_a_numerator = SXX*SXX*MIN_SXXY - SX*MIN_SXXY*SXXX - SXX*SXXX*MIN_SXY + SX*SXXXX*MIN_SXY + SXXX*SXXX*MIN_SY - SXX*SXXXX*MIN_SY;
    double MIN_b_numerator = SX*SXX*MIN_SXXY  - S1*MIN_SXXY*SXXX - SXX*SXX*MIN_SXY  + S1*SXXXX*MIN_SXY + SXX*SXXX*MIN_SY  - SX*SXXXX*MIN_SY;
    double MIN_c_numerator = -SX*SX*MIN_SXXY  + S1*SXX*MIN_SXXY  + SX*SXX*MIN_SXY   - S1*SXXX*MIN_SXY  - SXX*SXX*MIN_SY   + SX*SXXX*MIN_SY;
    double GAS_a_numerator = SXX*SXX*GAS_SXXY - SX*GAS_SXXY*SXXX - SXX*SXXX*GAS_SXY + SX*SXXXX*GAS_SXY + SXXX*SXXX*GAS_SY - SXX*SXXXX*GAS_SY;
    double GAS_b_numerator = SX*SXX*GAS_SXXY  - S1*GAS_SXXY*SXXX - SXX*SXX*GAS_SXY  + S1*SXXXX*GAS_SXY + SXX*SXXX*GAS_SY  - SX*SXXXX*GAS_SY;
    double GAS_c_numerator = -SX*SX*GAS_SXXY  + S1*SXX*GAS_SXXY  + SX*SXX*GAS_SXY   - S1*SXXX*GAS_SXY  - SXX*SXX*GAS_SY   + SX*SXXX*GAS_SY;
    double denominator = SXX*SXX*SXX-2*SX*SXX*SXXX+S1*SXXX*SXXX+SX*SX*SXXXX-S1*SXX*SXXXX;
    MIN_a = MIN_a_numerator/denominator;
    MIN_b = -MIN_b_numerator/denominator;
    MIN_c = -MIN_c_numerator/denominator;
    GAS_a = GAS_a_numerator/denominator;
    GAS_b = -GAS_b_numerator/denominator;
    GAS_c = -GAS_c_numerator/denominator;
    rate_m = MIN_b + 2*x*MIN_c;
    rate_g = GAS_b + 2*x*GAS_c;
    if (rate_m<0) rate_m=0;
    if (rate_g<0) rate_g=0;
  }
  gatherRate.setMinerals(rate_m);
  gatherRate.setGas(rate_g);
  gatherRateA=Resources(MIN_a,GAS_a,0);
  gatherRateB=Resources(MIN_b,GAS_b,0);
  gatherRateC=Resources(MIN_c,GAS_c,0);
}

Resources MacroManager::getGatherRate() const
{
  return gatherRate;
}