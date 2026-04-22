// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZhoenusLobbyPlayerController.generated.h"

UCLASS(Config=Game)
class AZhoenusLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AZhoenusLobbyPlayerController();

protected:
	virtual void BeginPlay() override;
};
