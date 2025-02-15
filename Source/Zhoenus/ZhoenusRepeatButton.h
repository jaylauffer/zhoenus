#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "ZhoenusRepeatButton.generated.h"

/**
 * A custom button that fires an event repeatedly while held.
 */
UCLASS()
class UZhoenusRepeatButton : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    UZhoenusRepeatButton();

protected:
    // Overriding BP_OnPressed/BP_OnReleased to start/stop the timer
    virtual void NativeConstruct() override;
    virtual bool NativeOnHoldProgress(float DeltaTime) override;
    virtual void NativeOnPressed() override;
    virtual void NativeOnReleased() override;
    virtual void HoldReset() override;

private:

    // How often, in seconds, do we want to fire the OnIncrementTick event?
    UPROPERTY(EditAnywhere, Category = "RepeatButton")
    float RepeatInterval = 0.1f;
};
