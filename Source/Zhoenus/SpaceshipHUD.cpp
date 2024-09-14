#include "SpaceshipHUD.h"
#include "Engine/Canvas.h"
//#include "Engine/Texture2D.h"
//#include "TextureResource.h"
//#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "SpaceshipPawn.h"

namespace
{
	const FLinearColor PitchColor(1.f, 0.f, 1.f);
	const FLinearColor YawColor(0.5f, 1.f, 0.5f);
	const FLinearColor RollColor(.3f, 0, 1.f);
	const FLinearColor StabilizeColor(.7f, 0, .5f);
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
		//v is thrust
		FVector2D v{ Canvas->ClipX * .98f, Canvas->ClipY * .7f };

		FVector2D p{ Canvas->ClipX * .98f, Canvas->ClipY * .7f };
		FVector2D rl{ Canvas->ClipX * .04f, Canvas->ClipY * .7f };
		FVector2D rr{ Canvas->ClipX * .96f, Canvas->ClipY * .7f };
		FVector2D y{ Canvas->ClipX * .5f, Canvas->ClipY * .1f };

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

		//work through these.. pitch, 
		p.X -= 16.f;
		double Pitch{ -pawn->CachedInput.X };
		if (Pitch > 0.02f)
		{
			FVector2D e{ 10.f, -Pitch * Canvas->ClipY * .6f };
			DrawRect(PitchColor, p.X, p.Y, e.X, e.Y);
		}
		else if (Pitch < -0.02f)
		{
			FVector2D e{ 10.f, -Pitch * Canvas->ClipY * .2f };
			DrawRect(PitchColor, p.X, p.Y, e.X, e.Y);
		}

		//work through these.. roll
		//split the roll to two bars (left one and right one)
		//rl.X -= 18.f;
		double Roll{ -pawn->CachedInput.Z};
		if (Roll > 0.02f)
		{
			FVector2D e{ 10.f, -Roll * Canvas->ClipY * .6f };
			DrawRect(RollColor, rl.X, rl.Y, e.X, e.Y);
		}
		else if (Roll < -0.02f)
		{
			FVector2D e{ 10.f, -Roll * Canvas->ClipY * .6f };
			DrawRect(RollColor, rr.X, rr.Y, e.X, e.Y);
		}

		//work through these.. yaw
		y.X -= 20.f;
		double Yaw{ -pawn->CachedInput.Y };
		if (Yaw > 0.02f)
		{
			FVector2D e{ -Yaw * Canvas->ClipX * .6f, 10.f };
			DrawRect(YawColor, y.X, y.Y, e.X, e.Y);
		}
		else if (Yaw < -0.02f)
		{
			FVector2D e{ -Yaw * Canvas->ClipY * .6f, 10.f };
			DrawRect(YawColor, y.X, y.Y, e.X, e.Y);
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