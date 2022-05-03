// Fill out your copyright notice in the Description page of Project Settings.

#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/DefaultPawn.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/* Camera */
	// Create Camera Boom (pulls toward the player if theres's a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;  // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Create Follow Camera  
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;
	TurnValue = 0.f;
	LookUpValue = 0.f;
	Coins = 0;

	// Don't rotate when the controller rotates
	// Let that just affect camera.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.f, 0.0f);  // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.0f;

	/* Status */
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	ClimbStatus = EClimbStatus::ECS_NormalClimb;

	/* Normal & Climbing */
	bIsRightEdge = false;
	bIsLeftEdge = false;
	bIsFoothold = false;
	bIsBottomEdge = false;
	bIsTopEdge = false;
	bIsGround = false;

	/* for Start Climb */
	bIsCanGrab = false;
	ClimbStartTerm = 0.15f;
	InitClimbStartTerm = 0.15f;

	/* Climbing */
	bIsBodyWallFacing = false;
	NormalVectorBodyWallFacing = FVector(0.0f, 0.0f, 0.0f);
	bTooFarFromWall = false;

	/* Climbing::Turn Corner */
	bIsTurnCornerRightEdge = false;
	bIsTurnCornerLeftEdge = false;
	bCanRightTurnInsideCorner = false;
	bCanLeftTurnInsideCorner = false;

	/* Climbing::Dash Jump*/
	bIsLeftDashing = false;
	bIsRightDashing = false;
	ClimbDashInclination = 0.0f;


	/* Climb Up*/
	bClimbUpEnoughSpace = false;
	bClimbUpTraceGround = false;

	/* Climb Down*/
	NormalVectorGrabWallFromTop = FVector(0.0f, 0.0f, 0.0f);
	bCanGrabWallFromTop = false;

	/* Front Flip */
	bIsLowerRightEdge = false;
	bIsLowerLeftEdge = false;

	/* Jumping */
	bIsJumping = false;

	/* Gliding */
	GlidingCoolTime = 0.3f;
	InitGlidingCoolTime = 0.3f;

	// Glider Mesh
	GliderMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GliderMeshComponent"));
	GliderMeshComponent->SetupAttachment(GetRootComponent());


	/* for Stamina Status */
	bIsStaminaConsumSprinting = false;
	InitSprintJumpStaminaConsumDuration = 1.0f;
	SprintJumpStaminaConsumDuration = 1.0f;

	/* for current Stamina */
	SprintStaminaConsumption = 20.f;
	ClimbingStaminaConsumption = 5.f;
	GlidingStaminaConsumption = 5.f;
	FrontFlipStaminaConsumption = 10.f;
	ClimbDashStaminaConsumption = 15.f;
	WallJumpStaminaConsumption = 20.f;

	MaxStamina = 150.f;
	CurrentStamina = 150.f;

	InitStaminaRecoverTerm = 0.5f;
	StaminaRecoverTerm = 0.5f;


	/* only for Stamina Bar */
	StaminaConsumption = 0.0f;
	FadedStamina = 0.0f;
	FadedStaminaDiminishTerm = 0.3f;
	bIsStaminaFilledFull = false;

	/* Input */
	MoveForwardInputValue = 0.0f;
	MoveRightInputValue = 0.0f;

	bIsLeftShiftKeyDown = false;
	bIsQKeyDown = false;

	/* input::for Debug */
	bIsSpacebarDown = false;
	bIsFKeyDown = false;
	bIsWKeyDown = false;
	bIsAKeyDown = false;
	bIsSKeyDown = false;
	bIsDKeyDown = false;
	

	/* Debug */
	AngleDegree = 0.f;
	bDrawDebugLine = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();
	GliderMeshComponent->SetVisibility(false);
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Managing MovementStatus Transition
	MovementStatusManager(DeltaTime);

	// Managing StaminaStatus Transition
	StaminaStatusManager(DeltaTime);

	// Managing Current Stamina
	CurrentStaminaManager(DeltaTime);

	// Managing Variable that related to Stamina Bar only
	StaminaBarManager(DeltaTime);

}

