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
	virtual void Tick(float DeltaSeconds) override;
	virtual float GetProjectileAggroRadius() const override;
private:
	void CreateAimProjector();
	void UpdateAimProjector();
	void SetAimProjectorVisible(bool bVisible);

	UPROPERTY(Category = Reticle, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMaterialBillboardComponent> AimProjectorComponent;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	bool bEnableAimProjector = true;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	FString AimProjectorMaterialPath = TEXT("/Game/Textures/M_AimProjector.M_AimProjector");

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "150.0"))
	float AimProjectorTraceDistance = 5000.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "150.0"))
	float AimProjectorMaxVisibleDistance = 1400.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "12.0"))
	float AimProjectorDepthBias = 48.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "0.1"))
	float AimProjectorScale = 4.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "0.05"))
	float AimProjectorAggroRadiusScale = 0.5f;

public:
	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }

};
