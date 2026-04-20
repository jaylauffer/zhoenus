// Copyright 2026 Jay Lauffer All Rights Reserved.

#include "ZhoenusThumbstick.h"

namespace
{
	float GetTouchPressure(const FPointerEvent& Event)
	{
		return FMath::Max(0.0f, Event.GetTouchForce());
	}
}

void ZhoenusThumbstick::Construct(const FArguments& InArgs)
{
	OnStickPressureChanged = InArgs._OnStickPressureChanged;
	SVirtualJoystick::Construct(SVirtualJoystick::FArguments());
	ResetPressureState();
}

FReply ZhoenusThumbstick::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	EnsurePressureStateSize();

	const FReply Reply = SVirtualJoystick::OnTouchStarted(MyGeometry, Event);
	const int32 ControlIndex = FindControlIndexByPointer(Event.GetPointerIndex());
	if (ControlIndex != INDEX_NONE)
	{
		SetControlPressure(ControlIndex, GetTouchPressure(Event));
	}

	return Reply;
}

FReply ZhoenusThumbstick::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	EnsurePressureStateSize();

	const FReply Reply = SVirtualJoystick::OnTouchMoved(MyGeometry, Event);
	const int32 ControlIndex = FindControlIndexByPointer(Event.GetPointerIndex());
	if (ControlIndex != INDEX_NONE)
	{
		SetControlPressure(ControlIndex, GetTouchPressure(Event));
	}

	return Reply;
}

FReply ZhoenusThumbstick::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& Event)
{
	EnsurePressureStateSize();

	const int32 ControlIndex = FindControlIndexByPointer(Event.GetPointerIndex());
	const FReply Reply = SVirtualJoystick::OnTouchEnded(MyGeometry, Event);
	if (ControlIndex != INDEX_NONE)
	{
		SetControlPressure(ControlIndex, 0.0f);
	}

	return Reply;
}

void ZhoenusThumbstick::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	EnsurePressureStateSize();
	SVirtualJoystick::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

float ZhoenusThumbstick::GetControlPressure(int32 ControlIndex) const
{
	EnsurePressureStateSize();
	return ControlPressures.IsValidIndex(ControlIndex) ? ControlPressures[ControlIndex] : 0.0f;
}

float ZhoenusThumbstick::GetCurrentPressure() const
{
	EnsurePressureStateSize();

	float MaxPressure = 0.0f;
	for (const float Pressure : ControlPressures)
	{
		MaxPressure = FMath::Max(MaxPressure, Pressure);
	}

	return MaxPressure;
}

int32 ZhoenusThumbstick::GetActivePressureControlCount() const
{
	EnsurePressureStateSize();

	int32 ActiveControlCount = 0;
	for (const float Pressure : ControlPressures)
	{
		if (Pressure > 0.0f)
		{
			++ActiveControlCount;
		}
	}

	return ActiveControlCount;
}

void ZhoenusThumbstick::ResetPressureState()
{
	ControlPressures.SetNumZeroed(Controls.Num());
}

void ZhoenusThumbstick::EnsurePressureStateSize() const
{
	if (ControlPressures.Num() != Controls.Num())
	{
		ControlPressures.SetNumZeroed(Controls.Num());
	}
}

int32 ZhoenusThumbstick::FindControlIndexByPointer(int32 PointerIndex) const
{
	for (int32 ControlIndex = 0; ControlIndex < Controls.Num(); ++ControlIndex)
	{
		if (Controls[ControlIndex].CapturedPointerIndex == PointerIndex)
		{
			return ControlIndex;
		}
	}

	return INDEX_NONE;
}

void ZhoenusThumbstick::SetControlPressure(int32 ControlIndex, float Pressure)
{
	EnsurePressureStateSize();
	if (!ControlPressures.IsValidIndex(ControlIndex))
	{
		return;
	}

	const float SanitizedPressure = FMath::Max(0.0f, Pressure);
	const bool bWasActive = ControlPressures[ControlIndex] > 0.0f;
	const bool bIsActive = SanitizedPressure > 0.0f;
	if (FMath::IsNearlyEqual(ControlPressures[ControlIndex], SanitizedPressure))
	{
		return;
	}

	ControlPressures[ControlIndex] = SanitizedPressure;
	if (!FMath::IsNearlyEqual(SanitizedPressure, 0.0f) || bWasActive != bIsActive)
	{
		OnStickPressureChanged.ExecuteIfBound(ControlIndex, SanitizedPressure, bIsActive);
	}
}
