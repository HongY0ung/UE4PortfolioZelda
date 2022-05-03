// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Climbing UMETA(DisplayName = "Climbing"),
	EMS_Gliding UMETA(DisplayName = "Gliding"),
	EMS_ClimbUp UMETA(DisplayName = "Climb Up"),
	EMS_ClimbDown UMETA(DisplayName = "Climb Down"),
	EMS_WallJumping UMETA(DisplayName = "Wall Jumping"),
	EMS_HaltClimbing UMETA(DisplayName = "Halt Climbing"),
	EMS_FrontFlip UMETA(DisplayName = "Front Flip"),
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_Exhausted UMETA(DispayName = "Exhausted"),
	ESS_Sprinting UMETA(DispayName = "Sprinting"),
	ESS_Climbing UMETA(DispayName = "Climbing"),
	ESS_Gliding UMETA(DispayName = "Gliding"),
	ESS_Pause UMETA(DispayName = "Pause"),
	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EClimbStatus : uint8
{
	ECS_NormalClimb UMETA(DisplayName = "Normal Climb"),
	ECS_DashJump UMETA(DispayName = "Dash Jump"),
	ECS_TurnCorner UMETA(DispayName = "Turn Corner"),
	ESS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class SECOND_API AMain : public ACharacter
{
	GENERATED_BODY()

private:
	EMovementStatus MovementStatus;
	EStaminaStatus StaminaStatus;
	EClimbStatus ClimbStatus;

public:
	// Sets default values for this character's properties
	AMain();

	/* Camera Settings */

	/** Camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rate to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float LookUpValue;


	/* Normal */
	/** Jump */
	bool bIsJumping;

	/* Climbing Variable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Climbing)
	float ClimbingStaminaConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Climbing)
	float InitClimbStartTerm;
	float ClimbStartTerm;

	bool bTooFarFromWall;
	bool bIsRightEdge;
	bool bIsLeftEdge;
	bool bIsBottomEdge;
	bool bIsFoothold;
	bool bIsGround;
	bool bIsTopEdge;
	bool bIsCanGrab;

	bool bIsBodyWallFacing;
	FVector NormalVectorBodyWallFacing;

	/** Climbing::Turn Corner */
	bool bIsTurnCornerRightEdge;
	bool bIsTurnCornerLeftEdge;

	/*** Inside Corner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Turn Corner")
	UAnimMontage* TurnCornerInsideRightAnimMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Turn Corner")
	UAnimMontage* TurnCornerInsideLeftAnimMontage;

	bool bCanRightTurnInsideCorner;
	bool bCanLeftTurnInsideCorner;

	/*** Outside Corner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Turn Corner")
	UAnimMontage* TurnCornerOutsideRightAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Turn Corner")
	UAnimMontage* TurnCornerOutsideLeftAnimMontage;
	
	bool bCanRightTurnOutsideCorner;
	bool bCanLeftTurnOutsideCorner;

	/** Climbing::Dash Jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_U_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_D_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_R_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_L_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_UR_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_DR_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_UL_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	UAnimMontage* ClimbingDash_DL_AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Dash Jump")
	float ClimbDashStaminaConsumption;	

	float ClimbDashInclination;

	bool bIsLeftDashing;
	bool bIsRightDashing;

	/* Climb Up Variable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Up")
	UAnimMontage* ClimbUpAnimMontage;

	bool bClimbUpEnoughSpace;
	bool bClimbUpTraceGround;

	/* Climb Down Variable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb Down")
	UAnimMontage* GrabWallFromTopAnimMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climb Down") // using UPROPERTY for show message to player (ex: press F key for Climb Down)
	bool bCanGrabWallFromTop;

	FVector NormalVectorGrabWallFromTop;

	/* Sprinting Variable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sprinting")
	float SprintStaminaConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sprinting")
	float InitSprintJumpStaminaConsumDuration;
	float SprintJumpStaminaConsumDuration;
	
	bool bIsStaminaConsumSprinting;
	
	

	/* Front Flip Variable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Front Flip")
	UAnimMontage* FrontFlipAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Front Flip")
	float FrontFlipStaminaConsumption;

	bool bIsLowerRightEdge;
	bool bIsLowerLeftEdge;

	/* Wall Jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	UAnimMontage* WallJumpAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Jump")
	float WallJumpStaminaConsumption;

	/* Gliding */
	UPROPERTY(EditAnywhere, Category = "Gliding")
	class USkeletalMeshComponent* GliderMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	UAnimMontage* StartGlidingAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gliding")
	float GlidingStaminaConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = "Gliding")
	float InitGlidingCoolTime;
	float GlidingCoolTime;
	

	/* Stamina Variable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float InitStaminaRecoverTerm;
	float StaminaRecoverTerm;
	


	/* Only for Stamina Bar variable*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina Bar") // Using UPROPERTY for Stamina Bar widget
	float StaminaConsumption;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina Bar") // Using UPROPERTY for Stamina Bar widget
	float FadedStamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina Bar") // Using UPROPERTY for Stamina Bar widget
	float FadedStaminaDiminishTerm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina Bar") // Using UPROPERTY for Stamina Bar widget
	bool bIsStaminaFilledFull;

	/* Player Stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;


	// Input
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MoveForwardInputValue;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float MoveRightInputValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	bool bIsLeftShiftKeyDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	bool bIsSpacebarDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	bool bIsQKeyDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	bool bIsFKeyDown;

	// for Debugging keyboard input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bIsWKeyDown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bIsAKeyDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bIsSKeyDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bIsDKeyDown;

	// for debugging
	float AngleDegree;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugLine;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	virtual void Jump() override;

	void MovementStatusManager(float DeltaTime);
	void StaminaStatusManager(float DeltaTime);
	void CurrentStaminaManager(float DeltaTime);
	void StaminaBarManager(float DeltaTime);


	/* Camera */
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	/* Status */
	UFUNCTION(BlueprintCallable)
	EStaminaStatus GetStaminaStatus();

	UFUNCTION(BlueprintCallable)
	void SetStaminaStatus(EStaminaStatus Status);

	UFUNCTION(BlueprintCallable)
	EMovementStatus GetMovementStatus();

	UFUNCTION(BlueprintCallable)
	void SetMovementStatus(EMovementStatus Status);

	UFUNCTION(BlueprintCallable)
	EClimbStatus GetClimbStatus();

	UFUNCTION(BlueprintCallable)
	void SetClimbStatus(EClimbStatus Status);

	/* Climbing */
	void StartClimb();
	void StopClimb();

	bool ClimbStartEnoughSpaceConditionForGround();
	bool ClimbStartInputDirectionCondition();
	bool ClimbStartEnoughSpaceCondition();
	bool ClimbMaintainCondition();
	
	void SetIsFoothold();
	void SetIsTopEdge();
	void SetIsBottomEdge();
	void SetIsRightLeftEdgeAtNormal();
	void SetIsRightLeftEdgeAtClimbing();
	void SetIsGround();

	void SetTooFarFromWall();
	void SetIsBodyWallFacingAndNormalVector();

	/* Climbing::Turn Corner */
	// Inside
	void TurnCornerInsideRight();
	void TurnCornerInsideLeft();
	bool TurnCornerInsideRightCondition();
	bool TurnCornerInsideLeftCondition();
	void SetCanRightTurnInsideCorner();
	void SetCanLeftTurnInsideCorner();
	// Outside
	void TurnCornerOutsideRight();
	void TurnCornerOutsideLeft();
	bool TurnCornerOutsideRightCondition();
	bool TurnCornerOutsideLeftCondition();
	void SetCanRightTurnOutsideCorner();
	void SetCanLeftTurnOutsideCorner();

	/* Climbing::Dash Jump*/

	void ClimbDashJump();
	bool ClimbDashJumpCondition();
	bool ClimbDashCondition_U();
	bool ClimbDashCondition_D();
	bool ClimbDashCondition_R();
	bool ClimbDashCondition_L();
	bool ClimbDashCondition_UR();
	bool ClimbDashCondition_DR();
	bool ClimbDashCondition_UL();
	bool ClimbDashCondition_DL();

	void ClimbDashJumpStaminaManage();

	/* Climb Up */
	void ClimbUp();
	bool ClimbUpCondition();
	void SetClimbUpEnoughSpace();
	void SetClimbUpTraceGround();

	/* Climb Down */
	void GrabWallFromTop();
	bool GrabWallFromTopCondition();
	void SetCanGrabWallFromTopAndNormalVector();

	/* Front Flip */
	void FrontFlip();
	bool FrontFlipCondition();
	void SetIsLowerRightLeftEdgeAtGround();

	void FrontFlipStaminaManage();

	/* Wall Jump */
	void WallJump();
	bool WallJumpCondition();
	void WallJumpStaminaManage();

	/* Glidinig */
	void StartGliding();
	void StopGliding();
	bool EnoughSpaceForGliding();
	bool GlidingCondition();

	/* input */
	void SpaceBarPressed();
	void SpaceBarReleased();
	void LeftShiftPressed();
	void LeftShiftReleased();
	void QKeyPressed();
	void QKeyReleased();
	void FKeyPressed();
	void FKeyReleased();

};

