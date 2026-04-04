#include "AdjustShipUI.h"

#include "CommonButtonBase.h"
#include "CommonNumericTextBlock.h"
#include "Components/Widget.h"
#include "PowerUpStatWidgetUI.h"
#include "SaveThemAllGameInstance.h"

UAdjustShipUI::UAdjustShipUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAdjustShipUI::NativeConstruct()
{
	Super::NativeConstruct();

	const auto ApplyStatRowLabel = [this](const TCHAR* WidgetName, const TCHAR* LabelText)
	{
		if (UPowerUpStatWidgetUI* Row = FindStatRow(WidgetName))
		{
			Row->SetDisplayLabelText(FText::FromString(LabelText));
		}
	};

	ApplyStatRowLabel(TEXT("PowerUpText_ForwardAcceleration"), TEXT("Forward Acceleration"));
	ApplyStatRowLabel(TEXT("PowerUpText_BackwardsAcceleration"), TEXT("Backwards Acceleration"));
	ApplyStatRowLabel(
		TEXT("PowerUpText_PitchAcceleration"),
		TEXT("Pitch (Up/Down) Acceleration"));
	ApplyStatRowLabel(
		TEXT("PowerUpText_YawAcceleration"),
		TEXT("Yaw (Turn Left/Right) Acceleration"));
	ApplyStatRowLabel(TEXT("PowerUpText_RollAcceleration"), TEXT("Roll Acceleration"));

	SetIsFocusable(true);
	ConfigureButton(ZhoenusSaveButton);
	ConfigureButton(ZhoenusBackButton);

	if (ZhoenusSaveButton)
	{
		ZhoenusSaveButton->OnClicked().RemoveAll(this);
		ZhoenusSaveButton->OnClicked().AddUObject(this, &UAdjustShipUI::HandleSaveClicked);
	}
	if (ZhoenusBackButton)
	{
		ZhoenusBackButton->OnClicked().RemoveAll(this);
		ZhoenusBackButton->OnClicked().AddUObject(this, &UAdjustShipUI::HandleBackClicked);
	}

	LoadShipStats();
	RefreshPointsRemaining();
	ConfigureNavigation();
}

void UAdjustShipUI::NativeOnActivated()
{
	Super::NativeOnActivated();

	LoadShipStats();
	RefreshPointsRemaining();
}

void UAdjustShipUI::NativeTick(const FGeometry& Geo, float InDeltaTime)
{
	Super::NativeTick(Geo, InDeltaTime);

	RefreshPointsRemaining();
}

void UAdjustShipUI::ConfigureButton(UCommonButtonBase* Button) const
{
	if (!Button)
	{
		return;
	}

	Button->SetIsFocusable(true);
	Button->SetIsSelectable(true);
	Button->SetIsInteractableWhenSelected(true);
	Button->SetShouldSelectUponReceivingFocus(true);
}

void UAdjustShipUI::ConfigureNavigation() const
{
	const TArray<UPowerUpStatWidgetUI*> StatRows = GetStatRows();
	for (int32 RowIndex = 0; RowIndex < StatRows.Num(); ++RowIndex)
	{
		if (RowIndex + 1 < StatRows.Num())
		{
			LinkVerticalNavigation(StatRows[RowIndex], StatRows[RowIndex + 1]);
		}
	}

	if (StatRows.Num() > 0)
	{
		if (UCommonButtonBase* FirstDecrement = StatRows[0]->GetDecrementButton())
		{
			FirstDecrement->SetNavigationRuleExplicit(EUINavigation::Left, FirstDecrement);
		}
		if (UCommonButtonBase* FirstIncrement = StatRows[0]->GetIncrementButton())
		{
			FirstIncrement->SetNavigationRuleExplicit(EUINavigation::Right, FirstIncrement);
		}
	}

	if (ZhoenusBackButton && ZhoenusSaveButton)
	{
		ZhoenusBackButton->SetNavigationRuleExplicit(EUINavigation::Right, ZhoenusSaveButton);
		ZhoenusSaveButton->SetNavigationRuleExplicit(EUINavigation::Left, ZhoenusBackButton);
		ZhoenusBackButton->SetNavigationRuleExplicit(EUINavigation::Next, ZhoenusSaveButton);
		ZhoenusSaveButton->SetNavigationRuleExplicit(EUINavigation::Previous, ZhoenusBackButton);
	}

	if (StatRows.Num() > 0)
	{
		if (UPowerUpStatWidgetUI* LastRow = StatRows.Last())
		{
			if (UCommonButtonBase* LastDecrement = LastRow->GetDecrementButton())
			{
				LastDecrement->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusBackButton);
				if (ZhoenusBackButton)
				{
					ZhoenusBackButton->SetNavigationRuleExplicit(EUINavigation::Up, LastDecrement);
				}
			}
			if (UCommonButtonBase* LastIncrement = LastRow->GetIncrementButton())
			{
				LastIncrement->SetNavigationRuleExplicit(EUINavigation::Down, ZhoenusSaveButton);
				if (ZhoenusSaveButton)
				{
					ZhoenusSaveButton->SetNavigationRuleExplicit(EUINavigation::Up, LastIncrement);
				}
			}
		}
	}
}

