// Copyright 2026 Jay Lauffer All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZhoenusTouchPressureSettings.generated.h"

UCLASS(Config=Game, DefaultConfig)
class UZhoenusTouchPressureSettings : public UObject
{
	GENERATED_BODY()

public:
	static constexpr float DefaultPressureDeadzone = 0.12f;
	static constexpr float DefaultPressureScale = 1.0f;

	UZhoenusTouchPressureSettings();

	UPROPERTY(EditAnywhere, Config, Category = "Touch Pressure", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float StabilizePressureDeadzone;

	UPROPERTY(EditAnywhere, Config, Category = "Touch Pressure", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float FirePressureDeadzone;

	UPROPERTY(EditAnywhere, Config, Category = "Touch Pressure", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float StabilizePressureScale;

	UPROPERTY(EditAnywhere, Config, Category = "Touch Pressure", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float FirePressureScale;

	void ClampValues();
	void RestoreFactoryDefaults();
	float NormalizePressure(bool bForFire, float RawPressure) const;
};
