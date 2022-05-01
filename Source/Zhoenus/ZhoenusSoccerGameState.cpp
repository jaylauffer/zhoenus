// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ZhoenusSoccerGameState.h"
#include "Net/UnrealNetwork.h"

AZhoenusSoccerGameState::AZhoenusSoccerGameState()
{
	Score.Add(0);
	Score.Add(0);
}

void AZhoenusSoccerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Score);
}
