#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ConvertSpeedUI.generated.h"

class UCommonButtonBase;
class UCommonNumericTextBlock;
class USoundBase;
class USpinBox;
class UWidget;
struct FTimerHandle;

UCLASS()
class UConvertSpeedUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UConvertSpeedUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeOnDeactivated() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Decrement_Forward;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Decrement_Reverse;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Increment_Forward;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonButtonBase* ZhoenusButton_Increment_Reverse;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USpinBox* SpinBox_MaxForwardSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USpinBox* SpinBox_MaxReverseSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonNumericTextBlock* CommonNumericTextBlock_Forward;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonNumericTextBlock* CommonNumericTextBlock_Reverse;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonNumericTextBlock* CommonNumericTextBlock_Total;

private:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void ConfigureButton(UCommonButtonBase* Button) const;
	void ConfigureNavigation() const;
	void BindRepeatButton(UCommonButtonBase* Button, void (UConvertSpeedUI::*PressedHandler)(), void (UConvertSpeedUI::*ReleasedHandler)());
	void InitializeSpinBox(USpinBox* SpinBox, float ConvertibleSpeed);
	void AdjustSpinBox(USpinBox* SpinBox, float Direction);
	void AdjustSpinBoxWithFeedback(USpinBox* SpinBox, float Direction, bool bPlaySound);
	void RefreshConvertedPointPreview();
	float GetStepSize(const USpinBox* SpinBox) const;
	void StartRepeat(UCommonButtonBase* Button, USpinBox* SpinBox, float Direction);
	void StopRepeat(UCommonButtonBase* Button);
	void HandleRepeatTick();
	void ClearRepeatState();
	void PlayRepeatSound() const;

	UFUNCTION()
	void HandleForwardValueChanged(float InValue);

	UFUNCTION()
	void HandleReverseValueChanged(float InValue);

	UFUNCTION()
	void HandleDecrementForwardClicked();

	UFUNCTION()
	void HandleIncrementForwardClicked();

	UFUNCTION()
	void HandleDecrementReverseClicked();

	UFUNCTION()
	void HandleIncrementReverseClicked();

	UFUNCTION()
	void HandleDecrementForwardPressed();

	UFUNCTION()
	void HandleDecrementForwardReleased();

	UFUNCTION()
	void HandleIncrementForwardPressed();

	UFUNCTION()
	void HandleIncrementForwardReleased();

	UFUNCTION()
	void HandleDecrementReversePressed();

	UFUNCTION()
	void HandleDecrementReverseReleased();

	UFUNCTION()
	void HandleIncrementReversePressed();

	UFUNCTION()
	void HandleIncrementReverseReleased();

	UPROPERTY(EditAnywhere, Category = "Convert")
	float RepeatInterval = 0.1f;

	UPROPERTY()
	USoundBase* RepeatClickSound = nullptr;

	FTimerHandle RepeatTimerHandle;
	TWeakObjectPtr<UCommonButtonBase> ActiveRepeatButton;
	TWeakObjectPtr<USpinBox> ActiveRepeatSpinBox;
	float ActiveRepeatDirection = 0.f;
};
