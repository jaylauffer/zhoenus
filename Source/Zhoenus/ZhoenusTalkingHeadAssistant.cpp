// Copyright 2026 Jay Lauffer

#include "ZhoenusTalkingHeadAssistant.h"
#include "DonutFlyerPawn.h"
#include "SaveThemAllGameState.h"
#include "SpaceshipPawn.h"
#include "Components/BillboardComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Texture2D.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AZhoenusTalkingHeadAssistant::AZhoenusTalkingHeadAssistant()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	FaceBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("FaceProjection"));
	FaceBillboard->SetupAttachment(SceneRoot);
	FaceBillboard->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	FaceBillboard->SetRelativeScale3D(FVector(FaceScale));
	FaceBillboard->bIsScreenSizeScaled = true;
	FaceBillboard->ScreenSize = 0.0016f;

	CaptionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CaptionText"));
	CaptionText->SetupAttachment(SceneRoot);
	CaptionText->SetRelativeLocation(FVector(0.f, 0.f, -72.f));
	CaptionText->SetHorizontalAlignment(EHTA_Center);
	CaptionText->SetVerticalAlignment(EVRTA_TextCenter);
	CaptionText->SetTextRenderColor(FColor(160, 235, 255, 235));
	CaptionText->SetWorldSize(CaptionWorldSize);

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AssistantGlow"));
	GlowLight->SetupAttachment(SceneRoot);
	GlowLight->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	GlowLight->SetLightColor(FLinearColor(0.18f, 0.78f, 1.f));
	GlowLight->SetIntensity(700.f);
	GlowLight->SetAttenuationRadius(720.f);

	InteractionZone = CreateDefaultSubobject<USphereComponent>(TEXT("NaturalLanguageInteractionZone"));
	InteractionZone->SetupAttachment(SceneRoot);
	InteractionZone->SetSphereRadius(700.f);
	InteractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	FaceTexturePath = FSoftObjectPath(TEXT("/Game/Textures/zhoenus-eddie-j.zhoenus-eddie-j"));
	DefaultLine = NSLOCTEXT("ZhoenusTalkingHead", "DefaultLine", "Jay local head online. Ask me how to save the flyers.");
}

void AZhoenusTalkingHeadAssistant::BeginPlay()
{
	Super::BeginPlay();

	if (FaceBillboard != nullptr)
	{
		FaceBillboard->SetRelativeScale3D(FVector(FaceScale));
	}
	if (CaptionText != nullptr)
	{
		CaptionText->SetWorldSize(CaptionWorldSize);
	}

	ApplyVisualAssets();
	SetAssistantLine(DefaultLine, false);
	TimeUntilNextHint = 0.f;
}

void AZhoenusTalkingHeadAssistant::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFacePlayerCamera)
	{
		FacePlayerCamera();
	}

	UpdateSpeakingPulse(DeltaSeconds);

	if (!bAutoUpdateGameplayHints)
	{
		return;
	}

	TimeUntilNextHint -= DeltaSeconds;
	if (TimeUntilNextHint <= 0.f)
	{
		RefreshGameplayHint();
		TimeUntilNextHint = HintRefreshSeconds;
	}
}

void AZhoenusTalkingHeadAssistant::SubmitPlayerUtterance(const FText& PlayerUtterance)
{
	OnPlayerUtterance.Broadcast(this, PlayerUtterance);
	SetAssistantLine(BuildFallbackReply(PlayerUtterance), true);
}

void AZhoenusTalkingHeadAssistant::SetAssistantLine(const FText& NewLine, bool bNewSpeaking)
{
	if (CaptionText != nullptr)
	{
		CaptionText->SetText(NewLine);
	}
	SetSpeaking(bNewSpeaking);
}

void AZhoenusTalkingHeadAssistant::SetSpeaking(bool bNewSpeaking)
{
	bSpeaking = bNewSpeaking;
	if (!bSpeaking)
	{
		SpeakingPulseTime = 0.f;
		if (FaceBillboard != nullptr)
		{
			FaceBillboard->SetRelativeScale3D(FVector(FaceScale));
		}
	}
}

void AZhoenusTalkingHeadAssistant::RefreshGameplayHint()
{
	SetAssistantLine(BuildGameplayHint(), false);
}

