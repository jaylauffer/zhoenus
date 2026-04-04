#include "SaveThemAllGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

namespace
{
	float GetPointCostForDelta(float CurrentValue, float DefaultValue, float PointRatio)
	{
		return FMath::Max(0.f, CurrentValue - DefaultValue) * PointRatio;
	}

	void ResetShipStats(FShipStats& ShipStats)
	{
		ShipStats.ForwardAcceleration = USaveThemAllGameInstance::DefaultForwardAcceleration;
		ShipStats.ReverseAcceleration = USaveThemAllGameInstance::DefaultReverseAcceleration;
		ShipStats.PitchAcceleration = USaveThemAllGameInstance::DefaultPitchAcceleration;
		ShipStats.YawAcceleration = USaveThemAllGameInstance::DefaultYawAcceleration;
		ShipStats.RollAcceleration = USaveThemAllGameInstance::DefaultRollAcceleration;
		ShipStats.MaxSpeed = USaveThemAllGameInstance::DefaultMaxSpeed;
		ShipStats.MinSpeed = USaveThemAllGameInstance::DefaultMinSpeed;
	}

	void RepairLegacyShipStats(FShipStats& ShipStats)
	{
		if (!FMath::IsFinite(ShipStats.MaxSpeed))
		{
			ShipStats.MaxSpeed = USaveThemAllGameInstance::DefaultMaxSpeed;
		}
		if (!FMath::IsFinite(ShipStats.MinSpeed))
		{
			ShipStats.MinSpeed = USaveThemAllGameInstance::DefaultMinSpeed;
		}
		if (!FMath::IsFinite(ShipStats.ForwardAcceleration))
		{
			ShipStats.ForwardAcceleration = USaveThemAllGameInstance::DefaultForwardAcceleration;
		}
		if (!FMath::IsFinite(ShipStats.PitchAcceleration))
		{
			ShipStats.PitchAcceleration = USaveThemAllGameInstance::DefaultPitchAcceleration;
		}
		if (!FMath::IsFinite(ShipStats.YawAcceleration))
		{
			ShipStats.YawAcceleration = USaveThemAllGameInstance::DefaultYawAcceleration;
		}
		if (!FMath::IsFinite(ShipStats.RollAcceleration))
		{
			ShipStats.RollAcceleration = USaveThemAllGameInstance::DefaultRollAcceleration;
		}

		// Older saves predate ReverseAcceleration. Those loads come through as zero,
		// which is not a meaningful migrated value for the current handling model.
		if (!FMath::IsFinite(ShipStats.ReverseAcceleration) || ShipStats.ReverseAcceleration <= 0.f)
		{
			ShipStats.ReverseAcceleration = USaveThemAllGameInstance::DefaultReverseAcceleration;
		}
	}
}

USaveThemAllGameInstance::USaveThemAllGameInstance(const FObjectInitializer& initializer) : Super(initializer)
{
	ResetShipStats(shipStats);
	donutAggroTuning = FDonutAggroTuning{};
}

void USaveThemAllGameInstance::Init()
{
	Super::Init();

	const USaveThemAllSaveGame* SaveDefaults = GetDefault<USaveThemAllSaveGame>();
	if (!SaveDefaults || !LoadGame(SaveDefaults->SaveSlotName, SaveDefaults->UserIndex))
	{
		MakeNewGame();
	}
}

void USaveThemAllGameInstance::MakeNewGame()
{
	ResetShipStats(shipStats);
	donutAggroTuning = FDonutAggroTuning{};

	points = 0.f;

	Saved = 0;
	AcquiredPoints = 0.f;
	SpentPoints = 0.f;
	ConvertedPoints = 0.f;
	TotalAttempts = 0;
	TotalSuccess = 0;
}

float USaveThemAllGameInstance::GetConvertibleForwardSpeed() const
{
	return FMath::Max(0.f, shipStats.MaxSpeed - DefaultMaxSpeed);
}

float USaveThemAllGameInstance::GetConvertibleReverseSpeed() const
{
	return FMath::Max(0.f, DefaultMinSpeed - shipStats.MinSpeed);
}

float USaveThemAllGameInstance::Convert(float Amount) const
{
	return FMath::Max(0.f, Amount) * SpeedConversionRatio;
}

float USaveThemAllGameInstance::ConvertMaxSpeed(float ForwardAmount, float ReverseAmount, bool bPersist)
{
	const float ForwardToConvert = FMath::Clamp(ForwardAmount, 0.f, GetConvertibleForwardSpeed());
	const float ReverseToConvert = FMath::Clamp(ReverseAmount, 0.f, GetConvertibleReverseSpeed());
	if (ForwardToConvert <= 0.f && ReverseToConvert <= 0.f)
	{
		return 0.f;
	}

	shipStats.MaxSpeed -= ForwardToConvert;
	shipStats.MinSpeed += ReverseToConvert;

	const float TotalConvertedPoints = Convert(ForwardToConvert + ReverseToConvert);
	points += TotalConvertedPoints;
	ConvertedPoints += TotalConvertedPoints;

	if (bPersist)
	{
		SaveGame();
	}

	return TotalConvertedPoints;
}

bool USaveThemAllGameInstance::SpendPoints(float Amount, bool bPersist)
{
	const float SpendAmount = FMath::Max(0.f, Amount);
	if (SpendAmount <= 0.f || points < SpendAmount)
	{
		return false;
	}

	points -= SpendAmount;
	SpentPoints += SpendAmount;

	if (bPersist)
	{
		SaveGame();
	}

	return true;
}

float USaveThemAllGameInstance::GetShipAdjustmentAllocationCost(const FShipStats& Stats) const
{
	const float ForwardCost = GetPointCostForDelta(Stats.ForwardAcceleration, DefaultForwardAcceleration, ForwardAccelerationPointRatio);
	const float ReverseCost = GetPointCostForDelta(Stats.ReverseAcceleration, DefaultReverseAcceleration, ReverseAccelerationPointRatio);
	const float PitchCost = GetPointCostForDelta(Stats.PitchAcceleration, DefaultPitchAcceleration, PitchAccelerationPointRatio);
	const float YawCost = GetPointCostForDelta(Stats.YawAcceleration, DefaultYawAcceleration, YawAccelerationPointRatio);
	const float RollCost = GetPointCostForDelta(Stats.RollAcceleration, DefaultRollAcceleration, RollAccelerationPointRatio);
	return ForwardCost + ReverseCost + PitchCost + YawCost + RollCost;
}

float USaveThemAllGameInstance::GetShipAdjustmentPointBudget() const
{
	return points + GetShipAdjustmentAllocationCost(shipStats);
}

float USaveThemAllGameInstance::GetShipAdjustmentRemainingPoints(const FShipStats& PreviewStats) const
{
	return GetShipAdjustmentPointBudget() - GetShipAdjustmentAllocationCost(PreviewStats);
}

void USaveThemAllGameInstance::SyncShipSpeedStats(float CurrentMaxSpeed, float CurrentMinSpeed)
{
	shipStats.MaxSpeed = CurrentMaxSpeed;
	shipStats.MinSpeed = CurrentMinSpeed;
}

void USaveThemAllGameInstance::SaveGame()
{
	auto save{ Cast<USaveThemAllSaveGame>(UGameplayStatics::CreateSaveGameObject(USaveThemAllSaveGame::StaticClass())) };
	if (save)
	{
		save->ShipStats = shipStats;
		save->DonutAggroTuning = donutAggroTuning;
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
		RepairLegacyShipStats(shipStats);
		donutAggroTuning = save->DonutAggroTuning;
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
