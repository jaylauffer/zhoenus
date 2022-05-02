#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Goal.generated.h"

UCLASS()
class AGoal : public AActor
{
	GENERATED_BODY()

	AGoal(const FObjectInitializer& initializer);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UShapeComponent* CollisionShape;

	void BeginPlay();

	UFUNCTION()
	void OnGoal(UPrimitiveComponent *overlappedComponent, AActor *otherActor, UPrimitiveComponent *otherComp, int32 otherBodyIndex, bool fromSweep, const FHitResult &sweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team)
	int32 team;
};

