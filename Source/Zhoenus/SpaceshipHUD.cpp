#include "SpaceshipHUD.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "SpaceshipPawn.h"
#include "ZhoenusBatteryComponent.h"
#include "GameFramework/PlayerController.h"
#include "GlobalRenderResources.h"

#define ON_SCREEN_DEBUG 1
#ifdef ON_SCREEN_DEBUG
#include <Runtime/Engine/Classes/Engine/Engine.h>
#define ScreenDebug3(text) if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Yellow, text)
#else
#define ScreenDebug3(text) 
#endif

namespace
		{
			const FLinearColor RollColor(.3f, 0.f, 1.f, 0.32f);
			const FLinearColor StabilizeColor(.7f, 0, .5f);
			const float ReticleScreenMargin = 32.f;
		}

ASpaceshipHUD::ASpaceshipHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> Widget(TEXT("/Game/Blueprints/HUD"));
	HUDWidgetClass = Widget.Class;
}


void ASpaceshipHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass != nullptr)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void ASpaceshipHUD::DrawHUD()
{
	Super::DrawHUD();
	
	ASpaceshipPawn* pawn{ Cast<ASpaceshipPawn>(GetOwningPawn()) };
	if (pawn)
	{
		DrawAimReticle(*pawn);
		DrawBatteryIndicator(*pawn);
		DrawPitchIndicator(*pawn);
		DrawYawIndicator(*pawn);

			//v is thrust
			FVector2D v{ Canvas->ClipX * .98f, Canvas->ClipY * .7f };

		FVector2D rl{ Canvas->ClipX * .04f, Canvas->ClipY * .3f };
		FVector2D rr{ Canvas->ClipX * .96f, Canvas->ClipY * .3f };

		FVector2D s{ Canvas->ClipX * .97f, Canvas->ClipY * .7f };

		float& Speed{ pawn->CurrentForwardSpeed };
		if (Speed > 4.6f)
		{
			FVector2D e{ 10.f, -Speed / pawn->MaxSpeed * Canvas->ClipY * .6f };
			DrawRect(FLinearColor::Red, v.X, v.Y, e.X, e.Y);
		}
		else	if (Speed < -4.6f)
		{
			FVector2D e{ 10.f, Speed / pawn->MinSpeed * Canvas->ClipY * .2f };
			DrawRect(FLinearColor::Yellow, v.X, v.Y, e.X, e.Y);
		}

		v.X -= 10.f;
		double Accel{ -pawn->CachedInput.W };
		if (Accel > 0.02f)
		{
			FVector2D e{ 10.f, -Accel * Canvas->ClipY * .6f };
			DrawRect(FLinearColor::Blue, v.X, v.Y, e.X, e.Y);
		}
		else if (Accel < -0.02f)
		{
			FVector2D e{ 10.f, -Accel * Canvas->ClipY * .2f };
			DrawRect(FLinearColor::Green, v.X, v.Y, e.X, e.Y);
		}

		//work through these.. roll
		//split the roll to two bars (left one and right one)
		//rl.X -= 18.f;
		double Roll{ -pawn->CachedInput.Z};
		if (Roll > 0.02f)
		{
			FVector2D e{ 20.f, Roll * Canvas->ClipY * .6f };
			DrawRect(RollColor, rl.X, rl.Y, e.X, e.Y);
		}
		else if (Roll < -0.02f)
		{
			FVector2D e{ 20.f, -Roll * Canvas->ClipY * .6f };
			DrawRect(RollColor, rr.X, rr.Y, e.X, e.Y);
		}

		//work through these.. stabilize
		s.X -= 23.f;
		double Stabilize{ -pawn->StabilityInput.X };
		if (Stabilize > 0.02f)
		{
			FVector2D e{ 10.f, -Stabilize * Canvas->ClipY * .6f };
			DrawRect(StabilizeColor, s.X, s.Y, e.X, e.Y);
		}
		else if (Stabilize < -0.02f)
		{
			FVector2D e{ 10.f, -Stabilize * Canvas->ClipY * .2f };
			DrawRect(StabilizeColor, s.X, s.Y, e.X, e.Y);
		}
	}
	

}

