// Copyright 2026 Jay Lauffer

#include "PlanetSurfaceRuntime.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "PlanetBody.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlanetSurfaceRuntime, Log, All);

APlanetSurfaceRuntime::APlanetSurfaceRuntime()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SurfaceMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SurfaceMesh"));
	SurfaceMesh->SetupAttachment(SceneRoot);
	SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SurfaceMesh->SetCollisionObjectType(ECC_WorldStatic);
	SurfaceMesh->SetCollisionResponseToAllChannels(ECR_Block);
	SurfaceMesh->SetGenerateOverlapEvents(false);
	SurfaceMesh->SetCanEverAffectNavigation(false);
	SurfaceMesh->bUseAsyncCooking = true;
	SurfaceMesh->bUseComplexAsSimpleCollision = false;
	SurfaceMesh->CastShadow = false;
	SurfaceMesh->bCastDynamicShadow = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SurfaceMaterialFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SurfaceMaterialFinder.Succeeded())
	{
		SurfaceMaterial = SurfaceMaterialFinder.Object;
	}
}

void APlanetSurfaceRuntime::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrackedActor();
	RefreshPlanetBody();

	if (PlanetBody.IsValid() && PlanetBody->HasPlanetFrame() && TrackedActor.IsValid())
	{
		RebuildSurface(PlanetBody->GetProjectedSurfacePoint(TrackedActor->GetActorLocation()));
	}
}

void APlanetSurfaceRuntime::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTrackedActor();
	RefreshPlanetBody();

	APlanetBody* const CurrentPlanetBody = PlanetBody.Get();
	if (!TrackedActor.IsValid() || !IsValid(CurrentPlanetBody) || !CurrentPlanetBody->HasPlanetFrame())
	{
		return;
	}

	const FVector SurfaceAnchorPoint = CurrentPlanetBody->GetProjectedSurfacePoint(TrackedActor->GetActorLocation());
	if (!bHasBuiltSurface || FVector::DistSquared(LastSurfaceAnchorPoint, SurfaceAnchorPoint) >= FMath::Square(FMath::Max(RebuildDistance, 1000.f)))
	{
		RebuildSurface(SurfaceAnchorPoint);
	}
}

void APlanetSurfaceRuntime::UpdateTrackedActor()
{
	if (TrackedActor.IsValid())
	{
		return;
	}

	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		TrackedActor = Pawn;
	}
}

void APlanetSurfaceRuntime::RefreshPlanetBody()
{
	if (PlanetBody.IsValid())
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<APlanetBody> PlanetIt(World); PlanetIt; ++PlanetIt)
	{
		if (IsValid(*PlanetIt))
		{
			PlanetBody = *PlanetIt;
			return;
		}
	}
}

bool APlanetSurfaceRuntime::ShouldSkipTile(const FVector& TileCenterPoint, const FVector& East, const FVector& North) const
{
	APlanetBody* const CurrentPlanetBody = PlanetBody.Get();
	if (!IsValid(CurrentPlanetBody))
	{
		return false;
	}

	const float HalfTileSize = TileWorldSize * 0.5f;
	const FVector TileCorners[] = {
		TileCenterPoint + East * HalfTileSize + North * HalfTileSize,
		TileCenterPoint + East * HalfTileSize - North * HalfTileSize,
		TileCenterPoint - East * HalfTileSize + North * HalfTileSize,
		TileCenterPoint - East * HalfTileSize - North * HalfTileSize,
	};

	for (const FVector& TileCorner : TileCorners)
	{
		if (!CurrentPlanetBody->IsWithinAuthoredCore(CurrentPlanetBody->GetProjectedSurfacePoint(TileCorner)))
		{
			return false;
		}
	}

	return true;
}

