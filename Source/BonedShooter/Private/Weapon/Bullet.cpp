// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Bullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Set the sphere's collision radius.
	CollisionComponent->InitSphereRadius(3.0f);
	// Set the root component to be the collision component.
	RootComponent = CollisionComponent;


	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
	ProjectileMeshComponent->SetGenerateOverlapEvents(false);
	ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMeshComponent->SetupAttachment(RootComponent);

	// Projectile Setup.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->InitialSpeed = 1000.0f;
	ProjectileMovementComponent->MaxSpeed = 1000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = false;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;


	// Lifecycle
	InitialLifeSpan = 2.0f;

	//Collisions
	CollisionComponent->BodyInstance.SetObjectType(ECC_WorldDynamic);
	CollisionComponent->BodyInstance.SetResponseToAllChannels(ECR_Block);
	CollisionComponent->BodyInstance.SetResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComponent->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->BodyInstance.bNotifyRigidBodyCollision = true;

	CollisionComponent->OnComponentHit.AddDynamic(this, &ABullet::OnHit);

	// Replication specs
	bReplicates = true;
	MinNetUpdateFrequency = 33.f;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::LaunchInDirection(const FVector& ShootDirection)
{
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void ABullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != this)
	{
		// Optional parameters, for visual clarity
		float BaseDamage = 1.f;
		
		UGameplayStatics::ApplyPointDamage(OtherActor, BaseDamage, GetVelocity().GetSafeNormal(),
			Hit, GetInstigator()->GetInstigatorController(), this, UDamageType::StaticClass());
		
	}
	Destroy();
}

