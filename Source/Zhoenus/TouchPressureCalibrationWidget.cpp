// Copyright 2026 Jay Lauffer All Rights Reserved.

#include "TouchPressureCalibrationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "ZhoenusTouchPressureSettings.h"

namespace
{
	constexpr float MaxDeadzoneValue = 0.95f;
	constexpr float MaxScaleValue = 2.0f;
	constexpr int32 CircleSegments = 48;
	constexpr float SlowestFireIntervalSeconds = 2.3f;
	constexpr float FastestFireIntervalSeconds = 0.2f;

	FSlateFontInfo MakeFont(int32 Size)
	{
		return FCoreStyle::GetDefaultFontStyle("Regular", Size);
	}

	UTextBlock* MakeText(UWidgetTree* WidgetTree, const FName Name, const FString& Text, int32 FontSize, const FLinearColor& Color)
	{
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetText(FText::FromString(Text));
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetFont(MakeFont(FontSize));
		TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));
		return TextBlock;
	}

	void ConfigureButtonText(UButton* Button, UWidgetTree* WidgetTree, const FName Name, const FString& Label)
	{
		Button->SetContent(MakeText(WidgetTree, Name, Label, 18, FLinearColor(0.92f, 0.98f, 1.0f, 1.0f)));
	}

	void ConfigureCanvasSlot(
		UCanvasPanel* Canvas,
		UWidget* Child,
		const FAnchors& Anchors,
		const FVector2D& Alignment,
		const FVector2D& Position,
		const FVector2D& Size)
	{
		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetAutoSize(false);
		Slot->SetPosition(Position);
		Slot->SetSize(Size);
	}

	void AddCircle(
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FGeometry& Geometry,
		const FVector2D& Center,
		float Radius,
		const FLinearColor& Color,
		float Thickness)
	{
		TArray<FVector2f> Points;
		Points.Reserve(CircleSegments + 1);

		for (int32 Index = 0; Index <= CircleSegments; ++Index)
		{
			const float Angle = (2.0f * PI * Index) / CircleSegments;
			const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
			Points.Add(FVector2f(Point));
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			Color,
			true,
			Thickness);
	}

	void AddPolyline(
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FGeometry& Geometry,
		const TArray<FVector2D>& PointsIn,
		const FLinearColor& Color,
		float Thickness)
	{
		TArray<FVector2f> Points;
		Points.Reserve(PointsIn.Num());
		for (const FVector2D& Point : PointsIn)
		{
			Points.Add(FVector2f(Point));
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(),
			Points,
			ESlateDrawEffect::None,
			Color,
			true,
			Thickness);
	}
}

UTouchPressureCalibrationWidget::UTouchPressureCalibrationWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTouchPressureCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (!WidgetTree->RootWidget)
	{
		BuildWidgetTree();
	}

	LoadSettingsIntoControls();
	RefreshPreview();
}

int32 UTouchPressureCalibrationWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	CachedStickRadius = GetStickRadius(LocalSize);
	CachedLeftStickCenter = GetStickCenter(true, LocalSize);
	CachedRightStickCenter = GetStickCenter(false, LocalSize);

	const FSlateRoundedBoxBrush BackgroundBrush(FLinearColor(0.01f, 0.02f, 0.05f, 1.0f));
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		&BackgroundBrush,
		ESlateDrawEffect::None,
		FLinearColor(0.01f, 0.02f, 0.05f, 1.0f));

	const FVector2D BodySize(LocalSize.X * 0.54f, LocalSize.Y * 0.24f);
	const FVector2D BodyPos((LocalSize.X - BodySize.X) * 0.5f, LocalSize.Y * 0.51f);
	const FSlateRoundedBoxBrush BodyBrush(
		FLinearColor(0.05f, 0.11f, 0.16f, 0.92f),
		28.0f,
		FLinearColor(0.19f, 0.75f, 0.89f, 0.42f),
		2.0f);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(BodySize, FSlateLayoutTransform(BodyPos)),
		&BodyBrush);

	const FVector2D ShoulderSize(LocalSize.X * 0.16f, LocalSize.Y * 0.055f);
	const float ShoulderY = BodyPos.Y - ShoulderSize.Y * 0.8f;
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 2,
		AllottedGeometry.ToPaintGeometry(ShoulderSize, FSlateLayoutTransform(FVector2D(BodyPos.X + 18.0f, ShoulderY))),
		&BodyBrush);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 2,
		AllottedGeometry.ToPaintGeometry(
			ShoulderSize,
			FSlateLayoutTransform(FVector2D(BodyPos.X + BodySize.X - ShoulderSize.X - 18.0f, ShoulderY))),
		&BodyBrush);

	AddCircle(
		OutDrawElements,
		LayerId + 2,
		AllottedGeometry,
		CachedLeftStickCenter,
		CachedStickRadius * 1.15f,
		FLinearColor(0.05f, 0.26f, 0.33f, 0.48f),
		30.0f);
	AddCircle(
		OutDrawElements,
		LayerId + 2,
		AllottedGeometry,
		CachedRightStickCenter,
		CachedStickRadius * 1.15f,
		FLinearColor(0.05f, 0.26f, 0.33f, 0.48f),
		30.0f);

	AddPolyline(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		{ FVector2D(LocalSize.X * 0.17f, LocalSize.Y * 0.54f), FVector2D(LocalSize.X * 0.25f, LocalSize.Y * 0.54f) },
		FLinearColor(0.51f, 0.96f, 0.65f, 0.9f),
		6.0f);
	AddPolyline(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		{ FVector2D(LocalSize.X * 0.21f, LocalSize.Y * 0.50f), FVector2D(LocalSize.X * 0.21f, LocalSize.Y * 0.58f) },
		FLinearColor(0.51f, 0.96f, 0.65f, 0.9f),
		6.0f);

	const FVector2D FaceCenter(LocalSize.X * 0.79f, LocalSize.Y * 0.53f);
	const float FaceButtonRadius = CachedStickRadius * 0.17f;
	AddCircle(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		FaceCenter + FVector2D(0.0f, -FaceButtonRadius * 1.8f),
		FaceButtonRadius,
		FLinearColor(1.0f, 0.74f, 0.25f, 0.95f),
		7.0f);
	AddCircle(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		FaceCenter + FVector2D(FaceButtonRadius * 1.8f, 0.0f),
		FaceButtonRadius,
		FLinearColor(1.0f, 0.41f, 0.34f, 0.95f),
		7.0f);
	AddCircle(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		FaceCenter + FVector2D(0.0f, FaceButtonRadius * 1.8f),
		FaceButtonRadius,
		FLinearColor(0.45f, 0.87f, 1.0f, 0.95f),
		7.0f);
	AddCircle(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry,
		FaceCenter + FVector2D(-FaceButtonRadius * 1.8f, 0.0f),
		FaceButtonRadius,
		FLinearColor(0.53f, 1.0f, 0.51f, 0.95f),
		7.0f);

	const auto DrawStick = [this, &OutDrawElements, &AllottedGeometry, LayerId](
		const FVector2D& Center,
		const FStickCaptureState& State,
		const FLinearColor& RingColor,
		const FLinearColor& CoreColor)
	{
		AddCircle(OutDrawElements, LayerId + 4, AllottedGeometry, Center, CachedStickRadius, RingColor, 4.0f);
		AddCircle(OutDrawElements, LayerId + 4, AllottedGeometry, Center, CachedStickRadius * 0.62f, RingColor.CopyWithNewOpacity(0.45f), 2.0f);

		const FVector2D ThumbOffset = State.IsCaptured()
			? (State.LocalPosition - Center).GetClampedToMaxSize(CachedStickRadius * 0.42f)
			: FVector2D::ZeroVector;
		AddCircle(
			OutDrawElements,
			LayerId + 5,
			AllottedGeometry,
			Center + ThumbOffset,
			CachedStickRadius * 0.22f,
			CoreColor.CopyWithNewOpacity(0.95f),
			12.0f);
	};

	DrawStick(
		CachedLeftStickCenter,
		LeftStickState,
		FLinearColor(0.36f, 0.88f, 1.0f, 0.95f),
		FLinearColor(0.18f, 0.44f, 0.55f, 0.95f));
	DrawStick(
		CachedRightStickCenter,
		RightStickState,
		FLinearColor(1.0f, 0.58f, 0.25f, 0.95f),
		FLinearColor(0.58f, 0.24f, 0.12f, 0.95f));

	return Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId + 6,
		InWidgetStyle,
		bParentEnabled);
}

FReply UTouchPressureCalibrationWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return UpdateTouchCapture(InGeometry, InGestureEvent)
		? FReply::Handled()
		: Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return UpdateTouchCapture(InGeometry, InGestureEvent)
		? FReply::Handled()
		: Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnTouchForceChanged(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return UpdateTouchCapture(InGeometry, InGestureEvent)
		? FReply::Handled()
		: Super::NativeOnTouchForceChanged(InGeometry, InGestureEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	return ReleaseTouch(InGestureEvent.GetPointerIndex())
		? FReply::Handled()
		: Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UpdateMousePreview(InGeometry, InMouseEvent)
		? FReply::Handled()
		: Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return UpdateMousePreview(InGeometry, InMouseEvent)
		? FReply::Handled()
		: Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UTouchPressureCalibrationWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return ReleaseMousePreview()
			? FReply::Handled()
			: Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UTouchPressureCalibrationWidget::HandleStabilizeDeadzoneChanged(const float Value)
{
	if (bUpdatingControls)
	{
		return;
	}

	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->StabilizePressureDeadzone = Value;
	Settings->ClampValues();
	RefreshPreview();
	SetStatusMessage(FText::FromString(TEXT("Stabilize deadzone updated. Press Save to persist.")));
}

void UTouchPressureCalibrationWidget::HandleStabilizeScaleChanged(const float Value)
{
	if (bUpdatingControls)
	{
		return;
	}

	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->StabilizePressureScale = Value;
	Settings->ClampValues();
	RefreshPreview();
	SetStatusMessage(FText::FromString(TEXT("Stabilize scale updated. Press Save to persist.")));
}

void UTouchPressureCalibrationWidget::HandleFireDeadzoneChanged(const float Value)
{
	if (bUpdatingControls)
	{
		return;
	}

	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->FirePressureDeadzone = Value;
	Settings->ClampValues();
	RefreshPreview();
	SetStatusMessage(FText::FromString(TEXT("Fire deadzone updated. Press Save to persist.")));
}

void UTouchPressureCalibrationWidget::HandleFireScaleChanged(const float Value)
{
	if (bUpdatingControls)
	{
		return;
	}

	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->FirePressureScale = Value;
	Settings->ClampValues();
	RefreshPreview();
	SetStatusMessage(FText::FromString(TEXT("Fire scale updated. Press Save to persist.")));
}

void UTouchPressureCalibrationWidget::HandleSaveClicked()
{
	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->ClampValues();
	Settings->SaveConfig();

	SetStatusMessage(FText::FromString(FString::Printf(
		TEXT("Saved touch pressure calibration at %s."),
		*FDateTime::Now().ToString(TEXT("%H:%M:%S")))));
	RefreshPreview();
}

void UTouchPressureCalibrationWidget::HandleResetClicked()
{
	UZhoenusTouchPressureSettings* Settings = GetMutableDefault<UZhoenusTouchPressureSettings>();
	Settings->RestoreFactoryDefaults();
	Settings->ClampValues();
	LoadSettingsIntoControls();
	SetStatusMessage(FText::FromString(TEXT("Reset to factory defaults. Press Save to persist.")));
}

void UTouchPressureCalibrationWidget::BuildWidgetTree()
{
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TouchCalibrationRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* HeaderPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HeaderPanel"));
	HeaderPanel->SetBrushColor(FLinearColor(0.03f, 0.08f, 0.14f, 0.88f));
	HeaderPanel->SetPadding(FMargin(18.0f, 16.0f));
	UVerticalBox* HeaderBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HeaderBox"));
	HeaderPanel->SetContent(HeaderBox);
	HeaderBox->AddChildToVerticalBox(MakeText(WidgetTree, TEXT("TitleText"), TEXT("Touch Pressure Lab"), 28, FLinearColor(0.98f, 0.98f, 0.90f, 1.0f)));
	HintText = MakeText(
		WidgetTree,
		TEXT("HintText"),
		TEXT("Press on the left and right stick pads to watch raw touch force and normalized output in real time."),
		14,
		FLinearColor(0.76f, 0.90f, 1.0f, 1.0f));
	HeaderBox->AddChildToVerticalBox(HintText);
	StatusText = MakeText(
		WidgetTree,
		TEXT("StatusText"),
		TEXT("Adjust sliders to tune deadzone and scale, then press Save."),
		13,
		FLinearColor(0.90f, 0.82f, 0.38f, 1.0f));
	HeaderBox->AddChildToVerticalBox(StatusText);
	ConfigureCanvasSlot(
		RootCanvas,
		HeaderPanel,
		FAnchors(0.5f, 0.0f),
		FVector2D(0.5f, 0.0f),
		FVector2D(0.0f, 22.0f),
		FVector2D(620.0f, 116.0f));

	const auto CreateSliderCard = [this](
		const FName PanelName,
		const FString& Title,
		const FLinearColor& AccentColor,
		USlider*& OutDeadzoneSlider,
		UTextBlock*& OutDeadzoneValueText,
		USlider*& OutScaleSlider,
		UTextBlock*& OutScaleValueText) -> UBorder*
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), PanelName);
		Panel->SetBrushColor(FLinearColor(0.04f, 0.08f, 0.12f, 0.90f));
		Panel->SetPadding(FMargin(18.0f, 18.0f, 18.0f, 12.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), FName(*FString::Printf(TEXT("%sBox"), *PanelName.ToString())));
		Panel->SetContent(Box);

		UTextBlock* TitleText = MakeText(WidgetTree, FName(*FString::Printf(TEXT("%sTitle"), *PanelName.ToString())), Title, 21, AccentColor);
		Box->AddChildToVerticalBox(TitleText);

		const auto AddSliderRow = [this, Box, PanelName](const FName Name, const FString& Label, const FLinearColor& ValueColor, USlider*& OutSlider, UTextBlock*& OutValueText, float Min, float Max)
		{
			const FString WidgetPrefix = FString::Printf(TEXT("%s_%s"), *PanelName.ToString(), *Name.ToString());
			UTextBlock* LabelText = MakeText(WidgetTree, FName(*FString::Printf(TEXT("%sLabel"), *WidgetPrefix)), Label, 15, FLinearColor(0.86f, 0.94f, 1.0f, 1.0f));
			UVerticalBoxSlot* LabelSlot = Box->AddChildToVerticalBox(LabelText);
			LabelSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 4.0f));

			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(*FString::Printf(TEXT("%sRow"), *WidgetPrefix)));
			UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Row);
			RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));

			OutSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), FName(*WidgetPrefix));
			OutSlider->SetMinValue(Min);
			OutSlider->SetMaxValue(Max);
			OutSlider->SetStepSize(0.01f);
			OutSlider->SetSliderBarColor(FLinearColor(0.10f, 0.21f, 0.28f, 1.0f));
			OutSlider->SetSliderHandleColor(ValueColor);
			UHorizontalBoxSlot* SliderSlot = Row->AddChildToHorizontalBox(OutSlider);
			SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			SliderSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));

			OutValueText = MakeText(
				WidgetTree,
				FName(*FString::Printf(TEXT("%sValue"), *WidgetPrefix)),
				TEXT("0.00"),
				15,
				ValueColor);
			UHorizontalBoxSlot* ValueSlot = Row->AddChildToHorizontalBox(OutValueText);
			ValueSlot->SetHorizontalAlignment(HAlign_Right);
			ValueSlot->SetVerticalAlignment(VAlign_Center);
		};

		AddSliderRow(TEXT("DeadzoneSlider"), TEXT("Deadzone"), AccentColor, OutDeadzoneSlider, OutDeadzoneValueText, 0.0f, MaxDeadzoneValue);
		AddSliderRow(TEXT("ScaleSlider"), TEXT("Scale"), AccentColor.CopyWithNewOpacity(0.92f), OutScaleSlider, OutScaleValueText, 0.0f, MaxScaleValue);

		return Panel;
	};

	UBorder* StabilizePanel = CreateSliderCard(
		TEXT("StabilizePanel"),
		TEXT("Left Stick: Stabilize"),
		FLinearColor(0.42f, 0.90f, 1.0f, 1.0f),
		StabilizeDeadzoneSlider,
		StabilizeDeadzoneValueText,
		StabilizeScaleSlider,
		StabilizeScaleValueText);
	ConfigureCanvasSlot(
		RootCanvas,
		StabilizePanel,
		FAnchors(0.0f, 0.0f),
		FVector2D(0.0f, 0.0f),
		FVector2D(28.0f, 152.0f),
		FVector2D(330.0f, 212.0f));

	UBorder* FirePanel = CreateSliderCard(
		TEXT("FirePanel"),
		TEXT("Right Stick: Fire"),
		FLinearColor(1.0f, 0.60f, 0.32f, 1.0f),
		FireDeadzoneSlider,
		FireDeadzoneValueText,
		FireScaleSlider,
		FireScaleValueText);
	ConfigureCanvasSlot(
		RootCanvas,
		FirePanel,
		FAnchors(1.0f, 0.0f),
		FVector2D(1.0f, 0.0f),
		FVector2D(-28.0f, 152.0f),
		FVector2D(330.0f, 212.0f));

	const auto CreateReadoutPanel = [this](
		const FName PanelName,
		const FString& Title,
		const FLinearColor& AccentColor,
		UTextBlock*& OutRawText,
		UProgressBar*& OutRawBar,
		UTextBlock*& OutNormalizedText,
		UProgressBar*& OutNormalizedBar) -> UBorder*
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), PanelName);
		Panel->SetBrushColor(FLinearColor(0.04f, 0.06f, 0.09f, 0.84f));
		Panel->SetPadding(FMargin(14.0f));

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), FName(*FString::Printf(TEXT("%sBox"), *PanelName.ToString())));
		Panel->SetContent(Box);
		Box->AddChildToVerticalBox(MakeText(WidgetTree, FName(*FString::Printf(TEXT("%sTitle"), *PanelName.ToString())), Title, 19, AccentColor));

		const auto AddProgressRow = [this, Box, PanelName](const FName Name, const FString& Label, const FLinearColor& FillColor, UTextBlock*& OutText, UProgressBar*& OutBar)
		{
			const FString WidgetPrefix = FString::Printf(TEXT("%s_%s"), *PanelName.ToString(), *Name.ToString());
			UTextBlock* LabelText = MakeText(WidgetTree, FName(*FString::Printf(TEXT("%sLabel"), *WidgetPrefix)), Label, 14, FLinearColor(0.84f, 0.90f, 0.97f, 1.0f));
			UVerticalBoxSlot* LabelSlot = Box->AddChildToVerticalBox(LabelText);
			LabelSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 2.0f));

			OutBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), FName(*WidgetPrefix));
			OutBar->SetFillColorAndOpacity(FillColor);
			OutBar->SetPercent(0.0f);
			UVerticalBoxSlot* BarSlot = Box->AddChildToVerticalBox(OutBar);
			BarSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));

			OutText = MakeText(
				WidgetTree,
				FName(*FString::Printf(TEXT("%sText"), *WidgetPrefix)),
				TEXT("0.000"),
				14,
				FillColor);
			Box->AddChildToVerticalBox(OutText);
		};

		AddProgressRow(TEXT("RawBar"), TEXT("Raw Touch Force"), AccentColor, OutRawText, OutRawBar);
		AddProgressRow(TEXT("NormalizedBar"), TEXT("Output After Deadzone + Scale"), AccentColor.CopyWithNewOpacity(0.92f), OutNormalizedText, OutNormalizedBar);
		return Panel;
	};

	UBorder* StabilizeReadout = CreateReadoutPanel(
		TEXT("StabilizeReadout"),
		TEXT("Stabilize Preview"),
		FLinearColor(0.38f, 0.86f, 1.0f, 1.0f),
		StabilizeRawText,
		StabilizeRawBar,
		StabilizeNormalizedText,
		StabilizeNormalizedBar);
	ConfigureCanvasSlot(
		RootCanvas,
		StabilizeReadout,
		FAnchors(0.0f, 1.0f),
		FVector2D(0.0f, 1.0f),
		FVector2D(34.0f, -252.0f),
		FVector2D(340.0f, 164.0f));

	UBorder* FireReadout = CreateReadoutPanel(
		TEXT("FireReadout"),
		TEXT("Fire Preview"),
		FLinearColor(1.0f, 0.58f, 0.28f, 1.0f),
		FireRawText,
		FireRawBar,
		FireNormalizedText,
		FireNormalizedBar);
	ConfigureCanvasSlot(
		RootCanvas,
		FireReadout,
		FAnchors(1.0f, 1.0f),
		FVector2D(1.0f, 1.0f),
		FVector2D(-34.0f, -252.0f),
		FVector2D(340.0f, 164.0f));

	UBorder* ButtonPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ButtonPanel"));
	ButtonPanel->SetBrushColor(FLinearColor(0.03f, 0.08f, 0.13f, 0.90f));
	ButtonPanel->SetPadding(FMargin(14.0f));
	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonRow"));
	ButtonPanel->SetContent(ButtonRow);

	SaveButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SaveButton"));
	SaveButton->SetBackgroundColor(FLinearColor(0.12f, 0.46f, 0.29f, 1.0f));
	SaveButton->OnClicked.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleSaveClicked);
	ConfigureButtonText(SaveButton, WidgetTree, TEXT("SaveButtonText"), TEXT("Save Calibration"));
	UHorizontalBoxSlot* SaveSlot = ButtonRow->AddChildToHorizontalBox(SaveButton);
	SaveSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
	SaveSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButton"));
	ResetButton->SetBackgroundColor(FLinearColor(0.35f, 0.14f, 0.14f, 1.0f));
	ResetButton->OnClicked.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleResetClicked);
	ConfigureButtonText(ResetButton, WidgetTree, TEXT("ResetButtonText"), TEXT("Reset Defaults"));
	UHorizontalBoxSlot* ResetSlot = ButtonRow->AddChildToHorizontalBox(ResetButton);
	ResetSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	ConfigureCanvasSlot(
		RootCanvas,
		ButtonPanel,
		FAnchors(0.5f, 1.0f),
		FVector2D(0.5f, 1.0f),
		FVector2D(0.0f, -28.0f),
		FVector2D(360.0f, 72.0f));

	StabilizeDeadzoneSlider->OnValueChanged.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleStabilizeDeadzoneChanged);
	StabilizeScaleSlider->OnValueChanged.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleStabilizeScaleChanged);
	FireDeadzoneSlider->OnValueChanged.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleFireDeadzoneChanged);
	FireScaleSlider->OnValueChanged.AddDynamic(this, &UTouchPressureCalibrationWidget::HandleFireScaleChanged);
}

