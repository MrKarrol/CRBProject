// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResourceBuildingInterface.h"
#include "ControlResourceBuildingUI.generated.h"

/**
 * 
 */
UCLASS()
class CRBPROJECT_API UControlResourceBuildingUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void AttachRbToWidget(TScriptInterface<IResourceBuildingInterface> rb);

	UFUNCTION(BlueprintCallable)
	void DestroyRb();

private:
	TScriptInterface<IResourceBuildingInterface> m_Rb;
};
