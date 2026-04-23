// Copyright 2025 Run Rong Games, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShipStats.h"
#include "ZhoenusBatteryComponent.generated.h"

UCLASS(ClassGroup = (Zhoenus), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UZhoenusBatteryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZhoenusBatteryComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void ConfigureFromShipStats(const FShipStats& ShipStats, bool bRefillBattery = true);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	void RefillBattery();

	UFUNCTION(BlueprintCallable, Category = "Battery")
	bool ConsumeEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Battery")
	bool ConsumeShotEnergy();

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool CanConsumeEnergy(float Amount) const;

	UFUNCTION(BlueprintPure, Category = "Battery")
	bool CanConsumeShotEnergy() const;

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetCurrentEnergy() const { return CurrentEnergy; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetMaxEnergy() const { return MaxEnergy; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetRechargeRate() const { return RechargeRate; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetShotEnergyCost() const { return ShotEnergyCost; }

	UFUNCTION(BlueprintPure, Category = "Battery")
	float GetEnergyFraction() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery", meta = (ClampMin = "1.0"))
	float MaxEnergy = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery", meta = (ClampMin = "0.0"))
	float RechargeRate = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battery", meta = (ClampMin = "0.0"))
	float ShotEnergyCost = 14.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battery")
	float CurrentEnergy = 100.f;

private:
	void ClampCurrentEnergy();
};
