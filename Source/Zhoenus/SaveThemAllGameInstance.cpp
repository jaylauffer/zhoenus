#include "SaveThemAllGameInstance.h"
#include "Kismet/GameplayStatics.h"

USaveThemAllGameInstance::USaveThemAllGameInstance(const FObjectInitializer& initializer) : Super(initializer)
{
	//if (IsValid(CollisionShape))
	//{
	//	CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AGoal::OnGoal);
	//}

	shipStats.ForwardAcceleration = 500.f;
	shipStats.PitchAcceleration = 50.f;
	shipStats.YawAcceleration = 50.f;
	shipStats.RollAcceleration = 100.f;

	shipStats.MaxSpeed = 3000.f;
	shipStats.MinSpeed = -1000.f;
}

void USaveThemAllGameInstance::MakeNewGame()
{
	{
		shipStats.ForwardAcceleration = 500.f;
		shipStats.PitchAcceleration = 50.f;
		shipStats.YawAcceleration = 50.f;
		shipStats.RollAcceleration = 100.f;

		shipStats.MaxSpeed = 3000.f;
		shipStats.MinSpeed = -1000.f;

		points = 0.f;

		Saved = 0;
		AcquiredPoints = 0.f;
		SpentPoints = 0.f;
		ConvertedPoints = 0.f;
		TotalAttempts = 0;
		TotalSuccess = 0;
	}
}

void USaveThemAllGameInstance::SaveGame()
{
	auto save{ Cast<USaveThemAllSaveGame>(UGameplayStatics::CreateSaveGameObject(USaveThemAllSaveGame::StaticClass())) };
	if (save)
	{
		save->ShipStats = shipStats;
		save->Points = points;
		save->Saved = Saved;
		save->AcquiredPoints = AcquiredPoints;
		save->SpentPoints = SpentPoints;
		save->ConvertedPoints = ConvertedPoints;
		save->TotalAttempts = TotalAttempts;
		save->TotalSuccess = TotalSuccess;

		UGameplayStatics::AsyncSaveGameToSlot(save, save->SaveSlotName, save->UserIndex);
	}
}

bool USaveThemAllGameInstance::LoadGame(const FString& SaveSlot, int32 UserIndex)
{
	auto save{ Cast<USaveThemAllSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, UserIndex)) };
	if (save)
	{
		shipStats = save->ShipStats;
		points = save->Points;
		Saved = save->Saved;
		AcquiredPoints = save->AcquiredPoints;
		SpentPoints = save->SpentPoints;
		ConvertedPoints = save->ConvertedPoints;
		TotalAttempts = save->TotalAttempts;
		TotalSuccess = save->TotalSuccess;
		return true;
	}
	return false;
}
