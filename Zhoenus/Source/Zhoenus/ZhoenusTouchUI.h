#pragma once
#include "CoreMinimal.h"
#include "UserWidget.h"
#include "ZhoenusTouchUI.generated.h"

UCLASS()
class UZhoenusTouchUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ButtonFire;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* ButtonStabilize;

	UZhoenusTouchUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;
};
