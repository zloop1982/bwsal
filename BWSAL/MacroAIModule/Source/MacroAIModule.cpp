#include "MacroAIModule.h"
void MacroAIModule::onStart()
{
  macroManager = new MacroManager(&arbitrator);
}
void MacroAIModule::onEnd(bool isWinner)
{
}
void MacroAIModule::onFrame()
{
  macroManager->update();
}