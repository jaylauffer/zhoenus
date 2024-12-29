#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AdjustShipUI.generated.h"

UCLASS()
class UAdjustShipUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCommonButtonBase* BackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCommonButtonBase* SaveButton;

	UAdjustShipUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;
};
