// Copyright 2026 Jay Lauffer All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TouchPressureCalibrationPlayerController.generated.h"

class UTouchPressureCalibrationWidget;

UCLASS()
class ATouchPressureCalibrationPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATouchPressureCalibrationPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	UTouchPressureCalibrationWidget* CalibrationWidget = nullptr;
};
