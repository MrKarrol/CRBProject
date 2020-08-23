// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateResourceBuildingUI.h"

UCreateResourceBuildingUI::UCreateResourceBuildingUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UCreateResourceBuildingUI::NativeConstruct()
{
	// Do some custom setup

	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();
}


void UCreateResourceBuildingUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	// Make sure to call the base class's NativeTick function
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Do your custom tick stuff here
}

FVector UCreateResourceBuildingUI::currentLocation()
{
	FVector mouseLocation;
	FVector mouseDirection;
	FHitResult result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, result);

	mouseLocation = result.Location;

	return mouseLocation;
}