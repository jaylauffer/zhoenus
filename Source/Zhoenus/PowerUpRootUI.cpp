#include "PowerUpRootUI.h"

#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Components/Widget.h"

UPowerUpRootUI::UPowerUpRootUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPowerUpRootUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (WidgetStack)
	{
		WidgetStack->OnDisplayedWidgetChanged().RemoveAll(this);
		WidgetStack->OnDisplayedWidgetChanged().AddUObject(this, &UPowerUpRootUI::HandleDisplayedWidgetChanged);
	}
}

void UPowerUpRootUI::NativeDestruct()
{
	if (WidgetStack)
	{
		WidgetStack->OnDisplayedWidgetChanged().RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPowerUpRootUI::NativeOnActivated()
{
	Super::NativeOnActivated();
	RefreshActiveFocus();
	RequestRefreshFocus();
}

UWidget* UPowerUpRootUI::NativeGetDesiredFocusTarget() const
{
	if (UWidget* FocusTarget = ResolveActiveFocusTarget())
	{
		return FocusTarget;
	}

	return WidgetStack ? WidgetStack : Super::NativeGetDesiredFocusTarget();
}

void UPowerUpRootUI::HandleDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget)
{
	if (!DisplayedWidget || !IsActivated())
	{
		return;
	}

	RefreshActiveFocus();
	RequestRefreshFocus();
}

void UPowerUpRootUI::RefreshActiveFocus() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	UWidget* FocusTarget = ResolveActiveFocusTarget();
	if (!PlayerController || !FocusTarget)
	{
		return;
	}

	FocusTarget->SetUserFocus(PlayerController);
}

UWidget* UPowerUpRootUI::ResolveActiveFocusTarget() const
{
	if (!WidgetStack)
	{
		return nullptr;
	}

	if (UCommonActivatableWidget* ActiveWidget = WidgetStack->GetActiveWidget())
	{
		if (UWidget* DesiredFocusTarget = ActiveWidget->GetDesiredFocusTarget())
		{
			return DesiredFocusTarget;
		}

		return ActiveWidget;
	}

	if (UCommonActivatableWidget* RootContent = WidgetStack->GetRootContent())
	{
		if (UWidget* DesiredFocusTarget = RootContent->GetDesiredFocusTarget())
		{
			return DesiredFocusTarget;
		}

		return RootContent;
	}

	return WidgetStack;
}
