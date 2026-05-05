#include "ZhoenusTouchUI.h"

#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

namespace
{
	void SetBrushTexturePreservingSize(FSlateBrush& Brush, UTexture2D* Texture)
	{
		if (!Texture)
		{
			return;
		}

		const FVector2D ExistingSize = Brush.ImageSize;
		Brush.SetResourceObject(Texture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.ImageSize = (ExistingSize.X > 0.0f && ExistingSize.Y > 0.0f)
			? ExistingSize
			: FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
	}
}

UZhoenusTouchUI::UZhoenusTouchUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TouchButtonTexture = TSoftObjectPtr<UTexture2D>(
		FSoftObjectPath(TEXT("/Game/Touch/ugly-bubble-thumb3.ugly-bubble-thumb3")));
}

void UZhoenusTouchUI::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyTouchButtonTexture();
}

void UZhoenusTouchUI::NativeTick(const FGeometry& Geo, float InDeltaTime)
{
	Super::NativeTick(Geo, InDeltaTime);
}

void UZhoenusTouchUI::ApplyTouchButtonTexture()
{
	UTexture2D* ButtonTexture = TouchButtonTexture.LoadSynchronous();
	if (!IsValid(ButtonTexture))
	{
		return;
	}

	ApplyTouchButtonTextureToButton(ButtonFire, ButtonTexture);
	ApplyTouchButtonTextureToButton(ButtonStabilize, ButtonTexture);
}

void UZhoenusTouchUI::ApplyTouchButtonTextureToButton(UButton* Button, UTexture2D* ButtonTexture) const
{
	if (!IsValid(Button) || !IsValid(ButtonTexture))
	{
		return;
	}

	FButtonStyle ButtonStyle = Button->GetStyle();
	SetBrushTexturePreservingSize(ButtonStyle.Normal, ButtonTexture);
	SetBrushTexturePreservingSize(ButtonStyle.Hovered, ButtonTexture);
	SetBrushTexturePreservingSize(ButtonStyle.Pressed, ButtonTexture);
	SetBrushTexturePreservingSize(ButtonStyle.Disabled, ButtonTexture);
	Button->SetStyle(ButtonStyle);
}
