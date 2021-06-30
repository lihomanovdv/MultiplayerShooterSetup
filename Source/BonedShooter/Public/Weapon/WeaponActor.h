// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class BONEDSHOOTER_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();

	void StartFire();
	void EndFire();

	// Projectile class to spawn.
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class ABullet> ProjectileClass;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	FName HandleSocketName;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	FName MuzzleSocketName;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	class USkeletalMeshComponent* WeaponSkeletalMeshComponent;

	// Time between shots in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	float DefaultDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BonedShooterCharacter|Weapon")
	TSubclassOf<UDamageType> DamageTypeClass;

	UFUNCTION(BlueprintCallable, Category = "BonedShooterCharacter|Weapon")
	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(AActor* ProjectileOwner, APawn* ProjectileInstigator, FVector SpawnLocation, FVector ProjectileDestination);
	
private:
	float LastFireTime = 0.f;
	FTimerHandle TimerHandle_AutoFire;

	UPROPERTY(Replicated)
	FVector ProjectileDirection;

};
