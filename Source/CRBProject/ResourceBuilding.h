// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
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
	UFUNCTION() float resourceBuildingIncome();

public:
	UPROPERTY(BlueprintReadWrite) bool isPlaced = true;
	bool buildingConfigured = false;

	UPROPERTY(EditInstanceOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(EditInstanceOnly) UStaticMeshComponent *cube;

	UPROPERTY(VisibleAnywhere) UTextRenderComponent *overlapPercents;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	UPROPERTY(VisibleAnywhere) float resourceIncomeDistance = 700;


};
