// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ResourceBuildingInterface.h"
#include "CRBProjectPlayerController.generated.h"

UCLASS()
class ACRBProjectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACRBProjectPlayerController();

	UFUNCTION(BlueprintCallable)
	void AttachRBToController(TScriptInterface<IResourceBuildingInterface> rb);

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for action. */
	void OnActionRequested();
	void OnActionDisbound();

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

private:
	void UntargetRB();

private:
	bool mIsRBAttached = false;
	TScriptInterface<IResourceBuildingInterface> rb = nullptr;

	bool mIsRBTargeted = false;
	TScriptInterface<IResourceBuildingInterface> targetedRb = nullptr;
};


