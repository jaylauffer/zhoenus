#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AdjustShipUI.generated.h"

UCLASS()
class UAdjustShipUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Return;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Save;

	UAdjustShipUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;
};
