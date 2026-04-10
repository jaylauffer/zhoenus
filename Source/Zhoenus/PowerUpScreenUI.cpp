#include "PowerUpScreenUI.h"

#include "CommonButtonBase.h"
#include "Components/Widget.h"

UPowerUpScreenUI::UPowerUpScreenUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPowerUpScreenUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	ConfigureButton(ZhoenusButton_Again);
	ConfigureButton(ZhoenusButton_Convert);
	ConfigureButton(ZhoenusButton_MainMenu);
	ConfigureButton(ZhoenusButton_PowerUp);
	ConfigureNavigation();
}

void UPowerUpScreenUI::ConfigureButton(UCommonButtonBase* Button) const
{
	if (!Button)
	{
		return;
	}

	Button->SetIsFocusable(true);
	Button->SetIsSelectable(false);
}

void UPowerUpScreenUI::ConfigureNavigation() const
{
	if (ZhoenusButton_Convert && ZhoenusButton_PowerUp)
	{
		ZhoenusButton_Convert->SetNavigationRuleExplicit(EUINavigation::Right, ZhoenusButton_PowerUp);
		ZhoenusButton_PowerUp->SetNavigationRuleExplicit(EUINavigation::Left, ZhoenusButton_Convert);
	}

	if (ZhoenusButton_MainMenu && ZhoenusButton_Again)
	{
		ZhoenusButton_MainMenu->SetNavigationRuleExplicit(EUINavigation::Right, ZhoenusButton_Again);
		ZhoenusButton_Again->SetNavigationRuleExplicit(EUINavigation::Left, ZhoenusButton_MainMenu);
	}

	if (ZhoenusButton_Convert && ZhoenusButton_MainMenu)
	{
		ZhoenusButton_Convert->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusButton_MainMenu);
		ZhoenusButton_MainMenu->SetNavigationRuleExplicit(EUINavigation::Up, ZhoenusButton_Convert);
		ZhoenusButton_Convert->SetNavigationRuleExplicit(EUINavigation::Next, ZhoenusButton_MainMenu);
		ZhoenusButton_MainMenu->SetNavigationRuleExplicit(EUINavigation::Previous, ZhoenusButton_Convert);
	}

	if (ZhoenusButton_PowerUp && ZhoenusButton_Again)
	{
		ZhoenusButton_PowerUp->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusButton_Again);
		ZhoenusButton_Again->SetNavigationRuleExplicit(EUINavigation::Up, ZhoenusButton_PowerUp);
		ZhoenusButton_PowerUp->SetNavigationRuleExplicit(EUINavigation::Next, ZhoenusButton_Again);
		ZhoenusButton_Again->SetNavigationRuleExplicit(EUINavigation::Previous, ZhoenusButton_PowerUp);
	}
}

UWidget* UPowerUpScreenUI::NativeGetDesiredFocusTarget() const
{
	if (ZhoenusButton_Convert)
	{
		return ZhoenusButton_Convert;
	}
	if (ZhoenusButton_PowerUp)
	{
		return ZhoenusButton_PowerUp;
	}
	if (ZhoenusButton_MainMenu)
	{
		return ZhoenusButton_MainMenu;
	}
	if (ZhoenusButton_Again)
	{
		return ZhoenusButton_Again;
	}
	return Super::NativeGetDesiredFocusTarget();
}
