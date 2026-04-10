#include "ConvertSpeedUI.h"

#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "Components/SpinBox.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "SaveThemAllGameInstance.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* RepeatClickSoundPath = TEXT("/Engine/VREditor/Sounds/UI/Click_on_Button.Click_on_Button");
}

UConvertSpeedUI::UConvertSpeedUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundBase> RepeatClickSoundAsset(RepeatClickSoundPath);
	RepeatClickSound = RepeatClickSoundAsset.Object;
}

void UConvertSpeedUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	ConfigureButton(ZhoenusButton_Decrement_Forward);
	ConfigureButton(ZhoenusButton_Increment_Forward);
	ConfigureButton(ZhoenusButton_Decrement_Reverse);
	ConfigureButton(ZhoenusButton_Increment_Reverse);
	BindRepeatButton(ZhoenusButton_Decrement_Forward, &UConvertSpeedUI::HandleDecrementForwardPressed, &UConvertSpeedUI::HandleDecrementForwardReleased);
	BindRepeatButton(ZhoenusButton_Increment_Forward, &UConvertSpeedUI::HandleIncrementForwardPressed, &UConvertSpeedUI::HandleIncrementForwardReleased);
	BindRepeatButton(ZhoenusButton_Decrement_Reverse, &UConvertSpeedUI::HandleDecrementReversePressed, &UConvertSpeedUI::HandleDecrementReverseReleased);
	BindRepeatButton(ZhoenusButton_Increment_Reverse, &UConvertSpeedUI::HandleIncrementReversePressed, &UConvertSpeedUI::HandleIncrementReverseReleased);

	if (SpinBox_MaxForwardSpeed)
	{
		SpinBox_MaxForwardSpeed->OnValueChanged.RemoveAll(this);
		SpinBox_MaxForwardSpeed->OnValueChanged.AddDynamic(this, &UConvertSpeedUI::HandleForwardValueChanged);
	}
	if (SpinBox_MaxReverseSpeed)
	{
		SpinBox_MaxReverseSpeed->OnValueChanged.RemoveAll(this);
		SpinBox_MaxReverseSpeed->OnValueChanged.AddDynamic(this, &UConvertSpeedUI::HandleReverseValueChanged);
	}

	if (const USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>())
	{
		InitializeSpinBox(SpinBox_MaxForwardSpeed, GameInstance->GetConvertibleForwardSpeed());
		InitializeSpinBox(SpinBox_MaxReverseSpeed, GameInstance->GetConvertibleReverseSpeed());
	}

	ConfigureNavigation();
	RefreshConvertedPointPreview();
}

void UConvertSpeedUI::NativeOnDeactivated()
{
	ClearRepeatState();
	Super::NativeOnDeactivated();
}

UWidget* UConvertSpeedUI::NativeGetDesiredFocusTarget() const
{
	if (ZhoenusButton_Decrement_Forward)
	{
		return ZhoenusButton_Decrement_Forward;
	}
	if (ZhoenusButton_Increment_Forward)
	{
		return ZhoenusButton_Increment_Forward;
	}
	if (SpinBox_MaxForwardSpeed)
	{
		return SpinBox_MaxForwardSpeed;
	}
	return Super::NativeGetDesiredFocusTarget();
}

void UConvertSpeedUI::ConfigureButton(UCommonButtonBase* Button) const
{
	if (!Button)
	{
		return;
	}

	Button->SetIsFocusable(true);
	Button->SetIsSelectable(false);
}

