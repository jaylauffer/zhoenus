// Copyright 2025 Run Rong Games, All Rights Reserved.

#include "ZhoenusBatteryComponent.h"

#include "Math/UnrealMathUtility.h"

UZhoenusBatteryComponent::UZhoenusBatteryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UZhoenusBatteryComponent::BeginPlay()
{
	Super::BeginPlay();
	ClampCurrentEnergy();
}

void UZhoenusBatteryComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (RechargeRate <= 0.f || MaxEnergy <= 0.f || CurrentEnergy >= MaxEnergy)
	{
		return;
	}

	CurrentEnergy = FMath::Min(MaxEnergy, CurrentEnergy + RechargeRate * FMath::Max(0.f, DeltaTime));
}

void UZhoenusBatteryComponent::ConfigureFromShipStats(const FShipStats& ShipStats, const bool bRefillBattery)
{
	MaxEnergy = FMath::Max(1.f, ShipStats.MaxBatteryEnergy);
	RechargeRate = FMath::Max(0.f, ShipStats.BatteryRechargeRate);
	ShotEnergyCost = FMath::Max(0.f, ShipStats.ZapShotEnergyCost);

	if (bRefillBattery)
	{
		RefillBattery();
		return;
	}

	ClampCurrentEnergy();
}

void UZhoenusBatteryComponent::RefillBattery()
{
	CurrentEnergy = MaxEnergy;
}

bool UZhoenusBatteryComponent::ConsumeEnergy(const float Amount)
{
	const float RequiredEnergy = FMath::Max(0.f, Amount);
	if (!CanConsumeEnergy(RequiredEnergy))
	{
		return false;
	}

	CurrentEnergy = FMath::Max(0.f, CurrentEnergy - RequiredEnergy);
	return true;
}

bool UZhoenusBatteryComponent::ConsumeShotEnergy()
{
	return ConsumeEnergy(ShotEnergyCost);
}

bool UZhoenusBatteryComponent::CanConsumeEnergy(const float Amount) const
{
	const float RequiredEnergy = FMath::Max(0.f, Amount);
	return RequiredEnergy <= 0.f || CurrentEnergy + KINDA_SMALL_NUMBER >= RequiredEnergy;
}

bool UZhoenusBatteryComponent::CanConsumeShotEnergy() const
{
	return CanConsumeEnergy(ShotEnergyCost);
}

float UZhoenusBatteryComponent::GetEnergyFraction() const
{
	if (MaxEnergy <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentEnergy / MaxEnergy, 0.f, 1.f);
}

void UZhoenusBatteryComponent::ClampCurrentEnergy()
{
	if (MaxEnergy <= 0.f || !FMath::IsFinite(MaxEnergy))
	{
		MaxEnergy = 1.f;
	}

	if (!FMath::IsFinite(CurrentEnergy))
	{
		CurrentEnergy = MaxEnergy;
		return;
	}

	CurrentEnergy = FMath::Clamp(CurrentEnergy, 0.f, MaxEnergy);
}
