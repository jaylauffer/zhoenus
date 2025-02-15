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
	class UCommonButtonBase* ZhoenusBackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCommonButtonBase* ZhoenusSaveButton;

//	UFUNCTION()

	UAdjustShipUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;
};
