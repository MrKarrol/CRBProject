// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "ResourceBuildingInterface.h"
#include "ResourceBuildingNavModComponent.h"

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

	UFUNCTION(BlueprintImplementableEvent)
	void OnSelected();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUnselected();

	void ShowIncomeArea(bool show);

	virtual bool IsPlaced() const override;

	virtual void SetPlaced(bool isPlaced) override;

	virtual bool IsSelected() const override;
	virtual void Select() override;
	virtual void Unselect() override;

	virtual void DestroyResourceBuilding() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* scene_root;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *cube;

	UPROPERTY(VisibleAnywhere) 
	UTextRenderComponent *income_text;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* income_area_circle;

	UPROPERTY(EditDefaultsOnly) float income_area_radius = 400;

private:
	UFUNCTION() FVector CurrentLocation() const;
	UFUNCTION() float ResourceBuildingIncome() const;

private:
	bool m_PostPlacingActionsApplied = false;
	bool m_IsPlaced = true;
	float m_CurrentIncome = 100;
	bool m_IsSelected = false;

	int m_Num = 0;
	static int m_RbCount;
};