void UTouchPressureCalibrationWidget::LoadSettingsIntoControls()
{
	bUpdatingControls = true;

	const UZhoenusTouchPressureSettings* Settings = GetDefault<UZhoenusTouchPressureSettings>();
	if (StabilizeDeadzoneSlider)
	{
		StabilizeDeadzoneSlider->SetValue(Settings->StabilizePressureDeadzone);
	}
	if (StabilizeScaleSlider)
	{
		StabilizeScaleSlider->SetValue(Settings->StabilizePressureScale);
	}
	if (FireDeadzoneSlider)
	{
		FireDeadzoneSlider->SetValue(Settings->FirePressureDeadzone);
	}
	if (FireScaleSlider)
	{
		FireScaleSlider->SetValue(Settings->FirePressureScale);
	}

	bUpdatingControls = false;
	RefreshPreview();
}

void UTouchPressureCalibrationWidget::RefreshPreview()
{
	RefreshSettingValueLabels();

	const float StabilizeRaw = FMath::Max(0.0f, LeftStickState.RawPressure);
	const float FireRaw = FMath::Max(0.0f, RightStickState.RawPressure);
	const float StabilizeEffective = LeftStickState.EffectivePressure;
	const float FireEffective = RightStickState.EffectivePressure;
	const float StabilizeNormalized = GetNormalizedPreview(false, StabilizeEffective);
	const float FireNormalized = GetNormalizedPreview(true, FireEffective);

	if (StabilizeRawText)
	{
		StabilizeRawText->SetText(FText::FromString(FString::Printf(
			TEXT("raw %.3f  travel %.3f  effective %.3f"),
			StabilizeRaw,
			LeftStickState.TravelPressure,
			StabilizeEffective)));
	}
	if (StabilizeNormalizedText)
	{
		StabilizeNormalizedText->SetText(FText::FromString(FString::Printf(TEXT("%.3f"), StabilizeNormalized)));
	}
	if (FireRawText)
	{
		FireRawText->SetText(FText::FromString(FString::Printf(
			TEXT("raw %.3f  travel %.3f  effective %.3f"),
			FireRaw,
			RightStickState.TravelPressure,
			FireEffective)));
	}
	if (FireNormalizedText)
	{
		const FString FireText = FireNormalized > 0.0f
			? FString::Printf(TEXT("%.3f  %.2fs/shot"), FireNormalized, GetFireIntervalSeconds(FireNormalized))
			: FString::Printf(TEXT("%.3f  no fire"), FireNormalized);
		FireNormalizedText->SetText(FText::FromString(FireText));
	}
	if (StabilizeRawBar)
	{
		StabilizeRawBar->SetPercent(GetRawPressureBarPercent(StabilizeRaw));
	}
	if (StabilizeNormalizedBar)
	{
		StabilizeNormalizedBar->SetPercent(StabilizeNormalized);
	}
	if (FireRawBar)
	{
		FireRawBar->SetPercent(GetRawPressureBarPercent(FireRaw));
	}
	if (FireNormalizedBar)
	{
		FireNormalizedBar->SetPercent(FireNormalized);
	}

	RefreshStatusText();
}

