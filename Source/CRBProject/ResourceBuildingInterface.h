// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ResourceBuildingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UResourceBuildingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRBPROJECT_API IResourceBuildingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool isPlaced() const = 0;
	virtual void setPlaced(bool isPlaced) = 0;

	virtual bool isTargeted() const = 0;
	virtual void targetRB() = 0;
	virtual void untargetRB() = 0;

	virtual void destroyRB() = 0;
};
