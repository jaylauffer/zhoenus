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