void UTouchPressureCalibrationWidget::RefreshSettingValueLabels()
{
	const UZhoenusTouchPressureSettings* Settings = GetDefault<UZhoenusTouchPressureSettings>();
	if (StabilizeDeadzoneValueText)
	{
		StabilizeDeadzoneValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Settings->StabilizePressureDeadzone)));
	}
	if (StabilizeScaleValueText)
	{
		StabilizeScaleValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Settings->StabilizePressureScale)));
	}
	if (FireDeadzoneValueText)
	{
		FireDeadzoneValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Settings->FirePressureDeadzone)));
	}
	if (FireScaleValueText)
	{
		FireScaleValueText->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Settings->FirePressureScale)));
	}
}

void UTouchPressureCalibrationWidget::RefreshStatusText()
{
	if (!HintText)
	{
		return;
	}

	if (LeftStickState.bUsingMouse || RightStickState.bUsingMouse)
	{
		HintText->SetText(FText::FromString(TEXT("Mouse preview uses raw 1.0, matching ordinary touch behavior; zero force uses travel fallback.")));
		return;
	}

	const bool bHasSeenRealTouchForce = bLeftStickHasSeenRealTouchForce || bRightStickHasSeenRealTouchForce;
	if (bHasSeenTouchInput && !bHasSeenRealTouchForce && (LeftStickState.IsCaptured() || RightStickState.IsCaptured()))
	{
		const bool bUsingTravelFallback =
			(LeftStickState.IsCaptured() && LeftStickState.RawPressure <= SMALL_NUMBER)
			|| (RightStickState.IsCaptured() && RightStickState.RawPressure <= SMALL_NUMBER);
		HintText->SetText(bUsingTravelFallback
			? FText::FromString(TEXT("This device is reporting zero touch force, so preview and live controls are using thumb travel fallback."))
			: FText::FromString(TEXT("This device is reporting default touch force, so preview and live controls treat it as a full press.")));
		return;
	}

	HintText->SetText(FText::FromString(TEXT("Left stick previews Stabilize. Right stick previews Fire. Raw, travel, effective, and tuned output now mirror live gameplay.")));
}

