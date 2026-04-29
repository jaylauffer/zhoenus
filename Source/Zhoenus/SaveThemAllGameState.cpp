//Copyright Jay Lauffer 2026

#include "SaveThemAllGameState.h"

ASaveThemAllGameState::ASaveThemAllGameState()
	: Saved{ 0 }
	, Total{ 0 }
	, CleanSaveStreak{ 0 }
{
}

void ASaveThemAllGameState::RecordCleanSave()
{
	++CleanSaveStreak;
}

void ASaveThemAllGameState::ResetCleanSaveStreak()
{
	CleanSaveStreak = 0;
}