void UAdjustShipUI::LoadShipStats() const
{
	const USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	if (UPowerUpStatWidgetUI* ForwardRow = FindStatRow(TEXT("PowerUpText_ForwardAcceleration")))
	{
		ForwardRow->SetDisplayedStatValue(GameInstance->shipStats.ForwardAcceleration);
	}
	if (UPowerUpStatWidgetUI* ReverseRow = FindStatRow(TEXT("PowerUpText_BackwardsAcceleration")))
	{
		ReverseRow->SetDisplayedStatValue(GameInstance->shipStats.ReverseAcceleration);
	}
	if (UPowerUpStatWidgetUI* PitchRow = FindStatRow(TEXT("PowerUpText_PitchAcceleration")))
	{
		PitchRow->SetDisplayedStatValue(GameInstance->shipStats.PitchAcceleration);
	}
	if (UPowerUpStatWidgetUI* YawRow = FindStatRow(TEXT("PowerUpText_YawAcceleration")))
	{
		YawRow->SetDisplayedStatValue(GameInstance->shipStats.YawAcceleration);
	}
	if (UPowerUpStatWidgetUI* RollRow = FindStatRow(TEXT("PowerUpText_RollAcceleration")))
	{
		RollRow->SetDisplayedStatValue(GameInstance->shipStats.RollAcceleration);
	}
}

FShipStats UAdjustShipUI::BuildPreviewStats() const
{
	const USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	FShipStats PreviewStats = GameInstance ? GameInstance->shipStats : FShipStats{};

	if (const UPowerUpStatWidgetUI* ForwardRow = FindStatRow(TEXT("PowerUpText_ForwardAcceleration")))
	{
		PreviewStats.ForwardAcceleration = ForwardRow->GetDisplayedStatValue();
	}
	if (const UPowerUpStatWidgetUI* ReverseRow = FindStatRow(TEXT("PowerUpText_BackwardsAcceleration")))
	{
		PreviewStats.ReverseAcceleration = ReverseRow->GetDisplayedStatValue();
	}
	if (const UPowerUpStatWidgetUI* PitchRow = FindStatRow(TEXT("PowerUpText_PitchAcceleration")))
	{
		PreviewStats.PitchAcceleration = PitchRow->GetDisplayedStatValue();
	}
	if (const UPowerUpStatWidgetUI* YawRow = FindStatRow(TEXT("PowerUpText_YawAcceleration")))
	{
		PreviewStats.YawAcceleration = YawRow->GetDisplayedStatValue();
	}
	if (const UPowerUpStatWidgetUI* RollRow = FindStatRow(TEXT("PowerUpText_RollAcceleration")))
	{
		PreviewStats.RollAcceleration = RollRow->GetDisplayedStatValue();
	}

	return PreviewStats;
}

