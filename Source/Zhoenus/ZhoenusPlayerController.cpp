#include "ZhoenusPlayerController.h"
#include "Zhoenus.h"
#include "SaveThemAllGameInstance.h"
#include "ZhoenusPawn.h"
#include "ZhoenusThumbstick.h"
#include "ZhoenusTouchUI.h"
#include "Components/Button.h"
#include "Misc/OutputDevice.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Parse.h"
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
		static ConstructorHelpers::FClassFinder<UZhoenusTouchUI> TouchUIFinder(TEXT("/Game/Blueprints/UI/Touch/loadngoTouch"));
		if (TouchUIFinder.Class != nullptr)
		{
			wTouchUI = TouchUIFinder.Class;
		}
	}
}

AZhoenusPlayerController::~AZhoenusPlayerController()
{

}

TSharedPtr<SVirtualJoystick> AZhoenusPlayerController::CreateVirtualJoystick()
{
	TSharedRef<ZhoenusThumbstick> Thumbstick = SNew(ZhoenusThumbstick)
		.OnStickPressureChanged(
			FOnStickPressureChanged::CreateUObject(
				this,
				&AZhoenusPlayerController::HandleStickPressureChanged));
	PressureThumbstick = Thumbstick;
	return StaticCastSharedRef<SVirtualJoystick>(Thumbstick);
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
bool AZhoenusPlayerController::TryHandleDebugConsoleCommand(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (!FParse::Command(&Cmd, TEXT("GrantPowerPoints")))
	{
		return false;
	}

	float Amount = 10.f;
	if (Cmd && *Cmd)
	{
		Amount = FCString::Atof(Cmd);
	}

	GrantPowerPoints(Amount);
	Ar.Logf(TEXT("GrantPowerPoints %.2f"), Amount);
	return true;
}

void AZhoenusPlayerController::GrantPowerPoints(float Amount)
{
	USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	if (!GameInstance)
	{
		const FString Message = TEXT("GrantPowerPoints failed: SaveThemAllGameInstance not available.");
		UE_LOG(LogZhoenus, Warning, TEXT("%s"), *Message);
		ClientMessage(Message);
		return;
	}

	const float GrantAmount = FMath::Max(0.f, Amount);
	if (GrantAmount <= 0.f)
	{
		const FString Message = TEXT("GrantPowerPoints ignored: amount must be greater than zero.");
		UE_LOG(LogZhoenus, Warning, TEXT("%s"), *Message);
		ClientMessage(Message);
		return;
	}

	GameInstance->points += GrantAmount;
	GameInstance->AcquiredPoints += GrantAmount;
	GameInstance->SaveGame();

	const FString Message = FString::Printf(TEXT("Granted %.2f power-up points. Current points: %.2f"), GrantAmount, GameInstance->points);
	UE_LOG(LogZhoenus, Display, TEXT("%s"), *Message);
	ClientMessage(Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, Message);
	}
}
#endif

bool AZhoenusPlayerController::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	if (TryHandleDebugConsoleCommand(Cmd, Ar))
	{
		return true;
	}
#endif

	return Super::ProcessConsoleExec(Cmd, Ar, Executor);
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

			if (touchUI->ButtonFire)
			{
				touchUI->ButtonFire->OnPressed.AddDynamic(this, &AZhoenusPlayerController::TouchFirePressed);
				touchUI->ButtonFire->OnReleased.AddDynamic(this, &AZhoenusPlayerController::TouchFireReleased);
			}
			if (touchUI->ButtonStabilize)
			{
				touchUI->ButtonStabilize->OnPressed.AddDynamic(this, &AZhoenusPlayerController::TouchAutoCorrectPressed);
				touchUI->ButtonStabilize->OnReleased.AddDynamic(this, &AZhoenusPlayerController::TouchAutoCorrectReleased);
			}
			UpdateTouchToggleVisuals();
			RefreshTouchPressureActions();
		}
	}
}

void AZhoenusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
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

void AZhoenusPlayerController::HandleStickPressureChanged(int32 ControlIndex, float Pressure, bool bIsActive)
{
	UE_LOG(
		LogZhoenus,
		Verbose,
		TEXT("Touch stick pressure control=%d pressure=%.3f active=%s"),
		ControlIndex,
		Pressure,
		bIsActive ? TEXT("true") : TEXT("false"));
	RefreshTouchPressureActions();
}

