// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "ResourceBuildingNavArea.generated.h"

/**
 * 
 */
UCLASS()
class CRBPROJECT_API UResourceBuildingNavArea : public UNavArea
{
	GENERATED_BODY()

	virtual void InitializeArea() override;
};
