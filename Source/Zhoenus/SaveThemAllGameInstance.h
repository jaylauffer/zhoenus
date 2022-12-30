#pragma once

#include "CoreMinimal.h"
#include "ShipStats.h"
#include "Kismet/BlueprintPlatformLibrary.h"
#include "SaveThemAllGameInstance.generated.h"

UCLASS()
class USaveThemAllGameInstance : public UPlatformGameInstance
{
	GENERATED_BODY()

	USaveThemAllGameInstance(const FObjectInitializer& initializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShipStats shipStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float points;

	UFUNCTION(BlueprintCallable)
	void MakeNewGame();

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	bool LoadGame(const FString& SlotName, int32 UserIndex);

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	uint64 Saved;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float AcquiredPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float SpentPoints;

	UPROPERTY(VisibleAnywhere, Category = Lifetime)
	float ConvertedPoints;

};

