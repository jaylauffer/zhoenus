// Copyright 2025 Run Rong Games, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SpaceshipPawn.h"
#include "ZhoenusPawn.generated.h"


UCLASS(Config=Game)
class AZhoenusPawn : public ASpaceshipPawn
{
	GENERATED_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(Category = Lights, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpotLightComponent* HeadLight;

	UPROPERTY(Category = Thrusters, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* LeftThruster;

	UPROPERTY(Category = Thrusters, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* RightThruster;

public:
	AZhoenusPawn(const FObjectInitializer &initializer);
	virtual void BeginPlay() override;
private:

public:
	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }

};
