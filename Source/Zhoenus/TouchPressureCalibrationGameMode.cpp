// Copyright 2026 Jay Lauffer All Rights Reserved.

#include "TouchPressureCalibrationGameMode.h"

#include "TouchPressureCalibrationPlayerController.h"

ATouchPressureCalibrationGameMode::ATouchPressureCalibrationGameMode()
{
	PlayerControllerClass = ATouchPressureCalibrationPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}
