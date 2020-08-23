// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceBuilding.generated.h"

UCLASS()
class CRBPROJECT_API AResourceBuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResourceBuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION() FVector currentLocation();

public:
	UPROPERTY(BlueprintReadOnly) bool isPlaced = false;
	bool buildingConfigured = false;
	UPROPERTY(EditInstanceOnly) UStaticMeshComponent *cube;

};
