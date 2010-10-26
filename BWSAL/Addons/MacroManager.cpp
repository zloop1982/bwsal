#include <MacroManager.h>
MacroManager* TheMacroManager;
Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;

MacroManager::MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  TheArbitrator = arbitrator;
  TheMacroManager = this;
}
void MacroManager::update()
{
  //reset reserved resources

  for each(TaskStream ts in taskStreams)
  {
    //update this task stream
    ts.update();
  }
}