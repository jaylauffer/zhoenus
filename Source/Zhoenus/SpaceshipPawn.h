// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ZhoenusMovementComponent.h"
#include "ZhoenusMovementSimulation.h"
#include "SpaceshipPawn.generated.h"


UCLASS(Config=Game)
class ASpaceshipPawn : public APawn
{
	GENERATED_BODY()

public:
	ASpaceshipPawn(const FObjectInitializer &initializer);

	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	/** Bound to the thrust axis */
	void ThrustInput(float Val);
	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	void RotateRightInput(float Val);

	void DisengageAutoCorrect(float Val);

	void FireWeapon(float Val);

	UPROPERTY(Category=Team, EditAnywhere)
	int32 Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Status)
	bool IsDonutTarget;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PlaneMesh;

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides
	void ProduceInput(const int32 DeltaMS, FZhoenusMovementInputCmd& Cmd);

private:

	UPROPERTY(Category = Movement, VisibleAnywhere)
	UZhoenusMovementComponent* FlyingMovementComponent;

	bool bCanFire;
	FVector GunOffset;
	void FireShot();
	void ShotTimerExpired();
	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	/** How quickly pawn can steer */
	UPROPERTY(Category=Plane, EditAnywhere)
	float TurnSpeed;

	UPROPERTY(Category = Plane, EditAnywhere)
	float RollSpeed;

	UPROPERTY(Category = Plane, EditAnywhere)
	float PitchSpeed;

public:
	FQuat CachedInput;

	/** How quickly forward speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
	float Acceleration;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere, BlueprintReadOnly)
		float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Yaw, EditAnywhere)
		float MinSpeed;

	float CurrentAcceleration;
	float TargetPitchSpeed;
	float TargetYawSpeed;
	float TargetRollSpeed;

	/** Current forward speed */
	UPROPERTY(BlueprintReadOnly)
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

	float AutoCorrectRate;

	float CurrentRateOfFire;
	
public:
	virtual void BeginPlay() override;

	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
};
