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

bool AResourceBuilding::isPlaced() const
{
	return mIsPlaced;
}

void AResourceBuilding::setPlaced(bool isPlaced)
{
	mIsPlaced = isPlaced;
}

bool AResourceBuilding::isTargeted() const
{
	return mIsTargeted;
}

void AResourceBuilding::targetRB()
{
	mIsTargeted = true;

	CursorToWorld->SetVisibility(true);
	overlapPercents->SetVisibility(true);

	onTargeted();
}

void AResourceBuilding::untargetRB()
{
	mIsTargeted = false;

	CursorToWorld->SetVisibility(false);
	overlapPercents->SetVisibility(false);

	onUntargeted();
}

void AResourceBuilding::destroyRB()
{
	onUntargeted();
	Destroy();
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

	if (! mIsPlaced)
	{
		auto newLocation = currentLocation();
		if (newLocation != FVector(0, 0, 0))
		{
			// change location based on cursor position
			auto oldLocation = GetActorLocation();
			SetActorLocation(newLocation);
		}
			
	}
	else
	{
		if (!buildingConfigured)
		{
			cube->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			CursorToWorld->SetVisibility(false);
			overlapPercents->SetVisibility(false);

			buildingConfigured = true;
		}
		// some logic
	}

	auto income = resourceBuildingIncome();
	if (mIsPlaced)
	{
		if (income > currentIncome) // if near rb destroyed
			currentIncome = income;
		else
			income = currentIncome;
	}
	else
		currentIncome = income;
	overlapPercents->SetText(FString::FromInt(income));
}

float AResourceBuilding::resourceBuildingIncome()
{
	TArray<FHitResult> OutHits;
	FQuat rot = FQuat::Identity;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(resourceIncomeDistance);;

	GetWorld()->SweepMultiByObjectType(OutHits, GetActorLocation(), GetActorLocation() + resourceIncomeDistance, rot, ObjectQueryParams, CollisionShape, {});

	//DrawDebugLine(GetWorld(), newLocation, newLocation + 500,
		//FColor(255, 0, 0), false, -1.f, 0, 5);

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Number of hits %d"), OutHits.Num()));

	float income = 100;
	for (const auto& result : OutHits)
	{
		auto actor = result.Actor.Get();
		if (!Cast<AResourceBuilding>(actor))
			continue;

		float distance = GetDistanceTo(result.Actor.Get());
		
		if (distance < 1) // hitted by himself
			continue;

		if (distance > resourceIncomeDistance) distance = resourceIncomeDistance;

		income += distance / resourceIncomeDistance * 100 - 100;
		auto name = result.Actor.Get()->GetName();
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Distance %f"), distance));
	}
	return income;
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

