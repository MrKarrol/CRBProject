// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"

// Sets default values
AResourceBuilding::AResourceBuilding()
{
	// In your constructor
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = SceneRoot;

	cube = CreateDefaultSubobject<UStaticMeshComponent>("Cube");
	cube->AttachTo(RootComponent);
	cube->SetRelativeLocation(FVector(0, 0, 0));
	
	cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->AttachTo(RootComponent);
	CursorToWorld->SetRelativeLocation(FVector(0, 0, 0));

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AResourceBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AResourceBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (! isPlaced)
	{
		auto newLocation = currentLocation();
		if (newLocation != FVector(0, 0, 0))
		{
			auto oldLocation = GetActorLocation();
			SetActorLocation(newLocation);
		}
			
	}
	else
	{
		if (!buildingConfigured)
		{
			cube->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

			buildingConfigured = true;
		}
		// some logic
	}
}

FVector AResourceBuilding::currentLocation()
{
	FVector mouseLocation;
	FVector mouseDirection;
	FHitResult result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, result);

	mouseLocation = result.Location;

	return mouseLocation;
}

