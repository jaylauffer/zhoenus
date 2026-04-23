#pragma once

#include "CoreMinimal.h"
#include "ShipStats.h"
#include "Kismet/BlueprintPlatformLibrary.h"
#include "SaveThemAllGameInstance.generated.h"

UCLASS()
class USaveThemAllGameInstance : public UPlatformGameInstance
{
	GENERATED_BODY()

	USaveThemAllGameInstance(const FObjectInitializer& initializer);

public:
	virtual void Init() override;

	static constexpr float DefaultForwardAcceleration = 500.f;
	static constexpr float DefaultReverseAcceleration = 200.f;
	static constexpr float DefaultPitchAcceleration = 50.f;
	static constexpr float DefaultYawAcceleration = 50.f;
	static constexpr float DefaultRollAcceleration = 100.f;
	static constexpr float DefaultMaxSpeed = 3000.f;
	static constexpr float DefaultMinSpeed = -1000.f;
	static constexpr float DefaultMaxBatteryEnergy = 100.f;
	static constexpr float DefaultBatteryRechargeRate = 18.f;
	static constexpr float DefaultZapShotEnergyCost = 14.f;
	static constexpr float SpeedConversionRatio = 1.f / 7.f;
	static constexpr float ForwardAccelerationPointRatio = 0.32f;
	static constexpr float ReverseAccelerationPointRatio = 0.18f;
	static constexpr float PitchAccelerationPointRatio = 0.07f;
	static constexpr float YawAccelerationPointRatio = 0.10f;
	static constexpr float RollAccelerationPointRatio = 0.08f;
	static constexpr float MaxBatteryEnergyPointRatio = 0.08f;
	static constexpr float BatteryRechargeRatePointRatio = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipStats shipStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDonutAggroTuning donutAggroTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float points;

	UFUNCTION(BlueprintPure, Category = Points)
	float GetConvertibleForwardSpeed() const;

	UFUNCTION(BlueprintPure, Category = Points)
	float GetConvertibleReverseSpeed() const;

	UFUNCTION(BlueprintPure, Category = Points)
	float Convert(float Amount) const;

	UFUNCTION(BlueprintCallable, Category = Points)
	float ConvertMaxSpeed(float ForwardAmount, float ReverseAmount, bool bPersist = true);

	UFUNCTION(BlueprintCallable, Category = Points)
	bool SpendPoints(float Amount, bool bPersist = true);

	UFUNCTION(BlueprintPure, Category = Points)
	float GetShipAdjustmentAllocationCost(const FShipStats& Stats) const;

	UFUNCTION(BlueprintPure, Category = Points)
	float GetShipAdjustmentPointBudget() const;

	UFUNCTION(BlueprintPure, Category = Points)
	float GetShipAdjustmentRemainingPoints(const FShipStats& PreviewStats) const;

	UFUNCTION(BlueprintCallable)
	void MakeNewGame();

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	bool LoadGame(const FString& SlotName, int32 UserIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	int64 Saved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	float AcquiredPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	float SpentPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	float ConvertedPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	int64 TotalAttempts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lifetime)
	int64 TotalSuccess;

	void SyncShipSpeedStats(float CurrentMaxSpeed, float CurrentMinSpeed);
};
