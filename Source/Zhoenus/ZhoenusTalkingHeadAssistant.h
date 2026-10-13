// Copyright 2026 Jay Lauffer

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"
#include "ZhoenusTalkingHeadAssistant.generated.h"

class ASpaceshipPawn;
class AZhoenusTalkingHeadAssistant;
class UBillboardComponent;
class UPointLightComponent;
class USceneComponent;
class USphereComponent;
class UTextRenderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FZhoenusTalkingHeadUtterance,
	AZhoenusTalkingHeadAssistant*, Assistant,
	FText, PlayerUtterance);

UCLASS(Blueprintable, Config=Game)
class ZHOENUS_API AZhoenusTalkingHeadAssistant : public AActor
{
	GENERATED_BODY()

public:
	AZhoenusTalkingHeadAssistant();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Zhoenus|Assistant")
	void SubmitPlayerUtterance(const FText& PlayerUtterance);

	UFUNCTION(BlueprintCallable, Category = "Zhoenus|Assistant")
	void SetAssistantLine(const FText& NewLine, bool bNewSpeaking);

	UFUNCTION(BlueprintCallable, Category = "Zhoenus|Assistant")
	void SetSpeaking(bool bNewSpeaking);

	UFUNCTION(BlueprintCallable, Category = "Zhoenus|Assistant")
	void RefreshGameplayHint();

	UPROPERTY(BlueprintAssignable, Category = "Zhoenus|Assistant")
	FZhoenusTalkingHeadUtterance OnPlayerUtterance;

protected:
	FText BuildFallbackReply(const FText& PlayerUtterance) const;
	FText BuildGameplayHint() const;
	FString BuildModelInstructions() const;
	void AskLocalModel(const FText& PlayerUtterance);
	int32 GetFollowerCount(const ASpaceshipPawn* Ship) const;
	void ApplyVisualAssets();
	void FacePlayerCamera();
	void UpdateSpeakingPulse(float DeltaSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assistant")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assistant")
	TObjectPtr<UBillboardComponent> FaceBillboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assistant")
	TObjectPtr<UTextRenderComponent> CaptionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assistant")
	TObjectPtr<UPointLightComponent> GlowLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assistant")
	TObjectPtr<USphereComponent> InteractionZone;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Visual")
	FSoftObjectPath FaceTexturePath;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Visual", meta = (ClampMin = "0.1"))
	float FaceScale = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assistant|Text")
	FText DefaultLine;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Text", meta = (ClampMin = "0.5"))
	float HintRefreshSeconds = 2.5f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Text", meta = (ClampMin = "20.0"))
	float CaptionWorldSize = 82.f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Visual")
	bool bFacePlayerCamera = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Gameplay")
	bool bAutoUpdateGameplayHints = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowSaveFraction = 0.3f;

	/** Ask loadngo's gpt_oss_serve on this machine; the built-in reply shows meanwhile and stays when no model answers. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Model")
	bool bAskLocalModel = true;

	/** The model's Unix socket; empty means ~/.loadngo/node/gpt-oss.sock. -ZhoenusModelSocket= overrides it. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Model")
	FString ModelSocketPath;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Model", meta = (ClampMin = "1000"))
	int32 ModelDeadlineMs = 30000;

	/** How long a model's answer stays before gameplay hints resume. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Assistant|Model", meta = (ClampMin = "1.0"))
	float ModelReplyHoldSeconds = 12.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Assistant|State")
	bool bSpeaking = false;

private:
	float TimeUntilNextHint = 0.f;
	float SpeakingPulseTime = 0.f;
	/** The latest question sent to the model; an older reply is dropped. */
	int32 ModelRequestSerial = 0;
};
