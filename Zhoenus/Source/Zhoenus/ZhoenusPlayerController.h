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
protected:
	virtual void OnPossess(APawn* pawn) override;
	virtual void OnUnPossess() override;

private:
	/** Bound to the thrust axis */
	UFUNCTION(Server, Reliable, WithValidation)
	void ThrustInput(float Val);

	/** Bound to the vertical axis */
	UFUNCTION(Server, Reliable, WithValidation)
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	UFUNCTION(Server, Reliable, WithValidation)
	void MoveRightInput(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void RotateRightInput(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void DisengageAutoCorrect(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void FireWeapon(float Val);

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

	//UFUNCTION()
	//void TestClick();
};