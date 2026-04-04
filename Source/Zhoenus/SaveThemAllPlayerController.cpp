#include "SaveThemAllPlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "SpaceshipPawn.h"
#include "SaveThemAllGameInstance.h"
#include "SaveThemAllGameState.h"

#define ON_SCREEN_DEBUG 1
#ifdef ON_SCREEN_DEBUG
#include <Runtime/Engine/Classes/Engine/Engine.h>
#define ScreenDebug(text) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, TEXT(text))
#else
#define ScreenDebug(text) 
#endif

ASaveThemAllPlayerController::ASaveThemAllPlayerController()
{
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> UIFinder(TEXT("/Game/Blueprints/UI/StartLevelText"));
		if (UIFinder.Class != nullptr)
		{
			wStartText = UIFinder.Class;
		}
	}
}

ASaveThemAllPlayerController::~ASaveThemAllPlayerController()
{

}

void ASaveThemAllPlayerController::OnPossess(APawn *pawn)
{
	Super::OnPossess(pawn);

	UUserWidget* startText = CreateWidget<UUserWidget>(this, wStartText);
	if (startText)
	{
		startText->AddToViewport();
	}
}

void ASaveThemAllPlayerController::OnUnPossess()
{
	ASpaceshipPawn* p{ GetPawn<ASpaceshipPawn>() };
	USaveThemAllGameInstance* gi{ GetGameInstance<USaveThemAllGameInstance>() };
	if (gi && p)
	{
		gi->SyncShipSpeedStats(p->MaxSpeed, p->MinSpeed);
	}

	Super::OnUnPossess();
}
