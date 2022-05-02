#include "Goal.h"
#include "DonutFlyerPawn.h"
#include "DonutFlyerAIController.h"
#include "ScoreKeeper.h"
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
	if (GetNetMode() < NM_Client)
	{
		ADonutFlyerPawn* donut{ Cast<ADonutFlyerPawn>(otherActor) };
		if (donut)
		{
			if (UWorld* World = GetWorld())
			{
				IScoreKeeperInterface* scoreKeeper{ Cast<IScoreKeeperInterface>(World->GetAuthGameMode()) };
				if (scoreKeeper)
				{
					ADonutFlyerAIController* controller{ donut->GetController<ADonutFlyerAIController>() };
					ensure(controller);
					scoreKeeper->Score(this, controller->GetTargetPlayer(), donut);
				}
			}
		}
	}
}

