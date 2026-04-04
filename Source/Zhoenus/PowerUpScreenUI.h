#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PowerUpScreenUI.generated.h"

class UCommonButtonBase;
class UWidget;

UCLASS()
class UPowerUpScreenUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPowerUpScreenUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Again;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Convert;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_MainMenu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_PowerUp;

private:
	void ConfigureButton(UCommonButtonBase* Button) const;
	void ConfigureNavigation() const;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
};
