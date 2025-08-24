// Copyright 2025 Run Rong Games, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpaceshipPawn.generated.h"

struct FInputActionValue;

UCLASS(Config=Game)
class ASpaceshipPawn : public APawn
{
	GENERATED_BODY()

public:
	ASpaceshipPawn(const FObjectInitializer &initializer);

	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	// Input action for jumping
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ThrustAction;

	// Input action for moving forward
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PitchAction;

	// Input action for moving forward
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* YawAction;

	// Input action for moving right
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RollAction;

	// Input action for stabilizing the ship
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StabilizeAction;

	/** Bound to the thrust axis */
	void OrigThrustInput(float Val);
	void ThrustInput(const FInputActionValue& Value);
	void PitchInput(const FInputActionValue& Value);
	void YawInput(const FInputActionValue& Value);
	void RollInput(const FInputActionValue& Value);
	void StabilizeInput(const FInputActionValue& Value);

	/** Bound to the vertical axis */
	void MoveUpInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	void RotateRightInput(float Val);

	void OrigDisengageAutoCorrect(float Val);

	void FireWeapon(float Val);

	UPROPERTY(Category=Team, EditAnywhere)
	int32 Team;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
        bool IsDonutTarget;

        //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
        TSet<TWeakObjectPtr<class ADonutFlyerPawn>> Followers;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        class UInputMappingContext* ShipInputMappingContext;
    
    // Virtual mouse-stick params/state
    UPROPERTY(EditAnywhere, Category="Input|MouseStick")
    float MouseStickSensitivity = 0.003f;    // px → axis units

    UPROPERTY(EditAnywhere, Category="Input|MouseStick")
    float MouseStickDecay = 0.05f;            // 0 = sticky; >0 = spring to 0

    // Map to your existing CachedInput: X=pitch, Y=yaw
    // (We reuse CachedInput so Tick physics keeps working)

    // Enhanced Input actions (assign in BP or Defaults)
    UPROPERTY(EditDefaultsOnly, Category="Input|MouseStick")
    UInputAction* MouseXAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Input|MouseStick")
    UInputAction* MouseYAction = nullptr;

    UFUNCTION()
    void OnMouseX(const FInputActionValue& Value);

    UFUNCTION()
    void OnMouseY(const FInputActionValue& Value);
    
    // Rotation rates (deg/s) at full stick
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float MaxPitchRateDeg = 180.f;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float MaxYawRateDeg   = 220.f;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float MaxRollRateDeg  = 260.f;

    // PD gains on angular rate (body space)
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float Rate_Kp = 10.0f;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float Rate_Kd = 0.25f;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float MaxCtrlTorque = 250000.f;

    // Optional input shaping and axis flips
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float Expo = 0.25f; // 0..0.5, makes small inputs more precise
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    bool bInvertMousePitch = true; // common for “flight” feel
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    bool bInvertYaw = false;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    bool bInvertRoll = false;

    // Linear drift taming
    UPROPERTY(EditAnywhere, Category="ArcadeLin")
    float LinBleedRate = 3.0f;   // how quickly side/up velocity damps (1/s)
    UPROPERTY(EditAnywhere, Category="ArcadeLin")
    float LinDeadzone  = 2.0f;   // cm/s

    // Angular damping for feel
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float DampingWhenPiloting = 0.1f;
    UPROPERTY(EditAnywhere, Category="ArcadeRot")
    float DampingWhenIdle     = 2.5f;
    
protected:
	/** StaticMesh component that will be the visuals for our flying pawn */
        UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UStaticMeshComponent* PlaneMesh;

        // Begin APawn overrides
        virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
        // End APawn overrides

private:

        bool bCanFire;
	FVector GunOffset;
	void FireShot();
	void ShotTimerExpired();
	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	/** How quickly pawn can steer */
	UPROPERTY(Category=Plane, EditAnywhere)
	float TurnSpeed;

	UPROPERTY(Category = Plane, EditAnywhere)
	float RollSpeed;

	UPROPERTY(Category = Plane, EditAnywhere)
	float PitchSpeed;
    
    enum class EInputSource
    {
        None,
        Mouse,
        Gamepad
    };

    EInputSource LastInputSource = EInputSource::None;

public:
	FQuat CachedInput;
	FQuat StabilityInput;

	/** How quickly forward speed changes */
	UPROPERTY(Category = Plane, EditAnywhere)
	float Acceleration;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere, BlueprintReadOnly)
		float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category = Yaw, EditAnywhere)
		float MinSpeed;

	float CurrentAcceleration;
	float TargetPitchSpeed;
	float TargetYawSpeed;
	float TargetRollSpeed;

	/** Current forward speed */
	UPROPERTY(BlueprintReadOnly)
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

	float AutoCorrectRate;

	float CurrentRateOfFire;
	
public:
	virtual void BeginPlay() override;

	/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return PlaneMesh; }
};
