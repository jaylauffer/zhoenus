// Copyright 2026 Jay Lauffer

#include "ZhoenusLobbyPlayerController.h"

#include "Engine/EngineTypes.h"

AZhoenusLobbyPlayerController::AZhoenusLobbyPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	bEnableMouseOverEvents = true;
}

void AZhoenusLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetVirtualJoystickVisibility(false);
	ActivateTouchInterface(nullptr);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
