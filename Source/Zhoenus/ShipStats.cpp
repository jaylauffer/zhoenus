#include "ShipStats.h"

USaveThemAllSaveGame::USaveThemAllSaveGame()
{
	SaveSlotName = TEXT("SaveSlot01");
	UserIndex = 0;
	Points = 0.f;
	Saved = 0;
	AcquiredPoints = 0.f;
	SpentPoints = 0.f;
	ConvertedPoints = 0.f;
	TotalAttempts = 0;
	TotalSuccess = 0;
	CleanSaveTotal = 0;
}
