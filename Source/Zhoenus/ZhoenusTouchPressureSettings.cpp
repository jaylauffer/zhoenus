// Copyright 2026 Jay Lauffer All Rights Reserved.

#include "ZhoenusTouchPressureSettings.h"

#include "Math/UnrealMathUtility.h"

UZhoenusTouchPressureSettings::UZhoenusTouchPressureSettings()
	: StabilizePressureDeadzone(DefaultPressureDeadzone)
	, FirePressureDeadzone(DefaultPressureDeadzone)
	, StabilizePressureScale(DefaultPressureScale)
	, FirePressureScale(DefaultPressureScale)
{
}

void UZhoenusTouchPressureSettings::ClampValues()
{
	StabilizePressureDeadzone = FMath::Clamp(StabilizePressureDeadzone, 0.0f, 0.95f);
	FirePressureDeadzone = FMath::Clamp(FirePressureDeadzone, 0.0f, 0.95f);
	StabilizePressureScale = FMath::Clamp(StabilizePressureScale, 0.0f, 2.0f);
	FirePressureScale = FMath::Clamp(FirePressureScale, 0.0f, 2.0f);
}

void UZhoenusTouchPressureSettings::RestoreFactoryDefaults()
{
	StabilizePressureDeadzone = DefaultPressureDeadzone;
	FirePressureDeadzone = DefaultPressureDeadzone;
	StabilizePressureScale = DefaultPressureScale;
	FirePressureScale = DefaultPressureScale;
}

float UZhoenusTouchPressureSettings::NormalizePressure(const bool bForFire, const float RawPressure) const
{
	const float ClampedPressure = FMath::Clamp(RawPressure, 0.0f, 1.0f);
	const float Deadzone = bForFire ? FirePressureDeadzone : StabilizePressureDeadzone;
	const float Scale = bForFire ? FirePressureScale : StabilizePressureScale;
	const float ClampedDeadzone = FMath::Clamp(Deadzone, 0.0f, 0.95f);
	if (ClampedPressure <= ClampedDeadzone)
	{
		return 0.0f;
	}

	const float NormalizedPressure = (ClampedPressure - ClampedDeadzone) / (1.0f - ClampedDeadzone);
	return FMath::Clamp(NormalizedPressure * FMath::Max(0.0f, Scale), 0.0f, 1.0f);
}