void AMain::MovementStatusManager(float DeltaTime)
{
	if (GetMovementStatus() == EMovementStatus::EMS_Climbing)
	{
		if (GetClimbStatus() == EClimbStatus::ECS_NormalClimb && GetMesh()->GetAnimInstance()->Montage_IsPlaying(NULL))
		{
			StopAnimMontage();
		}

		if (GetStaminaStatus() == EStaminaStatus::ESS_Exhausted && GetClimbStatus() == EClimbStatus::ECS_NormalClimb)
		{
			bCanGrabWallFromTop = false;
			StopClimb();
		}
		else
		{
			SetTooFarFromWall();
			SetIsBodyWallFacingAndNormalVector();
			SetIsGround();

			if (bTooFarFromWall)
			{
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, 1.0f);
			}

			if (ClimbMaintainCondition())
			{
				if (!(GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
				{
					SetIsBottomEdge();
					SetIsLowerRightLeftEdgeAtGround();
					SetIsTopEdge();
					SetClimbUpEnoughSpace();
					if (ClimbUpCondition())
					{
						ClimbUp();
					}
					else
					{
						SetIsRightLeftEdgeAtClimbing();
						SetCanLeftTurnInsideCorner();
						SetCanRightTurnInsideCorner();
						SetCanLeftTurnOutsideCorner();
						SetCanRightTurnOutsideCorner();

						// Turn Corner
						if (TurnCornerInsideRightCondition())
						{
							TurnCornerInsideRight();
						}
						else if (TurnCornerInsideLeftCondition())
						{
							TurnCornerInsideLeft();
						}
						else if (TurnCornerOutsideRightCondition())
						{
							TurnCornerOutsideRight();
						}
						else if (TurnCornerOutsideLeftCondition())
						{
							TurnCornerOutsideLeft();
						}
					}

				}
			}
			else
			{
				StopClimb();
			}
		}


		if (GlidingCoolTime > 0)
		{
			GlidingCoolTime -= DeltaTime;
		}

		if (bIsJumping)
		{
			bIsJumping = false;
		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_HaltClimbing)
	{
		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
		{
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_ClimbUp)
	{
		SetClimbUpTraceGround();
		SetIsBodyWallFacingAndNormalVector();
		if (bClimbUpTraceGround && !bIsBodyWallFacing)
		{
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
			const FVector Direction = -GetActorUpVector();
			FVector Position = GetActorLocation();
			Position.Z = Position.Z - 1.f;
			SetActorLocation(Position);
			SetMovementStatus(EMovementStatus::EMS_Normal);

		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_WallJumping)
	{
		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
		{
			StopAnimMontage();
			SetMovementStatus(EMovementStatus::EMS_Normal);
			bIsCanGrab = false;
		}
		else
		{
			if (!(GetStaminaStatus() == EStaminaStatus::ESS_Exhausted))
			{
				SetIsRightLeftEdgeAtClimbing();
				SetIsFoothold();
				SetIsTopEdge();
				SetIsBodyWallFacingAndNormalVector();
				if (bIsCanGrab && (ClimbStartEnoughSpaceCondition()))
				{
					StartClimb();
				}
			}
		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_Gliding)
	{
		if (GetStaminaStatus() != EStaminaStatus::ESS_Exhausted)
		{
			if (EnoughSpaceForGliding())
			{

				if (this->GetVelocity().Z < -200.f)
				{
					GetCharacterMovement()->GravityScale = 0.0f;
					GetMovementComponent()->Velocity.Z = -200.f;
				}
				else
				{
					GetCharacterMovement()->GravityScale = 0.3f;
				}

				SetIsRightLeftEdgeAtNormal();
				SetIsFoothold();
				SetIsBodyWallFacingAndNormalVector();
				SetIsTopEdge();

				if (ClimbStartEnoughSpaceCondition())
				{
					if (ClimbStartInputDirectionCondition())
					{
						ClimbStartTerm -= DeltaTime;
						//UE_LOG(LogTemp, Warning, TEXT("I'm in air~"));
					}
					else
					{
						ClimbStartTerm = InitClimbStartTerm;
					}

					if (ClimbStartTerm <= 0.f)
					{
						StopGliding();
						StartClimb();
					}
				}
			}
			else
			{
				StopGliding();
			}
		}
		else
		{
			StopGliding();
		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_ClimbDown)
	{
		const FVector Direction = -GetActorUpVector();
		FVector Position = GetActorLocation();
		Position.Z = Position.Z - 0.1f;
		SetActorLocation(Position);
		if (bIsCanGrab)
		{
			SetIsRightLeftEdgeAtNormal();
			SetIsFoothold();
			SetIsTopEdge();
			SetIsBodyWallFacingAndNormalVector();

			if (ClimbStartEnoughSpaceCondition())
			{
				StartClimb();
				bIsCanGrab = false;
			}
		}
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_FrontFlip)
	{

	}
	else
	{

		if (GetStaminaStatus() == EStaminaStatus::ESS_Exhausted)
		{
			GetCharacterMovement()->MaxWalkSpeed = 100.f;
		}
		else
		{
			if (GetMovementStatus() == EMovementStatus::EMS_Sprinting)
			{
				GetCharacterMovement()->MaxWalkSpeed = 1000.f;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = 500.f;
			}

			SetIsRightLeftEdgeAtNormal();
			SetIsLowerRightLeftEdgeAtGround();
			SetIsFoothold();
			SetIsTopEdge();
			SetIsBodyWallFacingAndNormalVector();
			SetCanGrabWallFromTopAndNormalVector();

			if (ClimbStartEnoughSpaceConditionForGround())
			{
				if (ClimbStartInputDirectionCondition())
				{
					ClimbStartTerm -= DeltaTime;
					//UE_LOG(LogTemp, Warning, TEXT("I'm in air~"));
				}
				else
				{
					ClimbStartTerm = InitClimbStartTerm;
				}

				if ((GetMovementStatus() == EMovementStatus::EMS_ClimbDown) || (ClimbStartTerm <= 0.0))
				{
					StartClimb();
				}

			}

		}

		if (GlidingCoolTime > 0)
		{
			GlidingCoolTime -= DeltaTime;
		}

		if (bIsJumping)
		{
			if (!GetCharacterMovement()->IsFalling())
			{
				bIsJumping = false;
			}
		}
	}

}

void AMain::StaminaStatusManager(float DeltaTime)
{
	// Managing Stamina Status by referring to Movement and Stamina Status

	if ((CurrentStamina <= 0.0f) &&
		(GetClimbStatus() == EClimbStatus::ECS_NormalClimb) &&
		(GetMovementStatus() != EMovementStatus::EMS_ClimbUp) &&
		(GetMovementStatus() != EMovementStatus::EMS_WallJumping) &&
		(GetMovementStatus() != EMovementStatus::EMS_FrontFlip))
	{
		CurrentStamina = 0.0f;
		StaminaConsumption = 0.0f;

		SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Exhausted)
	{
		if (CurrentStamina == MaxStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
		}
	}
	else if (bIsStaminaConsumSprinting && (GetStaminaStatus() != EStaminaStatus::ESS_Exhausted) && GetMovementStatus() != EMovementStatus::EMS_FrontFlip)
	{
		SetStaminaStatus(EStaminaStatus::ESS_Sprinting);

	}
	else if (GetMovementStatus() == EMovementStatus::EMS_Climbing)
	{
		SetStaminaStatus(EStaminaStatus::ESS_Climbing);
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_Gliding)
	{
		SetStaminaStatus(EStaminaStatus::ESS_Gliding);
	}
	else if (!this->GetMovementComponent()->IsFalling() && GetMovementStatus() == EMovementStatus::EMS_Normal)
	{
		SetStaminaStatus(EStaminaStatus::ESS_Normal);
	}
	else if (
		(this->GetMovementComponent()->IsFalling() && GetMovementStatus() == EMovementStatus::EMS_Normal) ||
		(GetMovementStatus() == EMovementStatus::EMS_ClimbUp) ||
		(GetMovementStatus() == EMovementStatus::EMS_HaltClimbing) ||
		(GetMovementStatus() == EMovementStatus::EMS_WallJumping) ||
		(GetMovementStatus() == EMovementStatus::EMS_ClimbDown) ||
		(GetMovementStatus() == EMovementStatus::EMS_FrontFlip)
		)
	{
		SetStaminaStatus(EStaminaStatus::ESS_Pause);
		if (GetMovementStatus() == EMovementStatus::EMS_FrontFlip)
		{
			bIsStaminaFilledFull = false;
		}
	}


	if (GetStaminaStatus() == EStaminaStatus::ESS_Sprinting && GetCharacterMovement()->Velocity.Size() > 300.f)
	{
		if (this->GetMovementComponent()->IsFalling() && bIsJumping && !(GetMovementStatus() == EMovementStatus::EMS_Gliding))
		{
			SprintJumpStaminaConsumDuration -= DeltaTime;
			if (SprintJumpStaminaConsumDuration <= 0.f)
			{
				bIsStaminaConsumSprinting = false;
				SprintJumpStaminaConsumDuration = InitSprintJumpStaminaConsumDuration;
			}
		}
		else if (this->GetMovementComponent()->IsFalling() && !bIsJumping)
		{
			SprintJumpStaminaConsumDuration = 0.f;
			bIsStaminaConsumSprinting = false;

		}
		else
		{
			bIsJumping = false;
			SprintJumpStaminaConsumDuration = InitSprintJumpStaminaConsumDuration;
		}
	}



}

void AMain::CurrentStaminaManager(float DeltaTime)
{
	// Managing Current Stamina by referring to Movement and Stamina Status
	if (GetStaminaStatus() == EStaminaStatus::ESS_Sprinting && GetCharacterMovement()->Velocity.Size() > 300.f)
	{
		CurrentStamina -= DeltaTime * SprintStaminaConsumption;
		StaminaRecoverTerm = InitStaminaRecoverTerm;
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Climbing)
	{
		StaminaRecoverTerm = InitStaminaRecoverTerm;

		if (MoveForwardInputValue != 0.f || MoveRightInputValue != 0.f || (GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
		{
			if (!(MoveRightInputValue > 0 && MoveForwardInputValue == 0 && bIsRightEdge)
				&& !(MoveRightInputValue < 0 && MoveForwardInputValue == 0 && bIsLeftEdge)
				&& !(MoveForwardInputValue > 0 && MoveRightInputValue == 0 && bIsTopEdge)
				&& !(MoveForwardInputValue < 0 && MoveRightInputValue == 0 && bIsBottomEdge))
			{
				if (GetClimbStatus() != EClimbStatus::ECS_DashJump)
				{
					CurrentStamina -= DeltaTime * ClimbingStaminaConsumption;
				}
				else
				{

				}
			}
			else if (GetClimbStatus() == EClimbStatus::ECS_TurnCorner)
			{
				CurrentStamina -= DeltaTime * ClimbingStaminaConsumption;
			}
			else
			{
	
			}

		}
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Gliding)
	{
		StaminaRecoverTerm = InitStaminaRecoverTerm;
		CurrentStamina -= DeltaTime * GlidingStaminaConsumption;
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Pause)
	{
		StaminaRecoverTerm = InitStaminaRecoverTerm;

	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Normal)
	{

		StaminaRecoverTerm -= DeltaTime;
		if (CurrentStamina < MaxStamina && StaminaRecoverTerm <= 0.f)
		{
			StaminaRecoverTerm = 0.f;
			CurrentStamina += DeltaTime * 40;
			if (CurrentStamina >= MaxStamina)
			{
				CurrentStamina = MaxStamina;
			}
		}
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Exhausted && GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{

		StaminaRecoverTerm -= DeltaTime;
		if (CurrentStamina < MaxStamina && StaminaRecoverTerm <= 0.f)
		{
			StaminaRecoverTerm = 0.f;
			CurrentStamina += DeltaTime * 40;
			if (CurrentStamina >= MaxStamina)
			{
				CurrentStamina = MaxStamina;
			}
		}
	}
	else
	{

	}
}

void AMain::StaminaBarManager(float DeltaTime)
{
	// Managing stamina bar variable by referring to Movement and Stamina Status

	// Managing StaminaConsumption
	if (GetStaminaStatus() == EStaminaStatus::ESS_Sprinting && GetCharacterMovement()->Velocity.Size() > 300.f)
	{
		StaminaConsumption = SprintStaminaConsumption / MaxStamina * 0.5f;

		if (StaminaConsumption > (CurrentStamina / MaxStamina))
		{
			StaminaConsumption = (CurrentStamina / MaxStamina);
		}
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Climbing)
	{
		if (MoveForwardInputValue != 0.f || MoveRightInputValue != 0.f || (GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
		{
			if (!(MoveRightInputValue > 0 && MoveForwardInputValue == 0 && bIsRightEdge)
				&& !(MoveRightInputValue < 0 && MoveForwardInputValue == 0 && bIsLeftEdge)
				&& !(MoveForwardInputValue > 0 && MoveRightInputValue == 0 && bIsTopEdge)
				&& !(MoveForwardInputValue < 0 && MoveRightInputValue == 0 && bIsBottomEdge))
			{
				if (GetClimbStatus() != EClimbStatus::ECS_DashJump)
				{
					StaminaConsumption = ClimbingStaminaConsumption / MaxStamina * 0.5f;
					if (StaminaConsumption > (CurrentStamina / MaxStamina))
					{
						StaminaConsumption = (CurrentStamina / MaxStamina);
					}
				}
				else
				{
					StaminaConsumption = 0.f;
				}
			}
			else if (GetClimbStatus() == EClimbStatus::ECS_TurnCorner)
			{
				StaminaConsumption = ClimbingStaminaConsumption / MaxStamina * 0.5f;
				if (StaminaConsumption > (CurrentStamina / MaxStamina))
				{
					StaminaConsumption = (CurrentStamina / MaxStamina);
				}
			}
			else
			{
				StaminaConsumption = 0.f;
			}

		}

		if (MoveForwardInputValue == 0.f && MoveRightInputValue == 0.f && !(GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
		{
			StaminaConsumption = 0.f;
		}


	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Gliding)
	{
		StaminaConsumption = GlidingStaminaConsumption / MaxStamina * 0.5f;
		if (StaminaConsumption > (CurrentStamina / MaxStamina))
		{
			StaminaConsumption = (CurrentStamina / MaxStamina);
		}
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Pause)
	{
		StaminaConsumption = 0.f;
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Normal)
	{
		StaminaConsumption = 0.f;
	}
	else if (GetStaminaStatus() == EStaminaStatus::ESS_Exhausted && GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling)
	{
		StaminaConsumption = 0.f;
	}
	else
	{
		StaminaConsumption = 0.f;
	}

	// Managing Faded Stamina
	if (FadedStamina > 0.f)
	{
		if (FadedStaminaDiminishTerm <= 0.f)
		{
			FadedStamina -= DeltaTime / 2.f;
		}
		else
		{
			FadedStaminaDiminishTerm -= DeltaTime;
		}

	}
	else
	{
		FadedStamina = 0.f;
		FadedStaminaDiminishTerm = 0.3f;
	}

	// Stamina Full Charge Check
	if (CurrentStamina >= MaxStamina)
	{
		bIsStaminaFilledFull = true;
	}
	else
	{
		bIsStaminaFilledFull = false;
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpaceBar", IE_Pressed, this, &AMain::SpaceBarPressed);
	PlayerInputComponent->BindAction("SpaceBar", IE_Released, this, &AMain::SpaceBarReleased);

	PlayerInputComponent->BindAction("LeftShift", IE_Pressed, this, &AMain::LeftShiftPressed);
	PlayerInputComponent->BindAction("LeftShift", IE_Released, this, &AMain::LeftShiftReleased);

	PlayerInputComponent->BindAction("Q_Keyboard", IE_Pressed, this, &AMain::QKeyPressed);
	PlayerInputComponent->BindAction("Q_Keyboard", IE_Released, this, &AMain::QKeyReleased);
	
	PlayerInputComponent->BindAction("F_Keyboard", IE_Pressed, this, &AMain::FKeyPressed);
	PlayerInputComponent->BindAction("F_Keyboard", IE_Released, this, &AMain::FKeyReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

void AMain::Jump()
{
	Super::Jump();
	bIsJumping = true;
}

void AMain::MoveForward(float Value)
{
	if (Value < 0.1f && Value > -0.1f)
	{
		Value = 0.f;
	}

	MoveForwardInputValue = Value;
	if (GetMovementStatus() == EMovementStatus::EMS_Sprinting)
	{
		SetMovementStatus(EMovementStatus::EMS_Normal);
	}

	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
	{
		
		if (GetMovementStatus() == EMovementStatus::EMS_Climbing)
		{
			bIsStaminaConsumSprinting = false;
			if (bIsQKeyDown)
			{
				if (Value != 0)
				{
					MoveForwardInputValue = 0.0f;
				}
				Value = 0.0f;
				
			}

			if (bIsTopEdge)
			{
				if (Value > 0)
				{
					Value = 0;
					MoveForwardInputValue = 0.0f;
				}

			}

			if (bIsBottomEdge)
			{
				if (Value < 0)
				{
					Value = 0;
				}
			}

			const FVector Direction = GetActorUpVector();
			AddMovementInput(Direction, Value);
		}
		else
		{

		}
		
	}
	else
	{
		if (!this->GetMovementComponent()->IsFalling())
		{

			if (!(GetMovementStatus() == EMovementStatus::EMS_Sprinting))
			{
				bIsStaminaConsumSprinting = false;
			}

			if ((Controller != nullptr) && (Value != 0.0f))
			{
				// find out which way is forward
				if (bIsLeftShiftKeyDown && !(GetStaminaStatus() == EStaminaStatus::ESS_Exhausted))
				{
					if (Value >= 0.3f)
					{
						Value = 1.f;
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
						bIsStaminaConsumSprinting = true;
					}
					else if (Value <= -0.3f)
					{
						Value = -1.f;
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
						bIsStaminaConsumSprinting = true;
					}
					else if (MoveRightInputValue <= 0.3f && MoveRightInputValue >= -0.3f)
					{
						Value = 0;
						SetMovementStatus(EMovementStatus::EMS_Normal);
						bIsStaminaConsumSprinting = false;
					}
				}

				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRoatation(0.f, Rotation.Yaw, 0.f);

				const FVector Direction = FRotationMatrix(YawRoatation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);
			}
		}
		else
		{
			if (GetMovementStatus() == EMovementStatus::EMS_Gliding)
			{
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRoatation(0.f, Rotation.Yaw, 0.f);

				const FVector Direction = FRotationMatrix(YawRoatation).GetUnitAxis(EAxis::X);
				AddMovementInput(Direction, Value);
			}
		}
		
	}
}




void AMain::MoveRight(float Value)
{
	if (Value < 0.1f && Value > -0.1f)
	{
		Value = 0.f;
	}
	

	MoveRightInputValue = Value;
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
	{
		if (GetMovementStatus() == EMovementStatus::EMS_Climbing)
		{
			bIsStaminaConsumSprinting = false;
			if (bIsQKeyDown)
			{
				Value = 0.0f;
				MoveRightInputValue = 0.0f;
			}

			if (bIsRightEdge)
			{
				if (Value > 0.0f)
				{
					Value = 0.0f;
				}
			}

			if (bIsLeftEdge)
			{
				if (Value < 0.0f)
				{
					Value = 0.0f;
				}
			}

			const FVector Direction = GetActorRightVector();
			AddMovementInput(Direction, Value);
		}
	}
	else
	{
		if (!this->GetMovementComponent()->IsFalling())
		{
			if (!(GetMovementStatus() == EMovementStatus::EMS_Sprinting))
			{
				bIsStaminaConsumSprinting = false;
			}

			if ((Controller != nullptr) && (Value != 0.0f))
			{
				
				if (bIsLeftShiftKeyDown && !(GetStaminaStatus() == EStaminaStatus::ESS_Exhausted))
				{
					if (Value >= 0.3f)
					{
						Value = 1.f;
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
						bIsStaminaConsumSprinting = true;
					}
					else if (Value <= -0.3f)
					{
						Value = -1.f;
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
						bIsStaminaConsumSprinting = true;
					}
					else if (MoveForwardInputValue <= 0.3f && MoveForwardInputValue >= -0.3f)
					{
						Value = 0;
						SetMovementStatus(EMovementStatus::EMS_Normal);
						bIsStaminaConsumSprinting = false;
					}
				}

				// find out which way is forward
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRoatation(0.f, Rotation.Yaw, 0.f);

				const FVector Direction = FRotationMatrix(YawRoatation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, Value);
			}
		}
		else
		{
			if (GetMovementStatus() == EMovementStatus::EMS_Gliding)
			{
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRoatation(0.f, Rotation.Yaw, 0.f);

				const FVector Direction = FRotationMatrix(YawRoatation).GetUnitAxis(EAxis::Y);
				AddMovementInput(Direction, Value);
			}
		}
	}

	
}

void AMain::TurnAtRate(float Rate)
{
	if (Rate < 0.1f && Rate > -0.1f)
	{
		Rate = 0.f;
	}

	TurnValue = Rate;

	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AMain::LookUpAtRate(float Rate)
{
	if (Rate < 0.1f && Rate > -0.1f)
	{
		Rate = 0.f;
	}

	LookUpValue = Rate;

	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AMain::EnoughSpaceForGliding()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Enough = true;

	FVector Start = GetActorLocation() - GetActorUpVector() * 100.f;
	FVector End = Start;
	FQuat Rot{};
	FHitResult OutHit{};
	FCollisionShape Shape;
	Shape.SetSphere(40.0f);
	
	Enough = !GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, ECollisionChannel::ECC_Visibility, Shape, CollisionParams);
	return Enough;
}

void AMain::SetIsRightLeftEdgeAtNormal()
{

	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector RightStart = GetActorLocation() + FVector(0.f, 0.f, 60.f) + GetActorRightVector() * 50.f;
	FVector RightEnd = RightStart + GetActorForwardVector() * 100.f;
	
	bIsRightEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, RightStart, RightEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
	
	FVector LeftStart = GetActorLocation() + FVector(0.f, 0.f, 60.f) - GetActorRightVector() * 50.f;
	FVector LeftEnd = LeftStart + GetActorForwardVector() * 100.f;

	bIsLeftEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, LeftStart, LeftEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
}

void AMain::SetIsLowerRightLeftEdgeAtGround()
{

	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector RightStart = GetActorLocation() + FVector(0.f, 0.f, -30.f) + GetActorRightVector() * 50.f;
	FVector RightEnd = RightStart + GetActorForwardVector() * 60.f;

	bIsLowerRightEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, RightStart, RightEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	FVector LeftStart = GetActorLocation() + FVector(0.f, 0.f, -30.f) - GetActorRightVector() * 50.f;
	FVector LeftEnd = LeftStart + GetActorForwardVector() * 60.f;

	bIsLowerLeftEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, LeftStart, LeftEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
}

void AMain::SetIsFoothold()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, -80.f);
	FVector End = Start + GetActorForwardVector() * 70.f;

	bIsFoothold = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
}

void AMain::SetIsTopEdge()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 90.f);
	FVector End = Start + GetActorForwardVector() * 70.f;

	bIsTopEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
}


void AMain::SetCanGrabWallFromTopAndNormalVector()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool bDeepEnoughSpace = false;
	bool bCloserGroundCheck = false;
	
	bool bSpaceCheckRight = false;
	bool bSpaceCheckLeft = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + GetActorForwardVector() * 42.f;
	FVector End = Start + GetActorUpVector() * (-276.f);



	bDeepEnoughSpace = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorForwardVector() * 15.f;
	End = Start + GetActorUpVector() * (-100.f);
	bCloserGroundCheck = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);


	Start = GetActorLocation() + GetActorForwardVector() * 42.f + GetActorUpVector() * (-184.f) + GetActorRightVector() * 42.f;
	End = Start + GetActorForwardVector() * (-35.f);
	bSpaceCheckRight = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorForwardVector() * 42.f + GetActorUpVector() * (-184.f) + GetActorRightVector() * (-42.f);
	End = Start + GetActorForwardVector() * (-35.f);
	bSpaceCheckLeft = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorForwardVector() * 42.f + GetActorUpVector() * (-184.f);
	End = Start + GetActorForwardVector() * (-35.f);
	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (bDeepEnoughSpace && bCloserGroundCheck && bSpaceCheckRight && bSpaceCheckLeft)
	{
		bCanGrabWallFromTop = true;
		NormalVectorGrabWallFromTop = OutHit.Normal;
	}
	else
	{
		bCanGrabWallFromTop = false;
	}

}


void AMain::SetTooFarFromWall()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 45.f;

	bTooFarFromWall = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

}

void AMain::SetClimbUpTraceGround()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorUpVector() * (-130.f);

	bClimbUpTraceGround = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

}

void AMain::SetIsGround()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorUpVector() * (-100.f);

	bIsGround = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

}

void AMain::SetIsBottomEdge()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, -80.f);
	FVector End = Start + GetActorForwardVector() * 80.f;

	bIsBottomEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

}

void AMain::SetCanRightTurnInsideCorner()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Condition_1 = false;
	bool Condition_2 = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + GetActorForwardVector() * 40.f;
	FVector End = Start + GetActorRightVector() * 45.f;

	Condition_1 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorForwardVector() * (-40.f);
	End = Start + GetActorRightVector() * 45.f;

	Condition_2 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (Condition_1 && Condition_2)
	{
		bCanRightTurnInsideCorner = true;
	}
	else
	{
		bCanRightTurnInsideCorner = false;
	}

}

void AMain::SetCanLeftTurnInsideCorner()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Condition_1 = false;
	bool Condition_2 = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + GetActorForwardVector() * 40.f;
	FVector End = Start + GetActorRightVector() * (-45.f);

	Condition_1 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorForwardVector() * (-40.f);
	End = Start + GetActorRightVector() * (-45.f);

	Condition_2 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (Condition_1 && Condition_2)
	{
		bCanLeftTurnInsideCorner = true;
	}
	else
	{
		bCanLeftTurnInsideCorner = false;
	}

}

void AMain::SetIsRightLeftEdgeAtClimbing()
{

	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	FHitResult OutHit{};
	FVector RightStart = GetActorLocation() + GetActorRightVector() * 50.f;
	FVector RightEnd = RightStart + GetActorForwardVector() * 60.f;

	bIsRightEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, RightStart, RightEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

	FVector LeftStart = GetActorLocation() + GetActorRightVector() * (-50.f);
	FVector LeftEnd = LeftStart + GetActorForwardVector() * 60.f;

	bIsLeftEdge = !GetWorld()->LineTraceSingleByChannel(OutHit, LeftStart, LeftEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

}


void AMain::SetCanRightTurnOutsideCorner()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Condition_1 = false;
	bool Condition_2 = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + GetActorRightVector() * 84.f + GetActorForwardVector() * 50.f;
	FVector End = Start + GetActorRightVector() * (-70.f);

	Condition_1 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorRightVector() * 84.f + GetActorForwardVector() * 126.f;
	End = Start + GetActorRightVector() * (-70.f);

	Condition_2 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (Condition_1 && Condition_2)
	{
		bCanRightTurnOutsideCorner = true;
	}
	else
	{
		bCanRightTurnOutsideCorner = false;
	}

}

void AMain::SetCanLeftTurnOutsideCorner()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Condition_1 = false;
	bool Condition_2 = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + GetActorRightVector() * (-84.f) + GetActorForwardVector() * 50.f;
	FVector End = Start + GetActorRightVector() * (70.f);

	Condition_1 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + GetActorRightVector() * (-84.f) + GetActorForwardVector() * 126.f;
	End = Start + GetActorRightVector() * (70.f);

	Condition_2 = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (Condition_1 && Condition_2)
	{
		bCanLeftTurnOutsideCorner = true;
	}
	else
	{
		bCanLeftTurnOutsideCorner = false;
	}

}



void AMain::SetClimbUpEnoughSpace()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool Condition_1 = false;
	bool Condition_2 = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 96.f);
	FVector End = Start + GetActorForwardVector() * 70.f;

	Condition_1 = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	Start = GetActorLocation() + FVector(0.f, 0.f, 300.f);
	End = Start + GetActorForwardVector() * 70.f;
	Condition_2 = !GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (Condition_1 && Condition_2)
	{
		bClimbUpEnoughSpace = true;
	}
	else
	{
		bClimbUpEnoughSpace = false;
	}
}

