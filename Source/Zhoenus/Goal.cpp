#include "Goal.h"
#include "DonutFlyerPawn.h"
#include "ZhoenusSoccerGameMode.h"
#include "Components/ShapeComponent.h"

AGoal::AGoal(const FObjectInitializer& initializer) : Super(initializer)
{
	if (IsValid(CollisionShape))
	{
		CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AGoal::OnGoal);
	}

}

void AGoal::OnGoal(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& sweepResult)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		if (otherActor->IsA<ADonutFlyerPawn>())
		{
			if (UWorld* World = GetWorld())
			{
				if (AZhoenusSoccerGameMode* mode = World->GetAuthGameMode<AZhoenusSoccerGameMode>())
				{
					mode->IncrementScore(team, 1);
				}
			}
		}
	}
}

