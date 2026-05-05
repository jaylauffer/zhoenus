#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "ZhoenusTouchUI.generated.h"

class UButton;
class UTexture2D;

UCLASS()
class UZhoenusTouchUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ButtonFire;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ButtonStabilize;

	UZhoenusTouchUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Touch", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UTexture2D> TouchButtonTexture;

	void ApplyTouchButtonTexture();
	void ApplyTouchButtonTextureToButton(UButton* Button, UTexture2D* ButtonTexture) const;
};
