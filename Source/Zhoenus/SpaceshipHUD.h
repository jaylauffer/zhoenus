#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpaceshipHUD.generated.h"

UCLASS(Config=Game)
class ASpaceshipHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASpaceshipHUD();

	virtual void BeginPlay() override;

	virtual void DrawHUD() override;

private:
	void DrawAimReticle(const class ASpaceshipPawn& Pawn);
	void DrawAimTriangle(const FVector2D& ScreenLocation);
	void DrawAimRangeReadout(const class ASpaceshipPawn& Pawn);
	void DrawBatteryIndicator(const class ASpaceshipPawn& Pawn);
	void DrawPitchIndicator(const class ASpaceshipPawn& Pawn);
	void DrawPitchTriangle(const FVector2D& BaseCenter, float Width, float Height, const FLinearColor& Color, bool bPointDown);
	void DrawYawIndicator(const class ASpaceshipPawn& Pawn);
	void DrawYawTriangle(const FVector2D& BaseCenter, float Width, float Height, const FLinearColor& Color, bool bPointRight);

	UPROPERTY(EditAnywhere, Category = "Widget")
		TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Widget")
		class UUserWidget* HUDWidget;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	bool bDrawAimReticle = true;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "250.0"))
	float AimReticleDistance = 5000.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "6.0"))
	float AimReticleWidth = 18.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "8.0"))
	float AimReticleHeight = 22.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "0.5"))
	float AimReticleLineThickness = 2.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	FLinearColor AimReticleColor = FLinearColor(0.68f, 1.f, 0.2f, 0.58f);

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	bool bDrawAimRangeReadout = true;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	bool bAimRangeReadoutRequireBlockingHit = true;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "0.0"))
	float AimRangeReadoutVerticalOffset = 28.f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle", meta = (ClampMin = "0.25"))
	float AimRangeReadoutTextScale = 0.9f;

	UPROPERTY(Config, EditAnywhere, Category = "Reticle")
	FLinearColor AimRangeReadoutColor = FLinearColor(0.82f, 1.f, 0.86f, 0.92f);

	UPROPERTY(Config, EditAnywhere, Category = "Battery")
	bool bDrawBatteryIndicator = true;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BatteryIndicatorHorizontalAnchor = 0.91f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BatteryIndicatorVerticalAnchor = 0.72f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "8.0"))
	float BatteryIndicatorWidth = 18.f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "24.0"))
	float BatteryIndicatorHeight = 136.f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "0.0"))
	float BatteryIndicatorPadding = 3.f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BatteryIndicatorLowThreshold = 0.2f;

	UPROPERTY(Config, EditAnywhere, Category = "Battery")
	FLinearColor BatteryIndicatorColor = FLinearColor(0.14f, 0.84f, 1.f, 0.85f);

	UPROPERTY(Config, EditAnywhere, Category = "Battery")
	FLinearColor BatteryIndicatorLowColor = FLinearColor(1.f, 0.38f, 0.1f, 0.92f);

	UPROPERTY(Config, EditAnywhere, Category = "Battery")
	FLinearColor BatteryIndicatorBackgroundColor = FLinearColor(0.04f, 0.08f, 0.1f, 0.28f);

	UPROPERTY(Config, EditAnywhere, Category = "Battery")
	FLinearColor BatteryIndicatorTextColor = FLinearColor(0.92f, 0.96f, 1.f, 0.9f);

	UPROPERTY(Config, EditAnywhere, Category = "Battery", meta = (ClampMin = "0.25"))
	float BatteryIndicatorTextScale = 0.9f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch")
	bool bDrawPitchIndicator = true;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PitchIndicatorTopAnchor = 0.12f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PitchIndicatorBottomAnchor = 0.88f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "8.0"))
	float PitchIndicatorWidth = 84.f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "8.0"))
	float PitchIndicatorHeight = 72.f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch")
	FLinearColor PitchIndicatorColor = FLinearColor(1.f, 0.45f, 0.08f, 0.12f);

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PitchIndicatorMinOpacity = 0.015f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PitchIndicatorDeadZone = 0.02f;

	UPROPERTY(Config, EditAnywhere, Category = "Pitch", meta = (ClampMin = "0.0"))
	float PitchIndicatorActiveScaleMultiplier = 1.6f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw")
	bool bDrawYawIndicator = true;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YawIndicatorVerticalAnchor = 0.1f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float YawIndicatorHorizontalAnchor = 0.16f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "8.0"))
	float YawIndicatorWidth = 72.f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "8.0"))
	float YawIndicatorHeight = 54.f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw")
	FLinearColor YawIndicatorColor = FLinearColor(0.05f, 1.f, 0.05f, 0.12f);

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YawIndicatorMinOpacity = 0.015f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YawIndicatorDeadZone = 0.02f;

	UPROPERTY(Config, EditAnywhere, Category = "Yaw", meta = (ClampMin = "0.0"))
	float YawIndicatorActiveScaleMultiplier = 1.8f;
};
