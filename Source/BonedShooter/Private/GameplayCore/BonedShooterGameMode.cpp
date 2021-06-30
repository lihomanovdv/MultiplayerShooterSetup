// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayCore/BonedShooterGameMode.h"
#include "GameplayCore/BonedShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABonedShooterGameMode::ABonedShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	// if (PlayerPawnBPClass.Class != NULL)
	// {
	// 	DefaultPawnClass = PlayerPawnBPClass.Class;
	// }
}