void ASpaceshipHUD::DrawBatteryIndicator(const ASpaceshipPawn& Pawn)
{
	if (!bDrawBatteryIndicator || Canvas == nullptr)
	{
		return;
	}

	const UZhoenusBatteryComponent* const Battery = Pawn.GetBatteryComponent();
	if (Battery == nullptr || Battery->GetMaxEnergy() <= 0.f)
	{
		return;
	}

	const float EnergyFraction = Battery->GetEnergyFraction();
	const float MeterWidth = BatteryIndicatorWidth;
	const float MeterHeight = BatteryIndicatorHeight;
	const float MeterLeft = Canvas->ClipX * BatteryIndicatorHorizontalAnchor - MeterWidth * 0.5f;
	const float MeterTop = Canvas->ClipY * BatteryIndicatorVerticalAnchor - MeterHeight * 0.5f;
	const float Padding = FMath::Clamp(BatteryIndicatorPadding, 0.f, MeterWidth * 0.4f);
	const float InnerWidth = FMath::Max(1.f, MeterWidth - Padding * 2.f);
	const float InnerHeight = FMath::Max(1.f, MeterHeight - Padding * 2.f);
	const float FillHeight = InnerHeight * EnergyFraction;
	const FLinearColor FillColor = EnergyFraction <= BatteryIndicatorLowThreshold ? BatteryIndicatorLowColor : BatteryIndicatorColor;

	DrawRect(BatteryIndicatorBackgroundColor, MeterLeft, MeterTop, MeterWidth, MeterHeight);
	if (FillHeight > 0.f)
	{
		DrawRect(
			FillColor,
			MeterLeft + Padding,
			MeterTop + Padding + (InnerHeight - FillHeight),
			InnerWidth,
			FillHeight);
	}

	FLinearColor BorderColor = BatteryIndicatorTextColor;
	BorderColor.A *= 0.65f;
	DrawLine(MeterLeft, MeterTop, MeterLeft + MeterWidth, MeterTop, BorderColor, 1.5f);
	DrawLine(MeterLeft + MeterWidth, MeterTop, MeterLeft + MeterWidth, MeterTop + MeterHeight, BorderColor, 1.5f);
	DrawLine(MeterLeft + MeterWidth, MeterTop + MeterHeight, MeterLeft, MeterTop + MeterHeight, BorderColor, 1.5f);
	DrawLine(MeterLeft, MeterTop + MeterHeight, MeterLeft, MeterTop, BorderColor, 1.5f);

	const float TerminalWidth = MeterWidth * 0.52f;
	const float TerminalHeight = 6.f;
	DrawRect(
		BorderColor,
		MeterLeft + (MeterWidth - TerminalWidth) * 0.5f,
		MeterTop - TerminalHeight - 3.f,
		TerminalWidth,
		TerminalHeight);

	const FString Label = TEXT("BAT");
	float LabelWidth = 0.f;
	float LabelHeight = 0.f;
	GetTextSize(Label, LabelWidth, LabelHeight, nullptr, BatteryIndicatorTextScale);
	DrawText(
		Label,
		BatteryIndicatorTextColor,
		MeterLeft + (MeterWidth - LabelWidth) * 0.5f,
		MeterTop - LabelHeight - 14.f,
		nullptr,
		BatteryIndicatorTextScale,
		false);

	const FString PercentText = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(EnergyFraction * 100.f));
	float PercentWidth = 0.f;
	float PercentHeight = 0.f;
	GetTextSize(PercentText, PercentWidth, PercentHeight, nullptr, BatteryIndicatorTextScale);
	DrawText(
		PercentText,
		BatteryIndicatorTextColor,
		MeterLeft + (MeterWidth - PercentWidth) * 0.5f,
		MeterTop + MeterHeight + 5.f,
		nullptr,
		BatteryIndicatorTextScale,
		false);
}

