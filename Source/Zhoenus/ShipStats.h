#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShipStats.generated.h"

USTRUCT(BlueprintType)
struct FShipStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	float MinSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float ForwardAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float PitchAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float YawAcceleration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acceleration")
	float RollAcceleration;

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
	uint64 Saved;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float AcquiredPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float SpentPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float ConvertedPoints;
};