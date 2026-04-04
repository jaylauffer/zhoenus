#pragma once

#include "CoreMinimal.h"
#include "DonutAggroTuning.generated.h"

USTRUCT(BlueprintType)
struct FDonutAggroTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SightRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SightThreatBase = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sight")
	float SightThreatDistanceBonus = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileNearMissThreat = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileHitThreat = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileThreatRadiusMultiplier = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionThreatScale = 0.07125f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
	float CollisionThreatDecayPerSecond = 0.0231f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
	float ProjectileThreatDecayPerSecond = 0.0456f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
	float SightThreatDecayPerSecond = 0.00931f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decay")
	float LostTargetThreatDecayPerSecond = 0.0542f;
};
