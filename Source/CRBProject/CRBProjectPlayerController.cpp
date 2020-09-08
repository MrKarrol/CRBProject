// Copyright Epic Games, Inc. All Rights Reserved.

#include "CRBProjectPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "CRBProjectCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "ResourceBuilding.h"

#include "Engine/World.h"

ACRBProjectPlayerController::ACRBProjectPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ACRBProjectPlayerController::AttachRbToController(TScriptInterface<IResourceBuildingInterface> rb)
{
	// preparation
	UnselectRb();

	TArray<AActor*> rbs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceBuilding::StaticClass(), rbs);
	for (auto resourceBuilding : rbs)
	{
		Cast<AResourceBuilding>(resourceBuilding)->ShowIncomeArea(true);
	}

	// creating
	m_AttachedRb = rb;
	m_AttachedRb->SetPlaced(false);

	m_IsRbAttached = true;
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

void ACRBProjectPlayerController::UnselectRb()
{
	if (m_IsRbSelected)
	{
		m_IsRbSelected = false;
		m_SelectedRb->Unselect();
		m_SelectedRb = nullptr;
	}
}


void ACRBProjectPlayerController::OnActionRequested()
{
	// if resource building attached
	if (m_IsRbAttached)
	{
		m_AttachedRb->SetPlaced(true);

		TArray<AActor*> rbs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceBuilding::StaticClass(), rbs);
		for (auto resourceBuilding : rbs)
		{
			Cast<AResourceBuilding>(resourceBuilding)->ShowIncomeArea(false);
		}

		m_IsRbSelected = true;
		m_SelectedRb = m_AttachedRb;
		m_SelectedRb->Select();
	}
	else // check if resource building clicked
	{
		FHitResult hit_result;
		GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, hit_result);
		if (Cast<IResourceBuildingInterface>(hit_result.Actor.Get()))
		{
			if (m_IsRbSelected)
				m_SelectedRb->Unselect();
			else
				m_IsRbSelected = true;

			m_SelectedRb = hit_result.Actor.Get();
			m_SelectedRb->Select();
		}
		else // request to move character
		{
			UnselectRb();
			OnSetDestinationPressed();
		}
	}
}

void ACRBProjectPlayerController::OnActionDisbound()
{
	if (m_IsRbAttached)
	{
		m_IsRbAttached = false;
		m_AttachedRb = nullptr;
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

