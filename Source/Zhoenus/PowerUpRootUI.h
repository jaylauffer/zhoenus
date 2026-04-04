#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PowerUpRootUI.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;
class UWidget;

UCLASS()
class UPowerUpRootUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPowerUpRootUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Focus")
	UCommonActivatableWidgetStack* WidgetStack;

private:
	void HandleDisplayedWidgetChanged(UCommonActivatableWidget* DisplayedWidget);
	void RefreshActiveFocus() const;
	UWidget* ResolveActiveFocusTarget() const;
};
