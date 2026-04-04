#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZhoenusPlayerController.h"
#include "SaveThemAllPlayerController.generated.h"

UCLASS(Config=Game)
class ASaveThemAllPlayerController : public AZhoenusPlayerController
{
	GENERATED_BODY()

public:
	ASaveThemAllPlayerController();
	~ASaveThemAllPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> wStartText;

protected:
	virtual void OnPossess(APawn* pawn) override;
	virtual void OnUnPossess() override;

private:

};
