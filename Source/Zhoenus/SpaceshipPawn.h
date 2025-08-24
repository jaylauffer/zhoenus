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
    
    // === Attitude stabilizer (body-space PD) ===
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float Stab_Kp = 6.0f;          // deg torque per deg error (after inertia scaling)
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float Stab_Kd = 0.6f;          // deg torque per deg/s
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float StabDeadzone = 0.05f;    // input neutral threshold
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    bool  bStabilizeYaw = false;   // hold heading or leave yaw free
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float StabMaxTorque = 200000.f;// safety clamp (tune for your mesh)

    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float ActiveAngularDamping = 0.2f;
    UPROPERTY(EditAnywhere, Category="Stabilizer")
    float IdleAngularDamping   = 8.0f;

    // === Linear drift taming (side/up bleed when idle) ===
    //UPROPERTY(EditAnywhere, Category="Stabilizer|Linear")
    //float LinBleedRate = 2.0f;     // FInterpTo speed (per second) for Y/Z velocity → 0
    UPROPERTY(EditAnywhere, Category="Stabilizer|Linear")
    float LinDeadzone  = 2.0f;     // ignore tiny cm/s noise

    // --- “Arcade rate” control ---
    UPROPERTY(EditAnywhere, Category="Arcade")
    float MaxPitchRateDeg = 180.f;
    UPROPERTY(EditAnywhere, Category="Arcade")
    float MaxYawRateDeg   = 220.f;
    UPROPERTY(EditAnywhere, Category="Arcade")
    float MaxRollRateDeg  = 260.f;

    // PD on angular *rate* (omega) → snappy response
    UPROPERTY(EditAnywhere, Category="Arcade")
    float Rate_Kp = 8.0f;      // how fast to chase target rate
    UPROPERTY(EditAnywhere, Category="Arcade")
    float Rate_Kd = 0.2f;      // damps overshoot/oscillation
    UPROPERTY(EditAnywhere, Category="Arcade")
    float MaxCtrlTorque = 250000.f; // safety clamp

    // Forward speed hold (arcade thrust)
    UPROPERTY(EditAnywhere, Category="Arcade")
    float MaxForwardSpeed = 3000.f; // hard cap
    UPROPERTY(EditAnywhere, Category="Arcade")
    float Speed_Kp = 8000.f;  // N per (cm/s) error (tune for your mass)
    UPROPERTY(EditAnywhere, Category="Arcade")
    float Speed_Kd = 50.f;    // N per (cm/s^2) (optional)

    // Input shaping (makes center feel crisp)
    UPROPERTY(EditAnywhere, Category="Arcade")
    float Expo = 0.25f; // 0..0.5; raises small inputs
    
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
