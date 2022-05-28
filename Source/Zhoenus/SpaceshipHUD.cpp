#include "SpaceshipHUD.h"
#include "Engine/Canvas.h"
//#include "Engine/Texture2D.h"
//#include "TextureResource.h"
//#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "SpaceshipPawn.h"

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
		FVector2D v{ Canvas->ClipX * .98f, Canvas->ClipY * .7f };

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
	}
	

}