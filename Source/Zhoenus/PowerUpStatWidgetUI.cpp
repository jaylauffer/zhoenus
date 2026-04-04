#include "PowerUpStatWidgetUI.h"

#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/Widget.h"
#include "UObject/UnrealType.h"

UPowerUpStatWidgetUI::UPowerUpStatWidgetUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPowerUpStatWidgetUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	DisplayedStatValue = CommonNumericTextBlock_StatValue ? CommonNumericTextBlock_StatValue->CurrentNumericValue : 0.f;
	ApplyDisplayLabelFromBlueprint();
	ConfigureButton(RB_Decrement);
	ConfigureButton(RB_Increment);

	if (RB_Decrement)
	{
		RB_Decrement->OnClicked().Clear();
		RB_Decrement->OnClicked().AddUObject(this, &ThisClass::HandleDecrementClicked);
	}

	if (RB_Increment)
	{
		RB_Increment->OnClicked().Clear();
		RB_Increment->OnClicked().AddUObject(this, &ThisClass::HandleIncrementClicked);
	}

	ConfigureNavigation();
}

void UPowerUpStatWidgetUI::GetStatValue(int32& StatValue) const
{
	StatValue = FMath::RoundToInt(DisplayedStatValue);
}

void UPowerUpStatWidgetUI::SetStatValue(int32 NewStatValue)
{
	ApplyDisplayedStatValue(static_cast<float>(NewStatValue));
}

void UPowerUpStatWidgetUI::SetDisplayLabelText(const FText& NewLabel)
{
	if (CommonTextBlock_StatName)
	{
		CommonTextBlock_StatName->SetText(NewLabel);
	}
}

float UPowerUpStatWidgetUI::GetDisplayedStatValue() const
{
	return DisplayedStatValue;
}

void UPowerUpStatWidgetUI::SetDisplayedStatValue(float NewValue)
{
	ApplyDisplayedStatValue(NewValue);
}

void UPowerUpStatWidgetUI::ConfigureButton(UCommonButtonBase* Button) const
{
	if (!Button)
	{
		return;
	}

	Button->SetIsFocusable(true);
	Button->SetIsSelectable(true);
	Button->SetIsInteractableWhenSelected(true);
	Button->SetShouldSelectUponReceivingFocus(true);
}

void UPowerUpStatWidgetUI::ConfigureNavigation() const
{
	if (RB_Decrement && RB_Increment)
	{
		RB_Decrement->SetNavigationRuleExplicit(EUINavigation::Right, RB_Increment);
		RB_Increment->SetNavigationRuleExplicit(EUINavigation::Left, RB_Decrement);
		RB_Decrement->SetNavigationRuleExplicit(EUINavigation::Next, RB_Increment);
		RB_Increment->SetNavigationRuleExplicit(EUINavigation::Previous, RB_Decrement);
	}
}

void UPowerUpStatWidgetUI::HandleDecrementClicked()
{
	ApplyDisplayedStatValue(DisplayedStatValue - 1.f);
}

void UPowerUpStatWidgetUI::HandleIncrementClicked()
{
	ApplyDisplayedStatValue(DisplayedStatValue + 1.f);
}

void UPowerUpStatWidgetUI::ApplyDisplayedStatValue(float NewValue)
{
	DisplayedStatValue = NewValue;

	if (CommonNumericTextBlock_StatValue)
	{
		CommonNumericTextBlock_StatValue->SetCurrentValue(DisplayedStatValue);
	}
}

void UPowerUpStatWidgetUI::ApplyDisplayLabelFromBlueprint()
{
	if (!CommonTextBlock_StatName)
	{
		return;
	}

	static const FName DisplayLabelPropertyName(TEXT("DisplayLabel"));
	const FTextProperty* DisplayLabelProperty = FindFProperty<FTextProperty>(GetClass(), DisplayLabelPropertyName);
	if (!DisplayLabelProperty)
	{
		return;
	}

	const FText* DisplayLabelValue = DisplayLabelProperty->ContainerPtrToValuePtr<FText>(this);
	if (!DisplayLabelValue)
	{
		return;
	}

	CommonTextBlock_StatName->SetText(*DisplayLabelValue);
}

UWidget* UPowerUpStatWidgetUI::NativeGetDesiredFocusTarget() const
{
	if (RB_Decrement)
	{
		return RB_Decrement;
	}
	if (RB_Increment)
	{
		return RB_Increment;
	}
	return Super::NativeGetDesiredFocusTarget();
}
