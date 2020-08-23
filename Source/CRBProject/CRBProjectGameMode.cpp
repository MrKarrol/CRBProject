// Copyright Epic Games, Inc. All Rights Reserved.

#include "CRBProjectGameMode.h"
#include "CRBProjectPlayerController.h"
#include "CRBProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACRBProjectGameMode::ACRBProjectGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ACRBProjectPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}