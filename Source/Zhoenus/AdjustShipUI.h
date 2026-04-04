#pragma once
#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ShipStats.h"
#include "AdjustShipUI.generated.h"

class UCommonButtonBase;
class UCommonNumericTextBlock;
class UPowerUpStatWidgetUI;
class UWidget;

UCLASS()
class UAdjustShipUI : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonButtonBase* ZhoenusBackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonButtonBase* ZhoenusSaveButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCommonNumericTextBlock* CommonNumericTextBlock_PointsRemaining;

	UAdjustShipUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeTick(const FGeometry& Geo, float InDeltaTime) override;

private:
	void ConfigureButton(UCommonButtonBase* Button) const;
	void ConfigureNavigation() const;
	void LoadShipStats() const;
	FShipStats BuildPreviewStats() const;
	void RefreshPointsRemaining() const;
	void HandleBackClicked();
	void HandleSaveClicked();
	TArray<UPowerUpStatWidgetUI*> GetStatRows() const;
	UPowerUpStatWidgetUI* FindStatRow(const TCHAR* WidgetName) const;
	void LinkVerticalNavigation(UPowerUpStatWidgetUI* UpperRow, UPowerUpStatWidgetUI* LowerRow) const;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;
};