void UConvertSpeedUI::ConfigureNavigation() const
{
	if (ZhoenusButton_Decrement_Forward && ZhoenusButton_Increment_Forward)
	{
		ZhoenusButton_Decrement_Forward->SetNavigationRuleExplicit(EUINavigation::Right, ZhoenusButton_Increment_Forward);
		ZhoenusButton_Increment_Forward->SetNavigationRuleExplicit(EUINavigation::Left, ZhoenusButton_Decrement_Forward);
		ZhoenusButton_Decrement_Forward->SetNavigationRuleExplicit(EUINavigation::Next, ZhoenusButton_Increment_Forward);
		ZhoenusButton_Increment_Forward->SetNavigationRuleExplicit(EUINavigation::Previous, ZhoenusButton_Decrement_Forward);
	}

	if (ZhoenusButton_Decrement_Reverse && ZhoenusButton_Increment_Reverse)
	{
		ZhoenusButton_Decrement_Reverse->SetNavigationRuleExplicit(EUINavigation::Right, ZhoenusButton_Increment_Reverse);
		ZhoenusButton_Increment_Reverse->SetNavigationRuleExplicit(EUINavigation::Left, ZhoenusButton_Decrement_Reverse);
		ZhoenusButton_Decrement_Reverse->SetNavigationRuleExplicit(EUINavigation::Next, ZhoenusButton_Increment_Reverse);
		ZhoenusButton_Increment_Reverse->SetNavigationRuleExplicit(EUINavigation::Previous, ZhoenusButton_Decrement_Reverse);
	}

	if (ZhoenusButton_Decrement_Forward && ZhoenusButton_Decrement_Reverse)
	{
		ZhoenusButton_Decrement_Forward->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusButton_Decrement_Reverse);
		ZhoenusButton_Decrement_Reverse->SetNavigationRuleExplicit(EUINavigation::Up, ZhoenusButton_Decrement_Forward);
	}

	if (ZhoenusButton_Increment_Forward && ZhoenusButton_Increment_Reverse)
	{
		ZhoenusButton_Increment_Forward->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusButton_Increment_Reverse);
		ZhoenusButton_Increment_Reverse->SetNavigationRuleExplicit(EUINavigation::Up, ZhoenusButton_Increment_Forward);
	}
}

void UConvertSpeedUI::BindRepeatButton(UCommonButtonBase* Button, void (UConvertSpeedUI::*PressedHandler)(), void (UConvertSpeedUI::*ReleasedHandler)())
{
	if (!Button)
	{
		return;
	}

	Button->OnPressed().RemoveAll(this);
	Button->OnReleased().RemoveAll(this);
	Button->OnPressed().AddUObject(this, PressedHandler);
	Button->OnReleased().AddUObject(this, ReleasedHandler);
}

void UConvertSpeedUI::InitializeSpinBox(USpinBox* SpinBox, float ConvertibleSpeed)
{
	if (!SpinBox)
	{
		return;
	}

	const float ClampedConvertibleSpeed = FMath::Max(0.f, ConvertibleSpeed);
	SpinBox->SetMinValue(0.f);
	SpinBox->SetMinSliderValue(0.f);
	SpinBox->SetMaxValue(ClampedConvertibleSpeed);
	SpinBox->SetMaxSliderValue(ClampedConvertibleSpeed);
	SpinBox->SetValue(ClampedConvertibleSpeed);
}

void UConvertSpeedUI::AdjustSpinBox(USpinBox* SpinBox, float Direction)
{
	AdjustSpinBoxWithFeedback(SpinBox, Direction, false);
}

void UConvertSpeedUI::AdjustSpinBoxWithFeedback(USpinBox* SpinBox, float Direction, bool bPlaySound)
{
	if (!SpinBox || FMath::IsNearlyZero(Direction))
	{
		return;
	}

	const float StepSize = GetStepSize(SpinBox);
	const float NewValue = FMath::Clamp(
		SpinBox->GetValue() + (Direction * StepSize),
		0.f,
		SpinBox->GetMaxValue());
	SpinBox->SetValue(NewValue);

	if (bPlaySound)
	{
		PlayRepeatSound();
	}
}

void UConvertSpeedUI::RefreshConvertedPointPreview()
{
	const USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	const float ForwardAmount = SpinBox_MaxForwardSpeed ? SpinBox_MaxForwardSpeed->GetValue() : 0.f;
	const float ReverseAmount = SpinBox_MaxReverseSpeed ? SpinBox_MaxReverseSpeed->GetValue() : 0.f;
	const float ForwardPoints = GameInstance->Convert(ForwardAmount);
	const float ReversePoints = GameInstance->Convert(ReverseAmount);

	if (CommonNumericTextBlock_Forward)
	{
		CommonNumericTextBlock_Forward->SetCurrentValue(ForwardPoints);
	}
	if (CommonNumericTextBlock_Reverse)
	{
		CommonNumericTextBlock_Reverse->SetCurrentValue(ReversePoints);
	}
	if (CommonNumericTextBlock_Total)
	{
		CommonNumericTextBlock_Total->SetCurrentValue(ForwardPoints + ReversePoints);
	}
}

