#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpaceshipHUD.generated.h"

UCLASS()
class ASpaceshipHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASpaceshipHUD();

	virtual void BeginPlay() override;

	virtual void DrawHUD() override;

private:

	UPROPERTY(EditAnywhere, Category = "Widget")
		TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Widget")
		class UUserWidget* HUDWidget;
};