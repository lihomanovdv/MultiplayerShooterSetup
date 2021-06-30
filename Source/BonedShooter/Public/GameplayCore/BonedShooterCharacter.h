// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BonedShooterCharacter.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFired);

UCLASS(config=Game)
class ABonedShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ABonedShooterCharacter();
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION(BlueprintCallable, Category="BonedShooterCharacter")
	class AWeaponActor* GetWeaponActor();
	
	UPROPERTY(Replicated, BlueprintReadWrite)
	float CalculatedSpread;

	UPROPERTY(BlueprintAssignable)
	FOnFired OnFired;
protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// --- Base movement -- //
	
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	UFUNCTION()
	void InputTurn(float Rate);
	void InputLookUp(float Rate);

	
	// --- Aiming & Shooting -- //
	
	/** 
	 * Called via input to turn Aiming on. 
	 */
	UFUNCTION(BlueprintNativeEvent)
	void StartAiming();
	/** 
	* Called via input to turn Aiming off. 
	*/
	UFUNCTION(BlueprintNativeEvent)
	void StopAiming();
	
	UFUNCTION(Server, Reliable)
	void ServerStartAim();
	
	UFUNCTION(Server, Reliable)
	void ServerStopAim();
	
	UFUNCTION(BlueprintGetter, Category = "BonedShooterCharacter")
    bool IsAiming() { return bIsAiming; };

	void StartFire();
	void EndFire();

	UPROPERTY(Replicated, BlueprintReadOnly)
	FRotator TargetAimRotation;
	
	UPROPERTY(Replicated)
	class AWeaponActor* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "BonedShooterCharacter")
	TSubclassOf<AWeaponActor> WeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "BonedShooterCharacter")
	FName WeaponSocketName;



private:
	UPROPERTY(Replicated, BlueprintGetter="IsAiming")
	bool bIsAiming;
 
	UFUNCTION()
	void ReplicateAimOffset();

	UFUNCTION(Server, Reliable)
	void SetRemoteControlRotation();
 
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** @return	Pawn's eye location */
	virtual FVector GetPawnViewLocation() const override;


};