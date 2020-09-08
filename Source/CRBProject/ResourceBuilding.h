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


	bool IsPlaced() const override;

	void SetPlaced(bool isPlaced) override;

	bool IsSelected() const override;
	void Select() override;
	void Unselect() override;

	void DestroyResourceBuilding() override;

	void ShowIncomeArea(bool show);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSelected();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUnselected();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditInstanceOnly)
	USceneComponent* scene_root;

	UPROPERTY(EditInstanceOnly) UStaticMeshComponent *cube;

	UPROPERTY(VisibleAnywhere) UTextRenderComponent *income_text;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* income_area_circle;

	UPROPERTY(VisibleAnywhere) float income_area_radius = 350;

private:
	UFUNCTION() FVector CurrentLocation();
	UFUNCTION() float ResourceBuildingIncome();

private:
	bool m_PostPlacingActionsApplied = false;
	bool m_IsPlaced = true;
	float m_CurrentIncome = 100;
	bool m_IsSelected = false;
};
