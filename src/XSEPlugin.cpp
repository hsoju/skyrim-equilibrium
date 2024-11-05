#include "InputEventHandler.h"
#include "RecoveryHandler.h"

bool SetupInputTracker()
{
	InputEventHandler::Register();
	return true;
}

bool SetupRecoveryHandler()
{
	RecoveryHandler::GetSingleton()->ImportSettings();
	return true;
}