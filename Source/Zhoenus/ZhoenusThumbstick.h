// Copyright 2026 Jay Lauffer All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SVirtualJoystick.h"

DECLARE_DELEGATE_ThreeParams(FOnStickPressureChanged, int32, float, bool)

/**
 * A pressure-aware virtual joystick that preserves the engine SVirtualJoystick behavior.
 */
class ZhoenusThumbstick : public SVirtualJoystick
{
public:
	SLATE_BEGIN_ARGS(ZhoenusThumbstick)
		{}

		SLATE_EVENT(FOnStickPressureChanged, OnStickPressureChanged)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& Event) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	float GetControlPressure(int32 ControlIndex) const;
	float GetCurrentPressure() const;
	int32 GetActivePressureControlCount() const;
	void ResetPressureState();

private:
	void EnsurePressureStateSize() const;
	int32 FindControlIndexByPointer(int32 PointerIndex) const;
	void SetControlPressure(int32 ControlIndex, float Pressure);

	mutable TArray<float, TInlineAllocator<2>> ControlPressures;
	FOnStickPressureChanged OnStickPressureChanged;
};
