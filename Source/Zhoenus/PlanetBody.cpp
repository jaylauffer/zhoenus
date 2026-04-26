// Copyright 2026 Jay Lauffer

#include "PlanetBody.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlanetBody, Log, All);

namespace
{
	constexpr float MaxRelevantAuthoredCoreExtent = 250000.f;

	bool IsLandscapeLikeActor(const AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass()->GetName();
		return ActorName.Contains(TEXT("Landscape")) || ClassName.Contains(TEXT("Landscape"));
	}

	bool IsIgnoredAuthoredCoreActor(const AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return true;
		}

		const FString ActorName = Actor->GetName();
		const FString ClassName = Actor->GetClass()->GetName();
		return
			Actor->IsA<APawn>() ||
			Actor->HasAnyFlags(RF_Transient) ||
			ActorName.Contains(TEXT("Planet")) ||
			ClassName.Contains(TEXT("Planet")) ||
			ActorName.Contains(TEXT("Sky")) ||
			ClassName.Contains(TEXT("Sky"));
	}

	bool ShouldIncludeActorInAuthoredCoreBounds(const AActor* Actor)
	{
		if (IsIgnoredAuthoredCoreActor(Actor))
		{
			return false;
		}

		FVector Origin = FVector::ZeroVector;
		FVector Extent = FVector::ZeroVector;
		Actor->GetActorBounds(false, Origin, Extent);

		if (Extent.IsNearlyZero())
		{
			return false;
		}

		return IsLandscapeLikeActor(Actor) || Extent.GetMax() <= MaxRelevantAuthoredCoreExtent;
	}
}

APlanetBody::APlanetBody()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void APlanetBody::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrackedActor();
	CacheAuthoredCoreBounds();
	if (InitializePlanetFrame())
	{
		SetActorTickEnabled(false);
	}
}

void APlanetBody::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPlanetFrameInitialized)
	{
		SetActorTickEnabled(false);
		return;
	}

	UpdateTrackedActor();
	if (InitializePlanetFrame())
	{
		SetActorTickEnabled(false);
	}
}

void APlanetBody::UpdateTrackedActor()
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

void APlanetBody::CacheAuthoredCoreBounds()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FBox CombinedBounds(EForceInit::ForceInit);

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* const Actor = *ActorIt;
		if (!ShouldIncludeActorInAuthoredCoreBounds(Actor))
		{
			continue;
		}

		FVector Origin = FVector::ZeroVector;
		FVector Extent = FVector::ZeroVector;
		Actor->GetActorBounds(false, Origin, Extent);
		CombinedBounds += Origin - Extent;
		CombinedBounds += Origin + Extent;
	}

	if (!CombinedBounds.IsValid)
	{
		return;
	}

	CombinedBounds.Min.X -= AuthoredCoreBoundsPadding;
	CombinedBounds.Min.Y -= AuthoredCoreBoundsPadding;
	CombinedBounds.Max.X += AuthoredCoreBoundsPadding;
	CombinedBounds.Max.Y += AuthoredCoreBoundsPadding;

	AuthoredCoreBounds = CombinedBounds;
	bHasAuthoredCoreBounds = true;

	UE_LOG(LogPlanetBody, Log, TEXT("Planet body cached authored core bounds: min %s max %s."),
		*AuthoredCoreBounds.Min.ToString(),
		*AuthoredCoreBounds.Max.ToString());
}

bool APlanetBody::InitializePlanetFrame()
{
	if (bPlanetFrameInitialized)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (World == nullptr || !TrackedActor.IsValid())
	{
		return false;
	}

	const FVector TraceOrigin = TrackedActor->GetActorLocation();
	const FVector TraceStart = TraceOrigin + FVector::UpVector * AnchorTraceStartHeight;
	const FVector TraceEnd = TraceOrigin - FVector::UpVector * AnchorTraceDepth;

	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlanetBodyAnchorTrace), false, TrackedActor.Get());
	QueryParams.AddIgnoredActor(this);

	FVector SurfaceAnchor = TraceOrigin;
	bool bFoundAnchor = false;

	if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		for (int32 HitIndex = Hits.Num() - 1; HitIndex >= 0; --HitIndex)
		{
			const FHitResult& Hit = Hits[HitIndex];
			if (!Hit.bBlockingHit)
			{
				continue;
			}

			if (IsLandscapeLikeActor(Hit.GetActor()))
			{
				SurfaceAnchor = Hit.ImpactPoint;
				bFoundAnchor = true;
				break;
			}
		}

		if (!bFoundAnchor)
		{
			for (int32 HitIndex = Hits.Num() - 1; HitIndex >= 0; --HitIndex)
			{
				if (Hits[HitIndex].bBlockingHit)
				{
					SurfaceAnchor = Hits[HitIndex].ImpactPoint;
					bFoundAnchor = true;
					break;
				}
			}
		}
	}

	if (!bFoundAnchor && bHasAuthoredCoreBounds)
	{
		SurfaceAnchor = FVector(
			TrackedActor->GetActorLocation().X,
			TrackedActor->GetActorLocation().Y,
			AuthoredCoreBounds.Max.Z);
	}

	PlanetCenter = SurfaceAnchor - FVector::UpVector * PlanetRadius;
	SetActorLocation(PlanetCenter);
	bPlanetFrameInitialized = true;

	UE_LOG(LogPlanetBody, Log, TEXT("Planet body initialized at center %s with radius %.1f."),
		*PlanetCenter.ToString(),
		PlanetRadius);

	return true;
}

FVector APlanetBody::GetSurfaceDirection(const FVector& SamplePoint) const
{
	FVector Direction = SamplePoint - PlanetCenter;
	if (!Direction.Normalize())
	{
		return FVector::UpVector;
	}

	return Direction;
}

FVector APlanetBody::GetProjectedSurfacePoint(const FVector& SamplePoint, const float ExtraOffset) const
{
	return PlanetCenter + GetSurfaceDirection(SamplePoint) * (PlanetRadius + SurfaceHeightOffset + ExtraOffset);
}

float APlanetBody::GetSurfaceDistanceScore(const FVector& WorldPoint) const
{
	return FMath::Abs(FVector::Dist(WorldPoint, PlanetCenter) - GetGuardrailRadius());
}

bool APlanetBody::IsWithinAuthoredCore(const FVector& WorldPoint) const
{
	if (!bHasAuthoredCoreBounds)
	{
		return false;
	}

	return
		WorldPoint.X >= AuthoredCoreBounds.Min.X &&
		WorldPoint.X <= AuthoredCoreBounds.Max.X &&
		WorldPoint.Y >= AuthoredCoreBounds.Min.Y &&
		WorldPoint.Y <= AuthoredCoreBounds.Max.Y;
}

bool APlanetBody::ShouldEnforceGuardrailAt(const FVector& WorldPoint) const
{
	if (!bPlanetFrameInitialized)
	{
		return false;
	}

	if (IsWithinAuthoredCore(WorldPoint))
	{
		return false;
	}

	return FVector::DistSquared(WorldPoint, PlanetCenter) < FMath::Square(GetGuardrailRadius());
}
