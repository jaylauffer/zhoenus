// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetBody.generated.h"

class USceneComponent;

UCLASS(Config = Game)
class ZHOENUS_API APlanetBody : public AActor
{
	GENERATED_BODY()

public:
	APlanetBody();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	bool HasPlanetFrame() const { return bPlanetFrameInitialized; }
	FVector GetPlanetCenter() const { return PlanetCenter; }
	float GetPlanetRadius() const { return PlanetRadius; }
	float GetSurfaceHeightOffset() const { return SurfaceHeightOffset; }
	float GetGuardrailRadius() const { return PlanetRadius + SurfaceHeightOffset + GuardrailClearance; }
	FVector GetSurfaceDirection(const FVector& SamplePoint) const;
	FVector GetProjectedSurfacePoint(const FVector& SamplePoint, float ExtraOffset = 0.f) const;
	float GetSurfaceDistanceScore(const FVector& WorldPoint) const;
	bool IsWithinAuthoredCore(const FVector& WorldPoint) const;
	bool ShouldEnforceGuardrailAt(const FVector& WorldPoint) const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Planet")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float PlanetRadius = 1500000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float SurfaceHeightOffset = 0.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float GuardrailClearance = 350.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float AuthoredCoreBoundsPadding = 12000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float AnchorTraceStartHeight = 5000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float AnchorTraceDepth = 300000.f;

private:
	void UpdateTrackedActor();
	void CacheAuthoredCoreBounds();
	bool InitializePlanetFrame();

	TWeakObjectPtr<AActor> TrackedActor;
	FBox AuthoredCoreBounds;
	FVector PlanetCenter = FVector::ZeroVector;
	bool bPlanetFrameInitialized = false;
	bool bHasAuthoredCoreBounds = false;
};
