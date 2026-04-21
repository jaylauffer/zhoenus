#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Templates/SharedPointer.h"
#include "ZhoenusPlayerController.generated.h"

class SVirtualJoystick;
class ZhoenusThumbstick;

UCLASS(Config=Game)
class AZhoenusPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AZhoenusPlayerController();
	~AZhoenusPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UZhoenusTouchUI> wTouchUI;

	UZhoenusTouchUI* touchUI;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor) override;
protected:
	virtual void OnPossess(APawn* pawn) override;
	virtual void OnUnPossess() override;
	virtual TSharedPtr<SVirtualJoystick> CreateVirtualJoystick() override;

private:
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	bool TryHandleDebugConsoleCommand(const TCHAR* Cmd, FOutputDevice& Ar);
	void GrantPowerPoints(float Amount);
#endif

	UFUNCTION()
	void DisengageAutoCorrect(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDisengageAutoCorrect(float Val);

	UFUNCTION()
	void FireWeapon(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireWeapon(float Val);

	void HandleStickPressureChanged(int32 ControlIndex, float Pressure, bool bIsActive);
	void RefreshTouchPressureActions();
	float NormalizeTouchPressure(float RawPressure, float Deadzone, float Scale) const;
	void UpdateTouchToggleVisuals() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 StabilizePressureControlIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 FirePressureControlIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "0.95"))
	float StabilizePressureDeadzone = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "0.95"))
	float FirePressureDeadzone = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float StabilizePressureScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "2.0"))
	float FirePressureScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true"))
	FLinearColor TouchToggleInactiveColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true"))
	FLinearColor TouchFireToggleActiveColor = FLinearColor(1.0f, 0.45f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Touch Pressure", meta = (AllowPrivateAccess = "true"))
	FLinearColor TouchStabilizeToggleActiveColor = FLinearColor(0.35f, 0.85f, 1.0f, 1.0f);

	FDateTime SkillCD;	// only used on server side.
	TSharedPtr<ZhoenusThumbstick> PressureThumbstick;
	bool bTouchFirePressureModeEnabled = false;
	bool bTouchStabilizePressureModeEnabled = false;
	
	//UFUNCTION(Server, Reliable)
	UFUNCTION()
	void TouchFirePressed();
	//UFUNCTION(Server, Reliable)
	UFUNCTION()
	void TouchFireReleased();
	//UFUNCTION(Server, Reliable)
	UFUNCTION()
	void TouchAutoCorrectPressed();
	//UFUNCTION(Server, Reliable)
	UFUNCTION()
	void TouchAutoCorrectReleased();
};
