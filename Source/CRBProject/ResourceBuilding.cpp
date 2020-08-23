// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

// Sets default values
AResourceBuilding::AResourceBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	cube = CreateDefaultSubobject<UStaticMeshComponent>("cube");
	cube->SetupAttachment(RootComponent);
	
	cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		if (cube->GetCollisionEnabled())
			cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
			// add collision
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

