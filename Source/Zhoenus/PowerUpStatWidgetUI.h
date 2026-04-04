#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PowerUpStatWidgetUI.generated.h"

class UCommonButtonBase;
class UCommonNumericTextBlock;
class UCommonTextBlock;
class UWidget;

UCLASS()
class UPowerUpStatWidgetUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPowerUpStatWidgetUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintPure, Category = "PowerUp")
	void GetStatValue(int32& StatValue) const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void SetStatValue(int32 NewStatValue);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void SetDisplayLabelText(const FText& NewLabel);

	float GetDisplayedStatValue() const;
	void SetDisplayedStatValue(float NewValue);

	UCommonButtonBase* GetDecrementButton() const { return RB_Decrement; }
	UCommonButtonBase* GetIncrementButton() const { return RB_Increment; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* RB_Decrement;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* RB_Increment;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonNumericTextBlock* CommonNumericTextBlock_StatValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonTextBlock* CommonTextBlock_StatName;

private:
	void ConfigureButton(UCommonButtonBase* Button) const;
	void ConfigureNavigation() const;
	void HandleDecrementClicked();
	void HandleIncrementClicked();
	void ApplyDisplayedStatValue(float NewValue);
	void ApplyDisplayLabelFromBlueprint();

	float DisplayedStatValue = 0.f;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
};
