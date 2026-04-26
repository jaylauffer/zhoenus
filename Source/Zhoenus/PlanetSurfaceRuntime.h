// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetSurfaceRuntime.generated.h"

class APlanetBody;
class UMaterialInterface;
class UProceduralMeshComponent;
class USceneComponent;

UCLASS(Config = Game)
class ZHOENUS_API APlanetSurfaceRuntime : public AActor
{
	GENERATED_BODY()

public:
	APlanetSurfaceRuntime();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetPlanetBody(APlanetBody* InPlanetBody) { PlanetBody = InPlanetBody; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Planet")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Planet")
	TObjectPtr<UProceduralMeshComponent> SurfaceMesh;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float TileWorldSize = 30000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet", meta = (ClampMin = "1", ClampMax = "64"))
	int32 TileResolution = 8;

	UPROPERTY(EditAnywhere, Config, Category = "Planet", meta = (ClampMin = "0", ClampMax = "8"))
	int32 TileRingCount = 2;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float RebuildDistance = 12000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	float MaterialWorldTiling = 60000.f;

	UPROPERTY(EditAnywhere, Config, Category = "Planet")
	bool bGenerateCollision = true;

	UPROPERTY(EditAnywhere, Category = "Planet")
	TObjectPtr<UMaterialInterface> SurfaceMaterial;

private:
	void UpdateTrackedActor();
	void RefreshPlanetBody();
	void RebuildSurface(const FVector& SurfaceAnchorPoint);
	bool ShouldSkipTile(const FVector& TileCenterPoint, const FVector& East, const FVector& North) const;

	TWeakObjectPtr<AActor> TrackedActor;
	TWeakObjectPtr<APlanetBody> PlanetBody;
	FVector LastSurfaceAnchorPoint = FVector::ZeroVector;
	bool bHasBuiltSurface = false;
};