float UConvertSpeedUI::GetStepSize(const USpinBox* SpinBox) const
{
	if (!SpinBox)
	{
		return 1.f;
	}

	const float Delta = SpinBox->GetDelta();
	return Delta > 0.f ? Delta : 1.f;
}

void UConvertSpeedUI::StartRepeat(UCommonButtonBase* Button, USpinBox* SpinBox, float Direction)
{
	if (!Button || !SpinBox || FMath::IsNearlyZero(Direction))
	{
		return;
	}

	ClearRepeatState();
	ActiveRepeatButton = Button;
	ActiveRepeatSpinBox = SpinBox;
	ActiveRepeatDirection = Direction;
	AdjustSpinBoxWithFeedback(SpinBox, Direction, true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RepeatTimerHandle, this, &UConvertSpeedUI::HandleRepeatTick, RepeatInterval, true, RepeatInterval);
	}
}

void UConvertSpeedUI::StopRepeat(UCommonButtonBase* Button)
{
	if (!ActiveRepeatButton.IsValid() || ActiveRepeatButton.Get() != Button)
	{
		return;
	}

	ClearRepeatState();
}

void UConvertSpeedUI::HandleRepeatTick()
{
	if (!ActiveRepeatSpinBox.IsValid() || FMath::IsNearlyZero(ActiveRepeatDirection))
	{
		ClearRepeatState();
		return;
	}

	AdjustSpinBoxWithFeedback(ActiveRepeatSpinBox.Get(), ActiveRepeatDirection, true);
}

void UConvertSpeedUI::ClearRepeatState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RepeatTimerHandle);
	}

	ActiveRepeatButton.Reset();
	ActiveRepeatSpinBox.Reset();
	ActiveRepeatDirection = 0.f;
}

void UConvertSpeedUI::PlayRepeatSound() const
{
	if (RepeatClickSound != nullptr)
	{
		UGameplayStatics::PlaySound2D(this, RepeatClickSound);
	}
}

void UConvertSpeedUI::HandleForwardValueChanged(float InValue)
{
	RefreshConvertedPointPreview();
}

void UConvertSpeedUI::HandleReverseValueChanged(float InValue)
{
	RefreshConvertedPointPreview();
}

void UConvertSpeedUI::HandleDecrementForwardClicked()
{
	AdjustSpinBox(SpinBox_MaxForwardSpeed, -1.f);
}

void UConvertSpeedUI::HandleIncrementForwardClicked()
{
	AdjustSpinBox(SpinBox_MaxForwardSpeed, 1.f);
}

void UConvertSpeedUI::HandleDecrementReverseClicked()
{
	AdjustSpinBox(SpinBox_MaxReverseSpeed, -1.f);
}

void UConvertSpeedUI::HandleIncrementReverseClicked()
{
	AdjustSpinBox(SpinBox_MaxReverseSpeed, 1.f);
}

void UConvertSpeedUI::HandleDecrementForwardPressed()
{
	StartRepeat(ZhoenusButton_Decrement_Forward, SpinBox_MaxForwardSpeed, -1.f);
}

void UConvertSpeedUI::HandleDecrementForwardReleased()
{
	StopRepeat(ZhoenusButton_Decrement_Forward);
}

void UConvertSpeedUI::HandleIncrementForwardPressed()
{
	StartRepeat(ZhoenusButton_Increment_Forward, SpinBox_MaxForwardSpeed, 1.f);
}

void UConvertSpeedUI::HandleIncrementForwardReleased()
{
	StopRepeat(ZhoenusButton_Increment_Forward);
}

void UConvertSpeedUI::HandleDecrementReversePressed()
{
	StartRepeat(ZhoenusButton_Decrement_Reverse, SpinBox_MaxReverseSpeed, -1.f);
}

void UConvertSpeedUI::HandleDecrementReverseReleased()
{
	StopRepeat(ZhoenusButton_Decrement_Reverse);
}

void UConvertSpeedUI::HandleIncrementReversePressed()
{
	StartRepeat(ZhoenusButton_Increment_Reverse, SpinBox_MaxReverseSpeed, 1.f);
}

void UConvertSpeedUI::HandleIncrementReverseReleased()
{
	StopRepeat(ZhoenusButton_Increment_Reverse);
}