void UTouchPressureCalibrationWidget::ApplyTouchUpdate(
	const bool bLeftStick,
	const int32 PointerIndex,
	const FVector2D& LocalPosition,
	const FVector2D& LocalSize,
	float RawPressure)
{
	FStickCaptureState& StickState = GetStickState(bLeftStick);
	StickState.PointerIndex = PointerIndex;
	StickState.bUsingMouse = false;
	StickState.LocalPosition = LocalPosition;
	StickState.RawPressure = FMath::Max(0.0f, RawPressure);
	StickState.TravelPressure = GetTravelPressureAtPosition(bLeftStick, LocalPosition, LocalSize);

	bHasSeenTouchInput = true;
	bool& bHasSeenRealTouchForce = bLeftStick ? bLeftStickHasSeenRealTouchForce : bRightStickHasSeenRealTouchForce;
	if (StickState.RawPressure > SMALL_NUMBER
		&& !UZhoenusTouchPressureSettings::IsDefaultTouchForce(StickState.RawPressure))
	{
		bHasSeenRealTouchForce = true;
	}
	StickState.EffectivePressure = UZhoenusTouchPressureSettings::ResolveEffectivePressure(
		StickState.RawPressure,
		StickState.TravelPressure,
		bHasSeenRealTouchForce);

	RefreshPreview();
}

