// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "OverCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AllyController.h"
#include "Kismet/GameplayStatics.h"
#include "Components\SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AOverCharacter

AOverCharacter::AOverCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	//CurrentFilter = EFilter::VE_All;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CurrentFilter = EHenpeckedEnum::HE_All;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOverCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("HenpeckedAttack", IE_Pressed, this, &AOverCharacter::HenpeckedAttack);
	PlayerInputComponent->BindAction("WhiteFilter", IE_Pressed, this, &AOverCharacter::WhiteFilter);
	PlayerInputComponent->BindAction("RedFilter", IE_Pressed, this, &AOverCharacter::RedFilter);
	PlayerInputComponent->BindAction("GreenFilter", IE_Pressed, this, &AOverCharacter::GreenFilter);
	PlayerInputComponent->BindAction("BlueFilter", IE_Pressed, this, &AOverCharacter::BlueFilter);
	PlayerInputComponent->BindAction("AllFilter", IE_Pressed, this, &AOverCharacter::AllFilter);
	PlayerInputComponent->BindAction("HenpeckedRetreat", IE_Pressed, this, &AOverCharacter::HenpeckedRetreat);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOverCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOverCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AOverCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AOverCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AOverCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AOverCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AOverCharacter::OnResetVR);
}




void AOverCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AOverCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AOverCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AOverCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AOverCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AOverCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

	}
}

void AOverCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AOverCharacter::Tick(float DeltaTime)
{
	FVector Coefficient(0, 0, 0);
	for (AHenpecked* const &ElemHenpecked : GetHenpeckeds(EHenpeckedEnum::HE_All))
	{
		if (!ElemHenpecked->HaveTask)
		{
			Coefficient += FVector(0, 40, 0);
			Cast<AAllyController>(ElemHenpecked->Controller)->MoveToLocation(this->GetActorLocation() - Coefficient);
			
		}
	}
	Coefficient = FVector(0, 0, 0);
}

void AOverCharacter::HenpeckedAttack()
{
	for (AHenpecked* const &ElemHenpecked :GetHenpeckeds(CurrentFilter))
	{
		ElemHenpecked->HaveTask = true;
		FVector StartPoint = this->GetActorLocation();
		FVector ForwardVector = this->FollowCamera->GetForwardVector();
		FVector End = (StartPoint + (ForwardVector * 500.f));
		Cast<AAllyController>(ElemHenpecked->Controller)->MoveToLocation(End);
	}
}

void AOverCharacter::HenpeckedRetreat()
{
	FVector Coefficient(0, 0, 0);
	for (AHenpecked* const &ElemHenpecked : GetHenpeckeds(CurrentFilter))
	{
		Coefficient += FVector(0, 40, 0);
		ElemHenpecked->HaveTask = false;
		Cast<AAllyController>(ElemHenpecked->Controller)->MoveToLocation(this->GetActorLocation()-Coefficient);
	}
	Coefficient = FVector(0, 0, 0);
}

void AOverCharacter::WhiteFilter()
{
	CurrentFilter = EHenpeckedEnum::HE_White;
	UpdateUI();
}

void AOverCharacter::BlueFilter()
{
	CurrentFilter = EHenpeckedEnum::HE_Blue;
	UpdateUI();
}

void AOverCharacter::GreenFilter()
{
	CurrentFilter = EHenpeckedEnum::HE_Green;
	UpdateUI();
}

void AOverCharacter::RedFilter()
{
	CurrentFilter = EHenpeckedEnum::HE_Red;
	UpdateUI();
}

void AOverCharacter::AllFilter()
{
	CurrentFilter = EHenpeckedEnum::HE_All;
	UpdateUI();
}

TArray<AHenpecked*> AOverCharacter::GetHenpeckeds(EHenpeckedEnum Filter)
{
	TArray<AActor*> Actors;
	AHenpecked* Henpecked;
	TArray<AHenpecked*> ArrHenpecked;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHenpecked::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		Henpecked = Cast<AHenpecked>(Actor);
		switch (Filter)
		{
		case EHenpeckedEnum::HE_White:
			if (Henpecked->TypeHenpecked == EHenpeckedEnum::HE_White)
			{
				ArrHenpecked.Add(Henpecked);
			}
			break;
		case EHenpeckedEnum::HE_Red:
			if (Henpecked->TypeHenpecked == EHenpeckedEnum::HE_Red)
			{
				ArrHenpecked.Add(Henpecked);
			}
			break;
		case EHenpeckedEnum::HE_Green:
			if (Henpecked->TypeHenpecked == EHenpeckedEnum::HE_Green)
			{
				ArrHenpecked.Add(Henpecked);
			}
			break;
		case EHenpeckedEnum::HE_Blue:
			if (Henpecked->TypeHenpecked == EHenpeckedEnum::HE_Blue)
			{
				ArrHenpecked.Add(Henpecked);
			}
			break;
		default:
			ArrHenpecked.Add(Henpecked);
			break;
		}
	}
	return ArrHenpecked;
}