float AZhoenusPlayerController::NormalizeTouchPressure(
	const float RawPressure,
	const float Deadzone,
	const float Scale) const
{
	const float ClampedPressure = FMath::Clamp(RawPressure, 0.0f, 1.0f);
	const float ClampedDeadzone = FMath::Clamp(Deadzone, 0.0f, 0.95f);
	if (ClampedPressure <= ClampedDeadzone)
	{
		return 0.0f;
	}

	const float NormalizedPressure = (ClampedPressure - ClampedDeadzone) / (1.0f - ClampedDeadzone);
	return FMath::Clamp(NormalizedPressure * FMath::Max(0.0f, Scale), 0.0f, 1.0f);
}

void AZhoenusPlayerController::RefreshTouchPressureActions()
{
	const float StabilizePressure =
		bTouchStabilizePressureModeEnabled && PressureThumbstick.IsValid()
			? NormalizeTouchPressure(
				PressureThumbstick->GetControlPressure(StabilizePressureControlIndex),
				StabilizePressureDeadzone,
				StabilizePressureScale)
			: 0.0f;

	const float FirePressure =
		bTouchFirePressureModeEnabled && PressureThumbstick.IsValid()
			? NormalizeTouchPressure(
				PressureThumbstick->GetControlPressure(FirePressureControlIndex),
				FirePressureDeadzone,
				FirePressureScale)
			: 0.0f;

	DisengageAutoCorrect(StabilizePressure);
	FireWeapon(FirePressure);
}

void AZhoenusPlayerController::UpdateTouchToggleVisuals() const
{
	if (!touchUI)
	{
		return;
	}

	if (touchUI->ButtonFire)
	{
		touchUI->ButtonFire->SetBackgroundColor(
			bTouchFirePressureModeEnabled ? TouchFireToggleActiveColor : TouchToggleInactiveColor);
	}

	if (touchUI->ButtonStabilize)
	{
		touchUI->ButtonStabilize->SetBackgroundColor(
			bTouchStabilizePressureModeEnabled ? TouchStabilizeToggleActiveColor : TouchToggleInactiveColor);
	}
}

void AZhoenusPlayerController::DisengageAutoCorrect(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Engage Auto Correct %f"), Val);
		pawn->OrigDisengageAutoCorrect(Val);
		//ServerDisengageAutoCorrect(Val);
	}
}

void AZhoenusPlayerController::ServerDisengageAutoCorrect_Implementation(float Val)
{
	AZhoenusPawn* pawn{ Cast<AZhoenusPawn>(GetPawn()) };
	if (pawn)
	{
		//UE_LOG(LogZhoenus, Display, TEXT("Engage Auto Correct %f"), Val);
		pawn->OrigDisengageAutoCorrect(Val);
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
		//pawn->FireWeapon(Val);
		ServerFireWeapon(Val);

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
	bTouchFirePressureModeEnabled = !bTouchFirePressureModeEnabled;
	UE_LOG(
		LogZhoenus,
		Display,
		TEXT("TouchFirePressed pressure_mode=%s"),
		bTouchFirePressureModeEnabled ? TEXT("enabled") : TEXT("disabled"));
	UpdateTouchToggleVisuals();
	RefreshTouchPressureActions();
}

void AZhoenusPlayerController::TouchFireReleased() //_Implementation()
{
	UE_LOG(LogZhoenus, Verbose, TEXT("TouchFireReleased"));
}

void AZhoenusPlayerController::TouchAutoCorrectPressed() //_Implementation()
{
	bTouchStabilizePressureModeEnabled = !bTouchStabilizePressureModeEnabled;
	UE_LOG(
		LogZhoenus,
		Display,
		TEXT("TouchAutoCorrectPressed pressure_mode=%s"),
		bTouchStabilizePressureModeEnabled ? TEXT("enabled") : TEXT("disabled"));
	UpdateTouchToggleVisuals();
	RefreshTouchPressureActions();
}

void AZhoenusPlayerController::TouchAutoCorrectReleased() //_Implementation()
{
	UE_LOG(LogZhoenus, Verbose, TEXT("TouchAutoCorrectReleased"));
}
