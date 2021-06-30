// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponActor.h"

#include "DrawDebugHelpers.h"
#include "GameplayCore/BonedShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Bullet.h"

// Sets default values
AWeaponActor::AWeaponActor()
{
	//Actor defaults
	WeaponSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	RootComponent = WeaponSkeletalMeshComponent;
	DefaultDamage = 1.f;
	TimeBetweenShots = .2f;

	// Replication specs
	bReplicates = true;
	MinNetUpdateFrequency = 33.f;

}



// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponActor::ServerFire_Implementation(AActor* ProjectileOwner, APawn* ProjectileInstigator, FVector SpawnLocation, FVector ProjectileDestination)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ProjectileOwner;
	SpawnParams.Instigator = ProjectileInstigator;

	ProjectileDirection = ProjectileDestination-SpawnLocation;
	ProjectileDirection.Normalize();
	float BulletSpread = Cast<ABonedShooterCharacter>(GetOwner())->CalculatedSpread;
	ProjectileDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(ProjectileDirection, BulletSpread);
	
	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(ProjectileClass, SpawnLocation, ProjectileDirection.ToOrientationRotator(), SpawnParams);
	if (Bullet)
	{
		Bullet->LaunchInDirection(ProjectileDirection);
		Cast<ABonedShooterCharacter>(GetOwner())->OnFired.Broadcast();
	}
	
}

bool AWeaponActor::ServerFire_Validate(AActor* ProjectileOwner, APawn* ProjectileInstigator, FVector SpawnLocation, FVector ProjectileDestination)
{
	return true;
}

void AWeaponActor::StartFire()
{
	const bool bDoLoop = true;
	const float FirstDelay = FMath::Max((LastFireTime - GetWorld()->TimeSeconds + TimeBetweenShots), 0.f);
	GetWorldTimerManager().SetTimer(TimerHandle_AutoFire, this, &AWeaponActor::Fire, TimeBetweenShots, bDoLoop, FirstDelay);
}

void AWeaponActor::EndFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_AutoFire);
}

void AWeaponActor::Fire()
{
		// Do the following on the client owner as well so that there is a minimal amount of latency when firing
	if (IsValid(GetOwner()))
	{
		LastFireTime = GetWorld()->TimeSeconds;

		// Hit-Scan Weapon: Trace the world from our Pawn point of view (camera) toward crosshair direction
		FVector TraceStart;
		FRotator ViewpointOrientation;
		GetOwner()->GetActorEyesViewPoint(TraceStart, ViewpointOrientation);
		const FVector ShotDirection = ViewpointOrientation.Vector();
		const FVector TraceEnd = TraceStart + ShotDirection * 10000.f;


		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FHitResult CameraTargetHitResult;
		bool bFirstHit = GetWorld()->LineTraceSingleByChannel(CameraTargetHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);


		if (ProjectileClass != nullptr)
		{
			FVector ProjectileTarget;
			if (bFirstHit)
			{
				ProjectileTarget = CameraTargetHitResult.Location;
			}
			
			if (!bFirstHit)
			{
				ProjectileTarget = TraceEnd;				
			}
			// Now we can spawn projectile
			FVector MuzzleLocation = WeaponSkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);

			ServerFire(this, GetOwner()->GetInstigator(), MuzzleLocation, ProjectileTarget);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AWeaponActor::Fire: No Owner."));
	}
}



void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponActor, HandleSocketName);
	DOREPLIFETIME(AWeaponActor, MuzzleSocketName);
	DOREPLIFETIME(AWeaponActor, WeaponSkeletalMeshComponent);
	DOREPLIFETIME(AWeaponActor, ProjectileDirection);
	

}
