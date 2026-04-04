// Copyright 2026 Run Rong Games. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "ZhoenusAimProjectionCommandlet.generated.h"

UCLASS()
class ZHOENUSEDITOREXT_API UZhoenusAimProjectionCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UZhoenusAimProjectionCommandlet(const FObjectInitializer& ObjectInitializer);

	virtual int32 Main(const FString& Params) override;
};
