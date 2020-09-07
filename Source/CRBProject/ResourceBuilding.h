// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "ResourceBuildingInterface.h"
#include "NavigationData.h"
#include "ResourceBuilding.generated.h"

UCLASS()
class CRBPROJECT_API AResourceBuilding 
	: public AActor
	, public IResourceBuildingInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResourceBuilding();


	bool isPlaced() const override;

	void setPlaced(bool isPlaced) override;

	bool isTargeted() const override;
	void targetRB() override;
	void untargetRB() override;

	void destroyRB() override;

	void showIncomeArea(bool show);

	UFUNCTION(BlueprintImplementableEvent)
		void onTargeted();

	UFUNCTION(BlueprintImplementableEvent)
		void onUntargeted();

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
	bool mIsPlaced = true;
	bool postPlacingActionsApplied = false;

	UPROPERTY(EditInstanceOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(EditInstanceOnly) UStaticMeshComponent *cube;

	UPROPERTY(VisibleAnywhere) UTextRenderComponent *overlapPercents;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	UPROPERTY(VisibleAnywhere) float incomeAreaRadius = 350;

private:
	float currentIncome = 100;

	bool mIsTargeted = false;
};
