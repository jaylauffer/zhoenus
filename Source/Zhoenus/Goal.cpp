#include "Goal.h"
#include "DonutFlyerPawn.h"
#include "DonutFlyerAIController.h"
#include "ScoreKeeper.h"
#include "SpaceshipPawn.h"
#include "Components/ShapeComponent.h"
#include "GameFramework/GameModeBase.h"

AGoal::AGoal(const FObjectInitializer& initializer) : Super(initializer)
{
	//if (IsValid(CollisionShape))
	//{
	//	CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AGoal::OnGoal);
	//}

}

void AGoal::BeginPlay()
{
	if (IsValid(CollisionShape))
	{
		CollisionShape->OnComponentBeginOverlap.AddDynamic(this, &AGoal::OnGoal);
	}

	Super::BeginPlay();
}

void AGoal::OnGoal(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool fromSweep, const FHitResult& sweepResult)
{
        UWorld* World = GetWorld();
        ADonutFlyerPawn* donut{ Cast<ADonutFlyerPawn>(otherActor) };
        if (donut)
        {
                IScoreKeeperInterface* scoreKeeper{ Cast<IScoreKeeperInterface>(World->GetAuthGameMode()) };
                if (scoreKeeper)
                {
                        ADonutFlyerAIController* controller{ donut->GetController<ADonutFlyerAIController>() };
                        ensure(controller);
                        scoreKeeper->Score(this, controller->GetTargetPlayer(), donut);
                }
        }
        ASpaceshipPawn* flyer{ Cast<ASpaceshipPawn>(otherActor) };
        if (flyer)
        {
                const FVector GoalLockLocation = IsValid(CollisionShape) ? CollisionShape->Bounds.Origin : GetActorLocation();
                for (const auto& dnt : flyer->Followers)
                {
                        ADonutFlyerPawn* dfp{ dnt.Get() };
                        if (IsValid(dfp))
                        {
                                dfp->LockTarget(this, GoalLockLocation);
                        }
                }
                flyer->Followers.Empty();
        }
}