void ASpaceshipHUD::DrawAimReticle(const ASpaceshipPawn& Pawn)
{
	if (!bDrawAimReticle || Canvas == nullptr || PlayerOwner == nullptr)
	{
		return;
	}

	FVector2D ScreenLocation;
	if (!PlayerOwner->ProjectWorldLocationToScreen(Pawn.GetProjectileAimPoint(AimReticleDistance), ScreenLocation, false))
	{
		return;
	}

	if (ScreenLocation.X < -ReticleScreenMargin
		|| ScreenLocation.Y < -ReticleScreenMargin
		|| ScreenLocation.X > Canvas->ClipX + ReticleScreenMargin
		|| ScreenLocation.Y > Canvas->ClipY + ReticleScreenMargin)
	{
		return;
	}

	DrawAimTriangle(ScreenLocation);
}

void ASpaceshipHUD::DrawAimTriangle(const FVector2D& ScreenLocation)
{
	const FVector2D Tip(ScreenLocation.X, ScreenLocation.Y);
	const FVector2D Left(ScreenLocation.X - AimReticleWidth, ScreenLocation.Y - AimReticleHeight);
	const FVector2D Right(ScreenLocation.X + AimReticleWidth, ScreenLocation.Y - AimReticleHeight);

	DrawLine(Left.X, Left.Y, Tip.X, Tip.Y, AimReticleColor, AimReticleLineThickness);
	DrawLine(Tip.X, Tip.Y, Right.X, Right.Y, AimReticleColor, AimReticleLineThickness);
	DrawLine(Right.X, Right.Y, Left.X, Left.Y, AimReticleColor, AimReticleLineThickness);
}

void ASpaceshipHUD::DrawPitchIndicator(const ASpaceshipPawn& Pawn)
{
	if (!bDrawPitchIndicator || Canvas == nullptr)
	{
		return;
	}

	const float PitchInput = FMath::Clamp(static_cast<float>(-Pawn.CachedInput.X), -1.f, 1.f);
	const float AbsPitchInput = FMath::Abs(PitchInput);
	const float PitchStrength = AbsPitchInput > PitchIndicatorDeadZone
		? FMath::GetMappedRangeValueClamped(
			FVector2D(PitchIndicatorDeadZone, 1.f),
			FVector2D(0.f, 1.f),
			AbsPitchInput)
		: 0.f;
	const float ActivePitchAlpha = AbsPitchInput > PitchIndicatorDeadZone
		? FMath::GetMappedRangeValueClamped(
			FVector2D(PitchIndicatorDeadZone, 1.f),
			FVector2D(PitchIndicatorMinOpacity, PitchIndicatorColor.A),
			AbsPitchInput)
		: PitchIndicatorMinOpacity;

	const float CenterX = Canvas->ClipX * 0.5f;
	const FVector2D TopBaseCenter(CenterX, Canvas->ClipY * PitchIndicatorTopAnchor);
	const FVector2D BottomBaseCenter(CenterX, Canvas->ClipY * PitchIndicatorBottomAnchor);

	FLinearColor TopColor = PitchIndicatorColor;
	FLinearColor BottomColor = PitchIndicatorColor;
	TopColor.A = PitchInput > 0.f ? ActivePitchAlpha : PitchIndicatorMinOpacity;
	BottomColor.A = PitchInput < 0.f ? ActivePitchAlpha : PitchIndicatorMinOpacity;

	const float ActiveScale = FMath::Lerp(1.f, PitchIndicatorActiveScaleMultiplier, PitchStrength);
	const float TopHeight = PitchIndicatorHeight * (PitchInput > 0.f ? ActiveScale : 1.f);
	const float BottomHeight = PitchIndicatorHeight * (PitchInput < 0.f ? ActiveScale : 1.f);

	DrawPitchTriangle(TopBaseCenter, PitchIndicatorWidth, TopHeight, TopColor, false);
	DrawPitchTriangle(BottomBaseCenter, PitchIndicatorWidth, BottomHeight, BottomColor, true);
}