void AMain::SetIsBodyWallFacingAndNormalVector()
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	if (bDrawDebugLine)
	{
		CollisionParams.TraceTag = TraceTag;
	}

	bool bDeepEnoughSpace = false;
	bool bCloserGroundCheck = false;

	bool bSpaceCheckRight = false;
	bool bSpaceCheckLeft = false;

	FHitResult OutHit{};
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * (70.f);

	bIsBodyWallFacing = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (bIsBodyWallFacing)
	{
		NormalVectorBodyWallFacing = OutHit.Normal;
	}
}

EStaminaStatus AMain::GetStaminaStatus()
{
	return StaminaStatus;
}


void AMain::SetStaminaStatus(EStaminaStatus Status)
{
	StaminaStatus = Status;
}


EMovementStatus AMain::GetMovementStatus()
{
	return MovementStatus;
}


void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
}

EClimbStatus AMain::GetClimbStatus()
{
	return ClimbStatus;
}


void AMain::SetClimbStatus(EClimbStatus Status)
{
	ClimbStatus = Status;
}

void AMain::StopClimb()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetCharacterMovement()->bOrientRotationToMovement = true;
}





void AMain::StartClimb()
{
	StopAnimMontage();
	FRotator MovementRotation = NormalVectorBodyWallFacing.Rotation();
	MovementRotation.Yaw = MovementRotation.Yaw + 180.f;
	SetActorRotation(MovementRotation);
	ClimbStartTerm = InitClimbStartTerm;
	SetMovementStatus(EMovementStatus::EMS_Climbing);
	bIsCanGrab = false;
	GetCharacterMovement()->Velocity = FVector(0.f, 0.f, 0.f);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bIsJumping = false;
}

