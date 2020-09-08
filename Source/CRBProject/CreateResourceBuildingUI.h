// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateResourceBuildingUI.generated.h"

/**
 * 
 */
UCLASS()
class CRBPROJECT_API UCreateResourceBuildingUI : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UCreateResourceBuildingUI(const FObjectInitializer& ObjectInitializer);

	// Optionally override the Blueprint "Event Construct" event
	virtual void NativeConstruct() override;

	// Optionally override the tick event
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable) FVector CurrentLocation();

public:
	UPROPERTY(BlueprintReadWrite) float height = 250.0;
};
