// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SaveThemAllGameState.h"
#include "Net/UnrealNetwork.h"

ASaveThemAllGameState::ASaveThemAllGameState() : Saved{0}
{
}

void ASaveThemAllGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Saved);
}
