// Copyright 2026 Run Rong Games. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "ZhoenusExpandLevel1LandscapeCommandlet.generated.h"

UCLASS()
class ZHOENUSEDITOREXT_API UZhoenusExpandLevel1LandscapeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UZhoenusExpandLevel1LandscapeCommandlet(const FObjectInitializer& ObjectInitializer);

	virtual int32 Main(const FString& Params) override;
};
