// Copyright Epic Games, Inc. All Rights Reserved.

#include "CRBProjectPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "CRBProjectCharacter.h"

#include "Kismet/GameplayStatics.h"

#include "Engine/World.h"

ACRBProjectPlayerController::ACRBProjectPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ACRBProjectPlayerController::AttachRBToController(TScriptInterface<IResourceBuildingInterface> newRb)
{
	rb = newRb;
	rb->setPlaced(false);

	mIsRBAttached = true;
}

void ACRBProjectPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}

void ACRBProjectPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ACRBProjectPlayerController::OnActionRequested);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ACRBProjectPlayerController::OnActionDisbound);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ACRBProjectPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ACRBProjectPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &ACRBProjectPlayerController::OnResetVR);
}

void ACRBProjectPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACRBProjectPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (ACRBProjectCharacter* MyPawn = Cast<ACRBProjectCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void ACRBProjectPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void ACRBProjectPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void ACRBProjectPlayerController::OnActionRequested()
{
	// if resource building attached
	if (mIsRBAttached)
	{
		// do stuff
		rb->setPlaced(true);
	}
	else // check if resource building clicked
	{
		FHitResult result;
		GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, result);
		if (Cast<IResourceBuildingInterface>(result.Actor.Get()))
		{
			if (mIsRBTargeted)
				targetedRb->untargetRB();
			else
				mIsRBTargeted = true;

			targetedRb = result.Actor.Get();
			targetedRb->targetRB();
		}
		else // request to move character
		{
			if (mIsRBTargeted)
			{
				mIsRBTargeted = false;
				targetedRb->untargetRB();
				targetedRb = nullptr;
			}
			OnSetDestinationPressed();
		}
	}
}

void ACRBProjectPlayerController::OnActionDisbound()
{
	if (mIsRBAttached)
	{
		mIsRBAttached = false;
		rb = nullptr;
	}
	else // request to stop moving
	{
		OnSetDestinationReleased();
	}
}

void ACRBProjectPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void ACRBProjectPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}