bool AMain::ClimbStartInputDirectionCondition()
{
	GetActorLocation();
	FVector Vec = FVector(MoveForwardInputValue, MoveRightInputValue, 0.f);

	Vec = UKismetMathLibrary::GetDirectionUnitVector(FVector(0.f, 0.f, 0.f), Vec);
	FRotator Rot = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
	FVector ControlDirection = Rot.RotateVector(Vec);

	FVector ForwardVec = GetActorForwardVector();
	float DotProd = UKismetMathLibrary::Dot_VectorVector(ControlDirection, ForwardVec);
	float degree1 = UKismetMathLibrary::DegAcos(DotProd);
	AngleDegree = degree1;
	DotProd = UKismetMathLibrary::Dot_VectorVector(-NormalVectorBodyWallFacing, ForwardVec);
	float degree2 = UKismetMathLibrary::DegAcos(DotProd);
	AngleDegree = degree2;
	if (degree1 <= 45.f && degree2 <= 30.f)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbMaintainCondition()
{
	if ((bIsBodyWallFacing || !(GetClimbStatus() == EClimbStatus::ECS_NormalClimb)) && !((bIsGround) && (MoveForwardInputValue < 0.0)))
	{
		return true;
	}

	return false;
}

bool AMain::ClimbUpCondition()
{

	if ( 
		( bIsTopEdge && bClimbUpEnoughSpace && !(GetClimbStatus() == EClimbStatus::ECS_DashJump)  || ((GetClimbStatus() == EClimbStatus::ECS_DashJump) && !bIsBottomEdge)) &&
		(!bIsRightEdge || !bIsLeftEdge) && 
		bIsTopEdge && 
		bClimbUpEnoughSpace 
		)
	{
		return true;
	}
	return false;
}

bool AMain::FrontFlipCondition()
{
	if ( bIsRightEdge && bIsLeftEdge & !bIsLowerLeftEdge && !bIsLowerRightEdge && bIsFoothold && bIsTopEdge && !GetCharacterMovement()->IsFalling())
	{
		return true;
	}
	return false;
}

bool AMain::TurnCornerInsideRightCondition()
{
	if (bCanRightTurnInsideCorner && ( ( MoveRightInputValue > 0.0f )  || (GetClimbStatus() == EClimbStatus::ECS_DashJump && bIsRightDashing) ) )
	{
		return true;
	}

	return false;
}

bool AMain::TurnCornerInsideLeftCondition()
{
	if (bCanLeftTurnInsideCorner && ((MoveRightInputValue < 0.0f) || (GetClimbStatus() == EClimbStatus::ECS_DashJump && bIsLeftDashing)))
	{
		return true;
	}

	return false;
}

bool AMain::TurnCornerOutsideRightCondition()
{
	if (bCanRightTurnOutsideCorner && bIsRightEdge && ((MoveRightInputValue > 0.0f) || (GetClimbStatus() == EClimbStatus::ECS_DashJump && bIsRightDashing)))
	{
		return true;
	}

	return false;
}

bool AMain::TurnCornerOutsideLeftCondition()
{
	if (bCanLeftTurnOutsideCorner && bIsLeftEdge && ((MoveRightInputValue < 0.0f) || (GetClimbStatus() == EClimbStatus::ECS_DashJump && bIsLeftDashing)))
	{
		return true;
	}

	

	return false;
}


void AMain::StartGliding()
{
	SetMovementStatus(EMovementStatus::EMS_Gliding);

	// Gliding start macro
	GetCharacterMovement()->GravityScale = 0.2f;
	GetCharacterMovement()->AirControl = 1.0f;
	FVector CurrentVelocity = GetCharacterMovement()->Velocity;
	GetCharacterMovement()->Velocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.f);

	LaunchCharacter(FVector(0.0f, 0.0f, 150.f), false, false);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->BrakingDecelerationFalling = 500.f;
	GetCharacterMovement()->MaxWalkSpeed = 750.f;

	// Play anim montage part
	if (StartGlidingAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(StartGlidingAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}

	bIsJumping = false;
	GliderMeshComponent->SetVisibility(true);

}

void AMain::StopGliding()
{
	bIsJumping = false;
	StopAnimMontage();
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->AirControl = 0.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetCharacterMovement()->BrakingDecelerationFalling = 0.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	GlidingCoolTime = InitGlidingCoolTime;
	
	GliderMeshComponent->SetVisibility(false);
}

void AMain::ClimbDashJump()
{
	ClimbDashJumpStaminaManage();

	if (!(MoveForwardInputValue == 0.f || MoveRightInputValue == 0.f))
	{
		ClimbDashInclination = MoveForwardInputValue / MoveRightInputValue;
	}

	if (ClimbDashCondition_U())
	{
		if (ClimbingDash_U_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_U_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsRightDashing = false;
		bIsLeftDashing = false;
	}
	else if (ClimbDashCondition_D())
	{
		if (ClimbingDash_D_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_D_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsRightDashing = false;
		bIsLeftDashing = false;
	}
	else if (ClimbDashCondition_R())
	{
		if (ClimbingDash_R_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_R_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsRightDashing = true;
	}
	else if (ClimbDashCondition_L())
	{
		if (ClimbingDash_L_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_L_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsLeftDashing = true;
	}
	else if (ClimbDashCondition_UR())
	{
		if (ClimbingDash_UR_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_UR_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsRightDashing = true;
	}
	else if (ClimbDashCondition_DR())
	{
		if (ClimbingDash_DR_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_DR_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsRightDashing = true;
	}
	else if (ClimbDashCondition_UL())
	{
		if (ClimbingDash_UL_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_UL_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsLeftDashing = true;
	}
	else if (ClimbDashCondition_DL())
	{
		if (ClimbingDash_DL_AnimMontage)
		{
			FTimerHandle WaitHandle;
			float WaitTime = PlayAnimMontage(ClimbingDash_DL_AnimMontage);
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{


				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
		}

		bIsLeftDashing = true;
	}
	SetClimbStatus(EClimbStatus::ECS_DashJump);

}

bool AMain::ClimbDashJumpCondition()
{
	if (GetMovementStatus() == EMovementStatus::EMS_Climbing && (GetClimbStatus() == EClimbStatus::ECS_NormalClimb) && !bIsQKeyDown && !(TurnCornerOutsideLeftCondition() && MoveRightInputValue < 0) && !(TurnCornerOutsideRightCondition() && MoveRightInputValue > 0))
	{
		return true;
	}

	return false;
}

void AMain::ClimbDashJumpStaminaManage()
{
	if (CurrentStamina > ClimbDashStaminaConsumption)
	{
		FadedStamina = ClimbDashStaminaConsumption / MaxStamina;
		CurrentStamina -= ClimbDashStaminaConsumption;
	}
	else
	{
		FadedStamina = CurrentStamina / MaxStamina;
		CurrentStamina = 0.f;
	}
	
	
}


bool AMain::ClimbDashCondition_U()
{
	if ((MoveRightInputValue == 0.0f && MoveForwardInputValue == 0.0f) || 
		(MoveForwardInputValue > 0.0f && MoveRightInputValue == 0.0f) ||
		(MoveForwardInputValue > 0.0f && ClimbDashInclination >= 2.414f) ||
		(MoveForwardInputValue > 0.0f && ClimbDashInclination < -2.414f) )
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_D()
{
	if ((MoveForwardInputValue < 0.f && MoveRightInputValue == 0.f) ||
		(MoveForwardInputValue < 0.f && ClimbDashInclination >= 2.414f) ||
		(MoveForwardInputValue < 0.f && ClimbDashInclination < -2.414f))
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_R()
{
	if ((MoveForwardInputValue == 0.f && MoveRightInputValue > 0.f) ||
		(MoveRightInputValue > 0.f && ClimbDashInclination < 0.414f  && ClimbDashInclination > 0.f) ||
		(MoveRightInputValue > 0.f && ClimbDashInclination < 0.f && ClimbDashInclination >= -0.414f))
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_L()
{
	if ((MoveForwardInputValue == 0.f && MoveRightInputValue < 0.f) ||
		(MoveRightInputValue < 0.f && ClimbDashInclination < 0.414f && ClimbDashInclination > 0.f) ||
		(MoveRightInputValue < 0.f && ClimbDashInclination < 0.f && ClimbDashInclination >= -0.414f))
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_UR()
{
	if (MoveForwardInputValue > 0.f && ClimbDashInclination >= 0.414f && ClimbDashInclination < 2.414f)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_DR()
{
	if (MoveForwardInputValue < 0.f && ClimbDashInclination < -0.414f && ClimbDashInclination >= -2.414f)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_UL()
{
	if (MoveForwardInputValue > 0.f && ClimbDashInclination < -0.414f && ClimbDashInclination >= -2.414f)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbDashCondition_DL()
{
	if (MoveForwardInputValue < 0.f && ClimbDashInclination >= 0.414f && ClimbDashInclination < 2.414f)
	{
		return true;
	}

	return false;
}

void AMain::SpaceBarPressed()
{
	bIsSpacebarDown = true;
	if (ClimbDashJumpCondition())
	{
		ClimbDashJump();
	}
	else if (WallJumpCondition())
	{
		WallJump();
	}
	else if (GlidingCondition() && GetMovementStatus() != EMovementStatus::EMS_Gliding && CurrentStamina > 0)
	{
		if (GlidingCoolTime <= 0)
		{
			StartGliding();
		}

	
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_Gliding)
	{
		StopGliding();
	}
	else if (GetMovementStatus() == EMovementStatus::EMS_ClimbUp)
	{

	}
	else if (GetMovementStatus() == EMovementStatus::EMS_ClimbDown)
	{

	}
	else if (GetMovementStatus() == EMovementStatus::EMS_FrontFlip)
	{

	}
	else if (GetMovementStatus() == EMovementStatus::EMS_Climbing && GetClimbStatus() != EClimbStatus::ECS_NormalClimb)
	{

	}
	else 
	{
		SetIsRightLeftEdgeAtNormal();
		SetIsLowerRightLeftEdgeAtGround();
		SetIsFoothold();
		SetIsTopEdge();
		SetIsBodyWallFacingAndNormalVector();
		SetCanGrabWallFromTopAndNormalVector();
		if (FrontFlipCondition())
		{
			FrontFlip();
		}
		else
		{
			Jump();
		}


	}



}

void AMain::SpaceBarReleased()
{

	bIsSpacebarDown = false;
	StopJumping();

}


void AMain::WallJump()
{
	WallJumpStaminaManage();

	if (WallJumpAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(WallJumpAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}

	GetCharacterMovement()->bOrientRotationToMovement = true;
	SetMovementStatus(EMovementStatus::EMS_WallJumping);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	FTimerHandle WaitHandle;
	float WaitTime = 0.3f;
	GetCharacterMovement()->GravityScale = 0.1f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			GetCharacterMovement()->GravityScale = 1.0f;
			bIsCanGrab = true;

		}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능

}

bool AMain::WallJumpCondition()
{
	if (GetMovementStatus() == EMovementStatus::EMS_Climbing && bIsQKeyDown && !(GetClimbStatus() == EClimbStatus::ECS_DashJump) && !(GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
	{
		return true;
	}
	
	return false;
}

void AMain::WallJumpStaminaManage()
{
	if (CurrentStamina > WallJumpStaminaConsumption)
	{
		FadedStamina = WallJumpStaminaConsumption / MaxStamina;
		CurrentStamina -= WallJumpStaminaConsumption;
	}
	else
	{
		FadedStamina = CurrentStamina / MaxStamina;
		CurrentStamina = 0.f;
	}


}

bool AMain::GlidingCondition()
{
	if ((GetMovementStatus() == EMovementStatus::EMS_Normal || 
		GetMovementStatus() == EMovementStatus::EMS_Sprinting || 
		GetMovementStatus() == EMovementStatus::EMS_HaltClimbing || 
		GetMovementStatus() == EMovementStatus::EMS_WallJumping)
		&&
		EnoughSpaceForGliding() &&
		(GetStaminaStatus() != EStaminaStatus::ESS_Exhausted)
		)
	{

		return true;
	}

	return false;
}

void AMain::LeftShiftPressed()
{
	bIsLeftShiftKeyDown = true;
	if (GetMovementStatus() == EMovementStatus::EMS_Climbing && !(GetClimbStatus() == EClimbStatus::ECS_DashJump) && !(GetClimbStatus() == EClimbStatus::ECS_TurnCorner))
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		SetMovementStatus(EMovementStatus::EMS_HaltClimbing);
	}
}

void AMain::LeftShiftReleased()
{
	bIsLeftShiftKeyDown = false;
}

void AMain::QKeyPressed()
{
	bIsQKeyDown = true;
}

void AMain::QKeyReleased()
{
	bIsQKeyDown = false;
}

void AMain::FKeyPressed()
{
	bIsFKeyDown = true;
	if (GrabWallFromTopCondition())
	{
		GrabWallFromTop();
	}
}

void AMain::FKeyReleased()
{
	bIsFKeyDown = false;
}

void AMain::GrabWallFromTop()
{
	bCanGrabWallFromTop = false;
	FRotator Rot = NormalVectorGrabWallFromTop.Rotation();
	SetActorRotation(FRotator(0.f, Rot.Yaw, 0.f));
	SetMovementStatus(EMovementStatus::EMS_ClimbDown);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	if(GrabWallFromTopAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(GrabWallFromTopAnimMontage) - 0.3f;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				bIsCanGrab = true;

			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}
}


bool AMain::GrabWallFromTopCondition()
{
	if (GetMovementStatus() == EMovementStatus::EMS_Normal && !GetCharacterMovement()->IsFalling() && bCanGrabWallFromTop)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbStartEnoughSpaceCondition()
{
	if (!bIsRightEdge && !bIsLeftEdge && !bIsTopEdge && bIsFoothold && bIsBodyWallFacing)
	{
		return true;
	}

	return false;
}

bool AMain::ClimbStartEnoughSpaceConditionForGround()
{
	if (!bIsRightEdge && !bIsLeftEdge && bIsFoothold && bIsBodyWallFacing)
	{
		return true;
	}

	return false;
}

void AMain::FrontFlip()
{
	FrontFlipStaminaManage();
	GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.0f);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	SetMovementStatus(EMovementStatus::EMS_FrontFlip);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	if (FrontFlipAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(FrontFlipAnimMontage);

		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
				SetMovementStatus(EMovementStatus::EMS_Normal);
				GetCharacterMovement()->bOrientRotationToMovement = true;

			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}
}

void AMain::FrontFlipStaminaManage()
{
	if (CurrentStamina > FrontFlipStaminaConsumption)
	{
		FadedStamina = FrontFlipStaminaConsumption / MaxStamina;
		CurrentStamina -= FrontFlipStaminaConsumption;
	}
	else
	{
		FadedStamina = CurrentStamina / MaxStamina;
		CurrentStamina = 0.f;
	}

	
}

void AMain::ClimbUp()
{
	GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, 0.0f);
	SetMovementStatus(EMovementStatus::EMS_ClimbUp);
	SetClimbStatus(EClimbStatus::ECS_NormalClimb);
	if (ClimbUpAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(ClimbUpAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}
}

void AMain::TurnCornerInsideRight()
{
	if (TurnCornerInsideRightAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(TurnCornerInsideRightAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능

		SetClimbStatus(EClimbStatus::ECS_TurnCorner);
	}
}

void AMain::TurnCornerInsideLeft()
{
	if (TurnCornerInsideLeftAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(TurnCornerInsideLeftAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능

		SetClimbStatus(EClimbStatus::ECS_TurnCorner);
	}
}

void AMain::TurnCornerOutsideRight()
{
	if (TurnCornerOutsideRightAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(TurnCornerOutsideRightAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{


			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능

		SetClimbStatus(EClimbStatus::ECS_TurnCorner);
	}		
}

void AMain::TurnCornerOutsideLeft()
{
	if (TurnCornerOutsideLeftAnimMontage)
	{
		FTimerHandle WaitHandle;
		float WaitTime = PlayAnimMontage(TurnCornerOutsideLeftAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{

			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능

		SetClimbStatus(EClimbStatus::ECS_TurnCorner);
	}
}