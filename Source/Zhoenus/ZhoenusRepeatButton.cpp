#include "ZhoenusRepeatButton.h"
#include "TimerManager.h"
#include "Engine/World.h"

UZhoenusRepeatButton::UZhoenusRepeatButton()
{
    // Constructor logic (if any)
}

void UZhoenusRepeatButton::NativeConstruct()
{
    Super::NativeConstruct();

    // Must require hold so it ticks
    bRequiresHold = true;
    // Set a large hold time so we don't stop automatically
    HoldTime = 9999.f;
    // No rollback
    HoldRollbackTime = 0.f;
}

void UZhoenusRepeatButton::NativeOnPressed()
{
    // Always call Super to ensure parent logic runs
    Super::NativeOnPressed();

    if (IsInteractionEnabled())
    {
        HandleTriggeringActionCommited(); // Simulates immediate "click"
    }
}

void UZhoenusRepeatButton::NativeOnReleased()
{
	// Always call Super to ensure parent logic runs
	Super::NativeOnReleased();
	// Reset the hold time
	Super::HoldReset();
}

bool UZhoenusRepeatButton::NativeOnHoldProgress(float DeltaTime)
{
	CurrentHoldTime += DeltaTime;
	if (CurrentHoldTime >= RepeatInterval)
	{
        CurrentHoldProgress = 1.f;
		HandleTriggeringActionCommited();
	}
    //true continues ticking...
    return true;
}

void UZhoenusRepeatButton::HoldReset()
{
	CurrentHoldTime = 0.f;
	CurrentHoldProgress = 0.f;
}