bool UTouchPressureCalibrationWidget::ReleaseTouch(const int32 PointerIndex)
{
	bool bChanged = false;
	if (LeftStickState.PointerIndex == PointerIndex)
	{
		LeftStickState.Reset();
		bChanged = true;
	}
	if (RightStickState.PointerIndex == PointerIndex)
	{
		RightStickState.Reset();
		bChanged = true;
	}

	if (bChanged)
	{
		RefreshPreview();
	}

	return bChanged;
}

bool UTouchPressureCalibrationWidget::ReleaseMousePreview()
{
	if (LeftStickState.bUsingMouse || RightStickState.bUsingMouse)
	{
		LeftStickState.Reset();
		RightStickState.Reset();
		RefreshPreview();
		return true;
	}

	return false;
}

bool UTouchPressureCalibrationWidget::UpdateTouchCapture(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InGestureEvent.GetScreenSpacePosition());
	const FVector2D LocalSize = InGeometry.GetLocalSize();

	if (LeftStickState.PointerIndex == InGestureEvent.GetPointerIndex())
	{
		ApplyTouchUpdate(true, InGestureEvent.GetPointerIndex(), LocalPosition, LocalSize, InGestureEvent.GetTouchForce());
		return true;
	}
	if (RightStickState.PointerIndex == InGestureEvent.GetPointerIndex())
	{
		ApplyTouchUpdate(false, InGestureEvent.GetPointerIndex(), LocalPosition, LocalSize, InGestureEvent.GetTouchForce());
		return true;
	}

	bool bLeftStick = false;
	if (!ResolveStickAtPosition(LocalPosition, LocalSize, bLeftStick))
	{
		return false;
	}

	FStickCaptureState& StickState = GetStickState(bLeftStick);
	if (StickState.IsCaptured())
	{
		return false;
	}

	ApplyTouchUpdate(bLeftStick, InGestureEvent.GetPointerIndex(), LocalPosition, LocalSize, InGestureEvent.GetTouchForce());
	return true;
}