void UAdjustShipUI::RefreshPointsRemaining() const
{
	const USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	if (!GameInstance || !CommonNumericTextBlock_PointsRemaining)
	{
		return;
	}

	const float RemainingPoints = GameInstance->GetShipAdjustmentRemainingPoints(BuildPreviewStats());
	CommonNumericTextBlock_PointsRemaining->SetCurrentValue(RemainingPoints);
}

void UAdjustShipUI::HandleBackClicked()
{
	DeactivateWidget();
}

void UAdjustShipUI::HandleSaveClicked()
{
	USaveThemAllGameInstance* GameInstance = GetGameInstance<USaveThemAllGameInstance>();
	if (!GameInstance)
	{
		return;
	}

	const FShipStats UpdatedStats = BuildPreviewStats();
	const float RemainingPoints = GameInstance->GetShipAdjustmentRemainingPoints(UpdatedStats);
	if (RemainingPoints < 0.f)
	{
		return;
	}

	GameInstance->shipStats = UpdatedStats;
	GameInstance->points = RemainingPoints;
	GameInstance->SaveGame();
	DeactivateWidget();
}

TArray<UPowerUpStatWidgetUI*> UAdjustShipUI::GetStatRows() const
{
	TArray<UPowerUpStatWidgetUI*> StatRows;
	StatRows.Reserve(5);

	if (UPowerUpStatWidgetUI* Row = FindStatRow(TEXT("PowerUpText_ForwardAcceleration")))
	{
		StatRows.Add(Row);
	}
	if (UPowerUpStatWidgetUI* Row = FindStatRow(TEXT("PowerUpText_BackwardsAcceleration")))
	{
		StatRows.Add(Row);
	}
	if (UPowerUpStatWidgetUI* Row = FindStatRow(TEXT("PowerUpText_PitchAcceleration")))
	{
		StatRows.Add(Row);
	}
	if (UPowerUpStatWidgetUI* Row = FindStatRow(TEXT("PowerUpText_YawAcceleration")))
	{
		StatRows.Add(Row);
	}
	if (UPowerUpStatWidgetUI* Row = FindStatRow(TEXT("PowerUpText_RollAcceleration")))
	{
		StatRows.Add(Row);
	}

	return StatRows;
}

UPowerUpStatWidgetUI* UAdjustShipUI::FindStatRow(const TCHAR* WidgetName) const
{
	return Cast<UPowerUpStatWidgetUI>(GetWidgetFromName(WidgetName));
}

void UAdjustShipUI::LinkVerticalNavigation(UPowerUpStatWidgetUI* UpperRow, UPowerUpStatWidgetUI* LowerRow) const
{
	if (!UpperRow || !LowerRow)
	{
		return;
	}

	UCommonButtonBase* UpperDecrement = UpperRow->GetDecrementButton();
	UCommonButtonBase* UpperIncrement = UpperRow->GetIncrementButton();
	UCommonButtonBase* LowerDecrement = LowerRow->GetDecrementButton();
	UCommonButtonBase* LowerIncrement = LowerRow->GetIncrementButton();

	if (UpperDecrement && LowerDecrement)
	{
		UpperDecrement->SetNavigationRuleExplicit(EUINavigation::Down, LowerDecrement);
		LowerDecrement->SetNavigationRuleExplicit(EUINavigation::Up, UpperDecrement);
	}

	if (UpperIncrement && LowerIncrement)
	{
		UpperIncrement->SetNavigationRuleExplicit(EUINavigation::Down, LowerIncrement);
		LowerIncrement->SetNavigationRuleExplicit(EUINavigation::Up, UpperIncrement);
	}
}

UWidget* UAdjustShipUI::NativeGetDesiredFocusTarget() const
{
	if (UPowerUpStatWidgetUI* ForwardRow = FindStatRow(TEXT("PowerUpText_ForwardAcceleration")))
	{
		return ForwardRow->GetDesiredFocusTarget();
	}
	if (ZhoenusSaveButton)
	{
		return ZhoenusSaveButton;
	}
	if (ZhoenusBackButton)
	{
		return ZhoenusBackButton;
	}
	return Super::NativeGetDesiredFocusTarget();
}