void ASpaceshipHUD::DrawPitchTriangle(const FVector2D& BaseCenter, float Width, float Height, const FLinearColor& Color, const bool bPointDown)
{
	const float HalfWidth = Width * 0.5f;
	const FVector2D Left(BaseCenter.X - HalfWidth, BaseCenter.Y);
	const FVector2D Right(BaseCenter.X + HalfWidth, BaseCenter.Y);
	const FVector2D Tip(BaseCenter.X, bPointDown ? (BaseCenter.Y + Height) : (BaseCenter.Y - Height));

	FCanvasTriangleItem TriangleItem(Tip, Left, Right, GWhiteTexture);
	TriangleItem.BlendMode = SE_BLEND_AlphaBlend;
	TriangleItem.SetColor(Color);
	Canvas->DrawItem(TriangleItem);
}

void ASpaceshipHUD::DrawYawIndicator(const ASpaceshipPawn& Pawn)
{
	if (!bDrawYawIndicator || Canvas == nullptr)
	{
		return;
	}

	const float YawInput = FMath::Clamp(static_cast<float>(-Pawn.CachedInput.Y), -1.f, 1.f);
	const float AbsYawInput = FMath::Abs(YawInput);
	const float YawStrength = AbsYawInput > YawIndicatorDeadZone
		? FMath::GetMappedRangeValueClamped(
			FVector2D(YawIndicatorDeadZone, 1.f),
			FVector2D(0.f, 1.f),
			AbsYawInput)
		: 0.f;
	const float ActiveYawAlpha = AbsYawInput > YawIndicatorDeadZone
		? FMath::GetMappedRangeValueClamped(
			FVector2D(YawIndicatorDeadZone, 1.f),
			FVector2D(YawIndicatorMinOpacity, YawIndicatorColor.A),
			AbsYawInput)
		: YawIndicatorMinOpacity;

	const float CenterY = Canvas->ClipY * YawIndicatorVerticalAnchor;
	const FVector2D LeftBaseCenter(Canvas->ClipX * YawIndicatorHorizontalAnchor, CenterY);
	const FVector2D RightBaseCenter(Canvas->ClipX * (1.f - YawIndicatorHorizontalAnchor), CenterY);

	FLinearColor LeftColor = YawIndicatorColor;
	FLinearColor RightColor = YawIndicatorColor;
	LeftColor.A = YawInput > 0.f ? ActiveYawAlpha : YawIndicatorMinOpacity;
	RightColor.A = YawInput < 0.f ? ActiveYawAlpha : YawIndicatorMinOpacity;

	const float ActiveScale = FMath::Lerp(1.f, YawIndicatorActiveScaleMultiplier, YawStrength);
	const float LeftWidth = YawIndicatorWidth * (YawInput > 0.f ? ActiveScale : 1.f);
	const float RightWidth = YawIndicatorWidth * (YawInput < 0.f ? ActiveScale : 1.f);

	DrawYawTriangle(LeftBaseCenter, LeftWidth, YawIndicatorHeight, LeftColor, false);
	DrawYawTriangle(RightBaseCenter, RightWidth, YawIndicatorHeight, RightColor, true);
}

void ASpaceshipHUD::DrawYawTriangle(const FVector2D& BaseCenter, float Width, float Height, const FLinearColor& Color, const bool bPointRight)
{
	const float HalfHeight = Height * 0.5f;
	const FVector2D Top(BaseCenter.X, BaseCenter.Y - HalfHeight);
	const FVector2D Bottom(BaseCenter.X, BaseCenter.Y + HalfHeight);
	const FVector2D Tip(bPointRight ? (BaseCenter.X + Width) : (BaseCenter.X - Width), BaseCenter.Y);

	FCanvasTriangleItem TriangleItem(Tip, Top, Bottom, GWhiteTexture);
	TriangleItem.BlendMode = SE_BLEND_AlphaBlend;
	TriangleItem.SetColor(Color);
	Canvas->DrawItem(TriangleItem);
}