FText AZhoenusTalkingHeadAssistant::BuildFallbackReply(const FText& PlayerUtterance) const
{
	FString Utterance = PlayerUtterance.ToString();
	Utterance.ToLowerInline();

	if (Utterance.Contains(TEXT("who")) || Utterance.Contains(TEXT("you")))
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "IdentityReply", "I am Jay's local head in the machine. I watch the flyers and keep the run readable.");
	}

	if (Utterance.Contains(TEXT("goal")) || Utterance.Contains(TEXT("gate")))
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "GoalReply", "Bring followers behind your ship, then cross the gate cleanly. Do not orbit the mouth of it.");
	}

	if (Utterance.Contains(TEXT("battery")) || Utterance.Contains(TEXT("zap")) || Utterance.Contains(TEXT("shoot")))
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "BatteryReply", "ZapEm shots are a control tool, not the goal. Spend battery to wake or steer flyers, then let it recharge.");
	}

	if (Utterance.Contains(TEXT("fly")) || Utterance.Contains(TEXT("thrust")) || Utterance.Contains(TEXT("control")))
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "FlightReply", "Use smooth six-axis corrections. Gather first, line up second, score last.");
	}

	if (Utterance.Contains(TEXT("donut")) || Utterance.Contains(TEXT("flyer")) || Utterance.Contains(TEXT("help")))
	{
		return BuildGameplayHint();
	}

	return BuildGameplayHint();
}

FText AZhoenusTalkingHeadAssistant::BuildGameplayHint() const
{
	const UWorld* World = GetWorld();
	const ASaveThemAllGameState* GameState = World != nullptr ? World->GetGameState<ASaveThemAllGameState>() : nullptr;
	const int32 Saved = GameState != nullptr ? GameState->Saved : 0;
	const int32 Total = GameState != nullptr ? GameState->Total : 0;
	const int32 Remaining = FMath::Max(0, Total - Saved);
	const ASpaceshipPawn* Ship = Cast<ASpaceshipPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	const int32 Followers = GetFollowerCount(Ship);

	if (Total <= 0)
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "NoFlyersHint", "I do not see the flyer count yet. Find the swarm and wake them.");
	}

	if (Remaining == 0)
	{
		return NSLOCTEXT("ZhoenusTalkingHead", "AllSavedHint", "All flyers are accounted for. Hold the line until the run closes.");
	}

	if (Followers > 0)
	{
		return FText::Format(
			NSLOCTEXT("ZhoenusTalkingHead", "FollowersHint", "{0} flyer(s) are with you. Line them up and cross the goal cleanly."),
			FText::AsNumber(Followers));
	}

	const float SavedFraction = Total > 0 ? static_cast<float>(Saved) / static_cast<float>(Total) : 0.f;
	if (Saved == 0)
	{
		return FText::Format(
			NSLOCTEXT("ZhoenusTalkingHead", "OpeningHint", "{0} flyer(s) still need saving. Wake one, turn gently, and make it follow."),
			FText::AsNumber(Remaining));
	}

	if (SavedFraction <= LowSaveFraction)
	{
		return FText::Format(
			NSLOCTEXT("ZhoenusTalkingHead", "LowSaveHint", "{0}/{1} saved. Slow the approach and gather before diving at the gate."),
			FText::AsNumber(Saved),
			FText::AsNumber(Total));
	}

	return FText::Format(
		NSLOCTEXT("ZhoenusTalkingHead", "ProgressHint", "{0}/{1} saved. Keep sweeping for stragglers before the song ends."),
		FText::AsNumber(Saved),
		FText::AsNumber(Total));
}

int32 AZhoenusTalkingHeadAssistant::GetFollowerCount(const ASpaceshipPawn* Ship) const
{
	if (Ship == nullptr)
	{
		return 0;
	}

	int32 Count = 0;
	for (const TWeakObjectPtr<ADonutFlyerPawn>& Follower : Ship->Followers)
	{
		if (Follower.IsValid())
		{
			++Count;
		}
	}
	return Count;
}

void AZhoenusTalkingHeadAssistant::ApplyVisualAssets()
{
	if (FaceBillboard == nullptr || FaceTexturePath.IsNull())
	{
		return;
	}

	if (UTexture2D* FaceTexture = Cast<UTexture2D>(FaceTexturePath.TryLoad()))
	{
		FaceBillboard->SetSprite(FaceTexture);
	}
}

void AZhoenusTalkingHeadAssistant::FacePlayerCamera()
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager == nullptr)
	{
		return;
	}

	const FVector ToCamera = CameraManager->GetCameraLocation() - GetActorLocation();
	if (ToCamera.IsNearlyZero())
	{
		return;
	}

	FRotator FacingRotation = ToCamera.Rotation();
	FacingRotation.Pitch = 0.f;
	FacingRotation.Roll = 0.f;
	SetActorRotation(FacingRotation);
}

void AZhoenusTalkingHeadAssistant::UpdateSpeakingPulse(float DeltaSeconds)
{
	if (!bSpeaking || FaceBillboard == nullptr)
	{
		return;
	}

	SpeakingPulseTime += DeltaSeconds;
	const float Pulse = 1.f + FMath::Sin(SpeakingPulseTime * 16.f) * 0.045f;
	FaceBillboard->SetRelativeScale3D(FVector(FaceScale * Pulse));

	if (GlowLight != nullptr)
	{
		GlowLight->SetIntensity(700.f + 180.f * FMath::Abs(FMath::Sin(SpeakingPulseTime * 10.f)));
	}
}
