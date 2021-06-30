// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayCore/BonedShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Weapon/WeaponActor.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ABonedShooterCharacter

ABonedShooterCharacter::ABonedShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	CameraBoom->SocketOffset = FVector(0.f, 100.f, 50.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}


AWeaponActor* ABonedShooterCharacter::GetWeaponActor()
{
	return CurrentWeapon;
}

void ABonedShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Spawn and Attach weapon to the Character. Only on server.
	if (HasAuthority() && WeaponClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		CurrentWeapon = GetWorld()->SpawnActor<AWeaponActor>(WeaponClass, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
		}
	}
}

void ABonedShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);		
}

//////////////////////////////////////////////////////////////////////////
// Input
void ABonedShooterCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABonedShooterCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABonedShooterCharacter::StopAiming);
	
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ABonedShooterCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ABonedShooterCharacter::EndFire);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ABonedShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABonedShooterCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ABonedShooterCharacter::InputTurn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABonedShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ABonedShooterCharacter::InputLookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABonedShooterCharacter::LookUpAtRate);

}

void ABonedShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	InputTurn(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());

}

void ABonedShooterCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	InputLookUp(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABonedShooterCharacter::InputTurn(float Rate)
{
	AddControllerYawInput(Rate);
	if (IsLocallyControlled())
		TargetAimRotation = GetControlRotation();
	
	ReplicateAimOffset();
}

void ABonedShooterCharacter::InputLookUp(float Rate)
{
	AddControllerPitchInput(Rate);
	if (IsLocallyControlled())
		TargetAimRotation = GetControlRotation();
	ReplicateAimOffset();
}

void ABonedShooterCharacter::ReplicateAimOffset()
{
	if (HasAuthority())
	{
		TargetAimRotation = GetControlRotation();
	}
	else
	{
		SetRemoteControlRotation();
	}
}

void ABonedShooterCharacter::SetRemoteControlRotation_Implementation()
{
	ReplicateAimOffset();
}

void ABonedShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABonedShooterCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
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

void ABonedShooterCharacter::StartAiming_Implementation()
{
	if (HasAuthority())
	{
		bIsAiming = true;
	}
	else
	{
		ServerStartAim();
	}
}

void ABonedShooterCharacter::StopAiming_Implementation()
{
	if (HasAuthority())
	{
		bIsAiming = false;
	}
	else
	{
		ServerStopAim();
	}
}

void ABonedShooterCharacter::ServerStopAim_Implementation()
{
	StopAiming();
}

void ABonedShooterCharacter::ServerStartAim_Implementation()
{
	StartAiming();
}

void ABonedShooterCharacter::StartFire()
{
	if (CurrentWeapon && bIsAiming)
	{
		CurrentWeapon->StartFire();
	}
}

void ABonedShooterCharacter::EndFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->EndFire();
	}
}

FVector ABonedShooterCharacter::GetPawnViewLocation() const
{
	return FollowCamera ? FollowCamera->GetComponentLocation() : Super::GetPawnViewLocation();
}

void ABonedShooterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABonedShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(ABonedShooterCharacter, bIsAiming);
	DOREPLIFETIME_CONDITION(ABonedShooterCharacter, TargetAimRotation, COND_SimulatedOnly );
	DOREPLIFETIME(ABonedShooterCharacter, CalculatedSpread);
}