bool UTouchPressureCalibrationWidget::UpdateMousePreview(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return false;
	}

	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D LocalSize = InGeometry.GetLocalSize();
	bool bLeftStick = false;
	if (!ResolveStickAtPosition(LocalPosition, LocalSize, bLeftStick))
	{
		return false;
	}

	FStickCaptureState& ActiveStick = GetStickState(bLeftStick);
	FStickCaptureState& InactiveStick = GetStickState(!bLeftStick);
	InactiveStick.Reset();
	ActiveStick.PointerIndex = INDEX_NONE;
	ActiveStick.bUsingMouse = true;
	ActiveStick.LocalPosition = LocalPosition;
	ActiveStick.RawPressure = UZhoenusTouchPressureSettings::DefaultTouchForce;
	ActiveStick.TravelPressure = GetTravelPressureAtPosition(bLeftStick, LocalPosition, LocalSize);
	ActiveStick.EffectivePressure = UZhoenusTouchPressureSettings::ResolveEffectivePressure(
		ActiveStick.RawPressure,
		ActiveStick.TravelPressure,
		false);
	RefreshPreview();
	return true;
}

bool UTouchPressureCalibrationWidget::ResolveStickAtPosition(
	const FVector2D& LocalPosition,
	const FVector2D& LocalSize,
	bool& bOutLeftStick) const
{
	const float StickRadius = GetStickRadius(LocalSize);
	const FVector2D LeftCenter = GetStickCenter(true, LocalSize);
	const FVector2D RightCenter = GetStickCenter(false, LocalSize);

	const bool bInsideLeft = FVector2D::Distance(LocalPosition, LeftCenter) <= StickRadius;
	const bool bInsideRight = FVector2D::Distance(LocalPosition, RightCenter) <= StickRadius;

	if (bInsideLeft)
	{
		bOutLeftStick = true;
		return true;
	}
	if (bInsideRight)
	{
		bOutLeftStick = false;
		return true;
	}

	return false;
}

FVector2D UTouchPressureCalibrationWidget::GetStickCenter(const bool bLeftStick, const FVector2D& LocalSize) const
{
	return FVector2D(
		LocalSize.X * (bLeftStick ? 0.31f : 0.69f),
		LocalSize.Y * 0.74f);
}

float UTouchPressureCalibrationWidget::GetStickRadius(const FVector2D& LocalSize) const
{
	return FMath::Clamp(FMath::Min(LocalSize.X, LocalSize.Y) * 0.115f, 82.0f, 150.0f);
}

float UTouchPressureCalibrationWidget::GetTravelPressureAtPosition(
	const bool bLeftStick,
	const FVector2D& LocalPosition,
	const FVector2D& LocalSize) const
{
	const float StickRadius = GetStickRadius(LocalSize);
	if (StickRadius <= SMALL_NUMBER)
	{
		return 0.0f;
	}

	const FVector2D StickCenter = GetStickCenter(bLeftStick, LocalSize);
	return FMath::Clamp(FVector2D::Distance(LocalPosition, StickCenter) / StickRadius, 0.0f, 1.0f);
}

float UTouchPressureCalibrationWidget::GetNormalizedPreview(const bool bFire, const float EffectivePressure) const
{
	return GetDefault<UZhoenusTouchPressureSettings>()->NormalizePressure(bFire, EffectivePressure);
}

void UTouchPressureCalibrationWidget::SetStatusMessage(const FText& Message)
{
	PendingStatusMessage = Message;
	if (StatusText)
	{
		StatusText->SetText(Message);
	}
}

float UTouchPressureCalibrationWidget::GetRawPressureBarPercent(const float RawPressure)
{
	return UZhoenusTouchPressureSettings::NormalizeRawTouchForce(RawPressure);
}

float UTouchPressureCalibrationWidget::GetFireIntervalSeconds(const float FireOutput)
{
	return FMath::GetRangeValue(
		FVector2D(SlowestFireIntervalSeconds, FastestFireIntervalSeconds),
		FMath::Clamp(FireOutput, 0.0f, 1.0f));
}

UTouchPressureCalibrationWidget::FStickCaptureState& UTouchPressureCalibrationWidget::GetStickState(const bool bLeftStick)
{
	return bLeftStick ? LeftStickState : RightStickState;
}

const UTouchPressureCalibrationWidget::FStickCaptureState& UTouchPressureCalibrationWidget::GetStickState(const bool bLeftStick) const
{
	return bLeftStick ? LeftStickState : RightStickState;
}