void APlanetSurfaceRuntime::RebuildSurface(const FVector& SurfaceAnchorPoint)
{
	APlanetBody* const CurrentPlanetBody = PlanetBody.Get();
	if (SurfaceMesh == nullptr || !IsValid(CurrentPlanetBody) || !CurrentPlanetBody->HasPlanetFrame())
	{
		return;
	}

	SetActorLocation(CurrentPlanetBody->GetPlanetCenter());

	const FVector SurfaceNormal = CurrentPlanetBody->GetSurfaceDirection(SurfaceAnchorPoint);
	FVector East = FVector::CrossProduct(FVector::UpVector, SurfaceNormal);
	if (!East.Normalize())
	{
		East = FVector::CrossProduct(FVector::ForwardVector, SurfaceNormal).GetSafeNormal();
	}

	const FVector North = FVector::CrossProduct(SurfaceNormal, East).GetSafeNormal();
	if (North.IsNearlyZero())
	{
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	const int32 StepsPerTile = FMath::Max(1, TileResolution);
	const float SafeTilingScale = FMath::Max(MaterialWorldTiling, 1.f);

	for (int32 TileY = -TileRingCount; TileY <= TileRingCount; ++TileY)
	{
		for (int32 TileX = -TileRingCount; TileX <= TileRingCount; ++TileX)
		{
			const FVector TileCenterPoint =
				SurfaceAnchorPoint +
				East * (static_cast<float>(TileX) * TileWorldSize) +
				North * (static_cast<float>(TileY) * TileWorldSize);

			if (ShouldSkipTile(TileCenterPoint, East, North))
			{
				continue;
			}

			const int32 BaseVertexIndex = Vertices.Num();
			for (int32 YStep = 0; YStep <= StepsPerTile; ++YStep)
			{
				const float YAlpha = static_cast<float>(YStep) / static_cast<float>(StepsPerTile);
				const float LocalYOffset = (static_cast<float>(TileY) + YAlpha - 0.5f) * TileWorldSize;

				for (int32 XStep = 0; XStep <= StepsPerTile; ++XStep)
				{
					const float XAlpha = static_cast<float>(XStep) / static_cast<float>(StepsPerTile);
					const float LocalXOffset = (static_cast<float>(TileX) + XAlpha - 0.5f) * TileWorldSize;
					const FVector PlaneSamplePoint =
						SurfaceAnchorPoint +
						East * LocalXOffset +
						North * LocalYOffset;

					const FVector SurfaceDirection = CurrentPlanetBody->GetSurfaceDirection(PlaneSamplePoint);
					const FVector WorldVertex = CurrentPlanetBody->GetProjectedSurfacePoint(PlaneSamplePoint);
					FVector TangentVector = FVector::CrossProduct(FVector::UpVector, SurfaceDirection).GetSafeNormal();
					if (TangentVector.IsNearlyZero())
					{
						TangentVector = East;
					}

					Vertices.Add(WorldVertex - CurrentPlanetBody->GetPlanetCenter());
					Normals.Add(SurfaceDirection);
					UV0.Add(FVector2D(LocalXOffset / SafeTilingScale, LocalYOffset / SafeTilingScale));
					VertexColors.Add(FLinearColor::White);
					Tangents.Add(FProcMeshTangent(TangentVector, false));
				}
			}

			for (int32 YStep = 0; YStep < StepsPerTile; ++YStep)
			{
				for (int32 XStep = 0; XStep < StepsPerTile; ++XStep)
				{
					const int32 RowStride = StepsPerTile + 1;
					const int32 TopLeft = BaseVertexIndex + YStep * RowStride + XStep;
					const int32 TopRight = TopLeft + 1;
					const int32 BottomLeft = TopLeft + RowStride;
					const int32 BottomRight = BottomLeft + 1;

					Triangles.Add(TopLeft);
					Triangles.Add(BottomLeft);
					Triangles.Add(TopRight);

					Triangles.Add(TopRight);
					Triangles.Add(BottomLeft);
					Triangles.Add(BottomRight);
				}
			}
		}
	}

	SurfaceMesh->ClearAllMeshSections();
	if (Vertices.Num() == 0)
	{
		SurfaceMesh->SetVisibility(false, true);
		LastSurfaceAnchorPoint = SurfaceAnchorPoint;
		bHasBuiltSurface = true;
		UE_LOG(LogPlanetSurfaceRuntime, Log, TEXT("Planet surface runtime is waiting outside the authored core before generating tiles."));
		return;
	}

	SurfaceMesh->CreateMeshSection_LinearColor(
		0,
		Vertices,
		Triangles,
		Normals,
		UV0,
		VertexColors,
		Tangents,
		bGenerateCollision);

	SurfaceMesh->SetCollisionEnabled(bGenerateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	SurfaceMesh->SetVisibility(true, true);
	if (SurfaceMaterial != nullptr)
	{
		SurfaceMesh->SetMaterial(0, SurfaceMaterial);
	}

	LastSurfaceAnchorPoint = SurfaceAnchorPoint;
	bHasBuiltSurface = true;
}
