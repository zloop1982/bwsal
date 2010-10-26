#include "MacroAIModule.h"
using namespace BWAPI;
MacroAIModule::MacroAIModule()
{
  macroManager = NULL;
}
MacroAIModule::~MacroAIModule()
{
  if (macroManager != NULL)
    delete macroManager;
  macroManager = NULL;
}
void MacroAIModule::onStart()
{
  macroManager = new MacroManager(&arbitrator);
  macroManager->taskStreams.push_back(TaskStream(Task(UnitTypes::Terran_SCV)));
}
void MacroAIModule::onEnd(bool isWinner)
{
}
void MacroAIModule::onFrame()
{
  macroManager->update();
  arbitrator.update();
}