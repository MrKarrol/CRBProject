// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
AResourceBuilding::AResourceBuilding()
{
	// In your constructor
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = SceneRoot;

	cube = CreateDefaultSubobject<UStaticMeshComponent>("Cube");
	cube->SetupAttachment(RootComponent);
	cube->SetRelativeLocation(FVector(0, 0, 0));
	
	cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	CursorToWorld->SetRelativeLocation(FVector(0, 0, 0));

	overlapPercents = CreateDefaultSubobject<UTextRenderComponent>("overlapPercents");
	overlapPercents->SetupAttachment(RootComponent);

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
			// change location based on cursor position
			auto oldLocation = GetActorLocation();
			SetActorLocation(newLocation);

			TArray<FHitResult> OutHits;
			FQuat rot = FQuat::Identity;
			FCollisionObjectQueryParams ObjectQueryParams;
			ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
			FCollisionShape CollisionShape = FCollisionShape::MakeSphere(500);;

			GetWorld()->SweepMultiByObjectType(OutHits, newLocation, newLocation + 500, rot, ObjectQueryParams, CollisionShape, {});

			DrawDebugLine(GetWorld(), newLocation, newLocation + 500,
				FColor(255, 0, 0), false, -1.f, 0, 5);

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Number of hits %d"), OutHits.Num()));

			for (const auto& result : OutHits)
			{
				auto name = result.Actor.Get()->GetName();
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Hits by %s"), *name));
			}
		}
			
	}
	else
	{
		if (!buildingConfigured)
		{
			cube->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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

