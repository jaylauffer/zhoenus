// Copyright 2026 Jay Lauffer All Rights Reserved.

#include "TouchPressureCalibrationPlayerController.h"

#include "TouchPressureCalibrationWidget.h"

ATouchPressureCalibrationPlayerController::ATouchPressureCalibrationPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	bEnableMouseOverEvents = true;
}

void ATouchPressureCalibrationPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetVirtualJoystickVisibility(false);
	ActivateTouchInterface(nullptr);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (!CalibrationWidget)
	{
		CalibrationWidget = CreateWidget<UTouchPressureCalibrationWidget>(
			this,
			UTouchPressureCalibrationWidget::StaticClass());
	}

	if (CalibrationWidget && !CalibrationWidget->IsInViewport())
	{
		CalibrationWidget->AddToViewport(100);
	}
}

void ATouchPressureCalibrationPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CalibrationWidget)
	{
		CalibrationWidget->RemoveFromParent();
	}

	Super::EndPlay(EndPlayReason);
}
