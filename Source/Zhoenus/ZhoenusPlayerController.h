#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZhoenusPlayerController.generated.h"

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

	FDateTime SkillCD;	// only used on server side.
	
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
