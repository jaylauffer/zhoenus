#include "ZhoenusPlayerController.h"
#include "Zhoenus.h"
#include "ZhoenusPawn.h"
#include "ZhoenusTouchUI.h"
#include "Button.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "UObject/ConstructorHelpers.h"

#define ON_SCREEN_DEBUG 1
#ifdef ON_SCREEN_DEBUG
#include <Runtime/Engine/Classes/Engine/Engine.h>
#define ScreenDebug(text) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, TEXT(text))
#else
#define ScreenDebug(text) 
#endif

AZhoenusPlayerController::AZhoenusPlayerController()
{
	{
		static ConstructorHelpers::FClassFinder<UZhoenusTouchUI> TouchUIFinder(TEXT("/Game/Touch/TouchUI"));
		if (TouchUIFinder.Class != nullptr)
		{
			wTouchUI = TouchUIFinder.Class;
		}
	}
}

AZhoenusPlayerController::~AZhoenusPlayerController()
{

}

void AZhoenusPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && SVirtualJoystick::ShouldDisplayTouchInterface() && wTouchUI)
	{
		touchUI = CreateWidget<UZhoenusTouchUI>(this, wTouchUI);
		if (touchUI)
		{
			touchUI->AddToViewport();

			touchUI->ButtonFire->OnPressed.AddDynamic(this, &AZhoenusPlayerController::TouchFirePressed);
			touchUI->ButtonFire->OnReleased.AddDynamic(this, &AZhoenusPlayerController::TouchFireReleased);
			touchUI->ButtonStabilize->OnPressed.AddDynamic(this, &AZhoenusPlayerController::TouchAutoCorrectPressed);
			touchUI->ButtonStabilize->OnReleased.AddDynamic(this, &AZhoenusPlayerController::TouchAutoCorrectReleased);
			//touchUI->ButtonFire->OnClicked.AddDynamic(this, &AZhoenusPlayerController::TestClick);
		}
	}
}

void AZhoenusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	//InputComponent->BindAxis("Thrust", this, &AZhoenusPlayerController::ThrustInput);
	//InputComponent->BindAxis("MoveUp", this, &AZhoenusPlayerController::MoveUpInput);
	//InputComponent->BindAxis("MoveRight", this, &AZhoenusPlayerController::RotateRightInput);
	//InputComponent->BindAxis("RotateRight", this, &AZhoenusPlayerController::MoveRightInput);
	//if (!SVirtualJoystick::ShouldDisplayTouchInterface())
	//{
	//	InputComponent->BindAxis("DisengageAutoCorrect", this, &AZhoenusPlayerController::DisengageAutoCorrect);
	//	InputComponent->BindAxis("FireWeapon", this, &AZhoenusPlayerController::FireWeapon);
	//}
}

void AZhoenusPlayerController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
	ScreenDebug("Possession");
}

void AZhoenusPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	ScreenDebug("Unpossessed");
}

void AZhoenusPlayerController::ThrustInput(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->ThrustInput(Val);
	}
}

void AZhoenusPlayerController::ServerThrustInput_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->ThrustInput(Val);
	}
}

bool AZhoenusPlayerController::ServerThrustInput_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::MoveUpInput(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->MoveUpInput(Val);
	}
}

void AZhoenusPlayerController::ServerMoveUpInput_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->MoveUpInput(Val);
	}
}

bool AZhoenusPlayerController::ServerMoveUpInput_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::MoveRightInput(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->MoveRightInput(Val);
	}
}

void AZhoenusPlayerController::ServerMoveRightInput_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->MoveRightInput(Val);
	}
}

bool AZhoenusPlayerController::ServerMoveRightInput_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::RotateRightInput(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->RotateRightInput(Val);
	}
}

void AZhoenusPlayerController::ServerRotateRightInput_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		pawn->RotateRightInput(Val);
	}
}

bool AZhoenusPlayerController::ServerRotateRightInput_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::DisengageAutoCorrect(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Engage Auto Correct %f"), Val);
		pawn->DisengageAutoCorrect(Val);
	}
}

void AZhoenusPlayerController::ServerDisengageAutoCorrect_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Engage Auto Correct %f"), Val);
		pawn->DisengageAutoCorrect(Val);
	}
}

bool AZhoenusPlayerController::ServerDisengageAutoCorrect_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::FireWeapon(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Fire Weapon %f"), Val);
		pawn->FireWeapon(Val);
	}
}

void AZhoenusPlayerController::ServerFireWeapon_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Fire Weapon %f"), Val);
		pawn->FireWeapon(Val);
	}
}

bool AZhoenusPlayerController::ServerFireWeapon_Validate(float)
{
	return true;
}

void AZhoenusPlayerController::TouchFirePressed() //_Implementation()
{
	UE_LOG(LogZhoenus, Display, TEXT("TouchFirePressed"));
	FireWeapon(0.95f);
}

void AZhoenusPlayerController::TouchFireReleased() //_Implementation()
{
	UE_LOG(LogZhoenus, Display, TEXT("TouchFireReleased"));
	FireWeapon(0.0f);
}

void AZhoenusPlayerController::TouchAutoCorrectPressed() //_Implementation()
{
	UE_LOG(LogZhoenus, Display, TEXT("TouchAutoCorrectPressed"));
	DisengageAutoCorrect(1.0f);
}

void AZhoenusPlayerController::TouchAutoCorrectReleased() //_Implementation()
{
	UE_LOG(LogZhoenus, Display, TEXT("TouchAutoCorrectReleased"));
	DisengageAutoCorrect(0.0f);
}

//void AZhoenusPlayerController::TestClick()
//{
//	UE_LOG(LogZhoenus, Display, TEXT("test click"));
//}