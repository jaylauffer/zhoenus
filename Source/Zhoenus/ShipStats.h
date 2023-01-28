#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShipStats.generated.h"

USTRUCT(BlueprintType)
struct FShipStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	float MaxSpeed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	float MinSpeed = -1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float ForwardAcceleration = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float PitchAcceleration = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float YawAcceleration = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float RollAcceleration = 100.f;

};

UCLASS()
class USaveThemAllSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USaveThemAllSaveGame();

	UPROPERTY(VisibleAnywhere, Category = General)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = General)
	int32 UserIndex;
	
	UPROPERTY(VisibleAnywhere, Category = Ship)
	FShipStats ShipStats;

	UPROPERTY(VisibleAnywhere, Category = Points)
	float Points;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	int64 Saved;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float AcquiredPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float SpentPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float ConvertedPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	int64 TotalAttempts;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	int64 TotalSuccess;

};