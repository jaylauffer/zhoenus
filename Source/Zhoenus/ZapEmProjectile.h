// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZapEmProjectile.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;
class USphereComponent;
class USoundBase;
class UNiagaraSystem;
class ADonutFlyerPawn;
class ASpaceshipPawn;
enum class EDonutAggroEventType : uint8;

UCLASS(config=Game)
class AZapEmProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	USphereComponent* AggroSphere;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	void TriggerDonutFlyerAggro(ADonutFlyerPawn* Pawn, EDonutAggroEventType EventType, float AggroAmount, bool bCountPlayerShot, const FVector& ContactLocation);
	void PlayAggroFeedback(const FVector& ContactLocation);
	void ResetAggroPulse();
	float ResolveAggroRadius() const;

	float BaseProjectileRadius{ 12.f };
	float ProjectileNearMissThreat{ 0.5f };
	float ProjectileHitThreat{ 3.0f };
	FVector BaseProjectileScale{ FVector::OneVector };
	float AggroRadiusOverride{ 0.f };
	FTimerHandle TimerHandle_AggroPulseReset;
	TSet<TWeakObjectPtr<ADonutFlyerPawn>> AggroedDonuts;

protected:
	virtual void BeginPlay() override;

public:
	AZapEmProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float AggroRadiusMultiplier{ 12.f };

	void SetAggroRadiusOverride(float InAggroRadius);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Feedback)
	float AggroPulseScale{ 1.75f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Feedback)
	float AggroPulseDuration{ 0.12f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Feedback)
	float AggroFeedbackSoundVolume{ 0.45f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Feedback)
	FVector AggroFlareScale{ FVector(0.09f, 0.09f, 0.09f) };

	/** Function to handle the projectile hitting something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnAggroSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Returns ProjectileMesh subobject **/
	FORCEINLINE UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Feedback, meta = (AllowPrivateAccess = "true"))
	USoundBase* AggroFeedbackSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Feedback, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* AggroFeedbackFlare;

	ASpaceshipPawn* Attacker;
};
