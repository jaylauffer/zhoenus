// Copyright 2026 Jay Lauffer All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TouchPressureCalibrationWidget.generated.h"

class UButton;
class UCanvasPanel;
class UProgressBar;
class USlider;
class UTextBlock;

UCLASS()
class UTouchPressureCalibrationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTouchPressureCalibrationWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	struct FStickCaptureState
	{
		int32 PointerIndex = INDEX_NONE;
		bool bUsingMouse = false;
		FVector2D LocalPosition = FVector2D::ZeroVector;
		float RawPressure = 0.0f;

		void Reset()
		{
			PointerIndex = INDEX_NONE;
			bUsingMouse = false;
			LocalPosition = FVector2D::ZeroVector;
			RawPressure = 0.0f;
		}

		bool IsCaptured() const
		{
			return bUsingMouse || PointerIndex != INDEX_NONE;
		}
	};

	UFUNCTION()
	void HandleStabilizeDeadzoneChanged(float Value);

	UFUNCTION()
	void HandleStabilizeScaleChanged(float Value);

	UFUNCTION()
	void HandleFireDeadzoneChanged(float Value);

	UFUNCTION()
	void HandleFireScaleChanged(float Value);

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleResetClicked();

	void BuildWidgetTree();
	void LoadSettingsIntoControls();
	void RefreshPreview();
	void RefreshSettingValueLabels();
	void RefreshStatusText();
	void ApplyTouchUpdate(bool bLeftStick, int32 PointerIndex, const FVector2D& LocalPosition, float RawPressure);
	bool ReleaseTouch(int32 PointerIndex);
	bool ReleaseMousePreview();
	bool UpdateTouchCapture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent);
	bool UpdateMousePreview(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	bool ResolveStickAtPosition(const FVector2D& LocalPosition, const FVector2D& LocalSize, bool& bOutLeftStick) const;
	FVector2D GetStickCenter(bool bLeftStick, const FVector2D& LocalSize) const;
	float GetStickRadius(const FVector2D& LocalSize) const;
	float GetNormalizedPreview(bool bFire, float RawPressure) const;
	void SetStatusMessage(const FText& Message);
	static float ClampTouchForce(float RawPressure);

	FStickCaptureState& GetStickState(bool bLeftStick);
	const FStickCaptureState& GetStickState(bool bLeftStick) const;

	UPROPERTY(Transient)
	UCanvasPanel* RootCanvas = nullptr;

	UPROPERTY(Transient)
	UTextBlock* HintText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* StatusText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* StabilizeDeadzoneValueText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* StabilizeScaleValueText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* FireDeadzoneValueText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* FireScaleValueText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* StabilizeRawText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* StabilizeNormalizedText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* FireRawText = nullptr;

	UPROPERTY(Transient)
	UTextBlock* FireNormalizedText = nullptr;

	UPROPERTY(Transient)
	UProgressBar* StabilizeRawBar = nullptr;

	UPROPERTY(Transient)
	UProgressBar* StabilizeNormalizedBar = nullptr;

	UPROPERTY(Transient)
	UProgressBar* FireRawBar = nullptr;

	UPROPERTY(Transient)
	UProgressBar* FireNormalizedBar = nullptr;

	UPROPERTY(Transient)
	USlider* StabilizeDeadzoneSlider = nullptr;

	UPROPERTY(Transient)
	USlider* StabilizeScaleSlider = nullptr;

	UPROPERTY(Transient)
	USlider* FireDeadzoneSlider = nullptr;

	UPROPERTY(Transient)
	USlider* FireScaleSlider = nullptr;

	UPROPERTY(Transient)
	UButton* SaveButton = nullptr;

	UPROPERTY(Transient)
	UButton* ResetButton = nullptr;

	FStickCaptureState LeftStickState;
	FStickCaptureState RightStickState;
	bool bUpdatingControls = false;
	bool bHasSeenTouchInput = false;
	bool bHasSeenPositiveTouchForce = false;
	mutable FVector2D CachedLeftStickCenter = FVector2D::ZeroVector;
	mutable FVector2D CachedRightStickCenter = FVector2D::ZeroVector;
	mutable float CachedStickRadius = 0.0f;
	FText PendingStatusMessage;
};
