// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"

#include <vector>


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
			SetActorLocation(newLocation);
			location = newLocation;
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

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Number of hits %d"), OutHits.Num()));

	using Point = std::pair<float, float>;
	// like so (on paper looks better))
	// 1       x
	// 5     xxxxx
	// 7    xxxxxxx
	// 9   xxxxxxxxx
	//11  xxxxxxxxxxx
	//11  xxxxxxxxxxx
	//12 xxxxxx xxxxxx
	//11  xxxxxxxxxxx
	//11  xxxxxxxxxxx
	// 9   xxxxxxxxx
	// 7    xxxxxxx
	// 5     xxxxx
	// 1       x
	auto getCirclePoints = [](float x, float y, float resourceIncomeDistance)
	{
		std::vector<Point> result;
		float cellSize = resourceIncomeDistance / 6;

		result.emplace_back(x, y + 6 * cellSize);
		result.emplace_back(x, y - 6 * cellSize);

		auto fillRange = [&result, x, y, cellSize](int range, int step)
		{
			for (int iter = -range; iter <= range; ++iter)
			{
				result.emplace_back(x + iter * cellSize, y + step * cellSize);
				if (step != 0) result.emplace_back(x + iter * cellSize, y - step * cellSize);
			}
		};

		fillRange(2, 5);
		fillRange(3, 4);
		fillRange(4, 3);
		fillRange(5, 2);
		fillRange(5, 1);
		fillRange(6, 0);
		auto centerIter = std::find(result.begin(), result.end(), std::pair<float, float>(x, y));
		if (centerIter != result.end())
			result.erase(centerIter);
		else
		{
			// must not pop up
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Error in income calc")));
		}
		
		return result;
	};

	auto circlePoints = getCirclePoints(GetActorLocation().X, GetActorLocation().Y, resourceIncomeDistance / 2);

	auto getDistance = [](const Point &a, const Point &b)
	{
		double result = 0;

		float x_diff = a.first - b.first;
		float y_diff = a.second - b.second;
		result = std::sqrt(x_diff * x_diff + y_diff * y_diff);

		return result;
	};

	std::vector<Point> wrongPoints;
	float income = 100;

	float debugZ = 0;

	for (const auto& result : OutHits)
	{
		auto actor = result.Actor.Get();
		if (!Cast<AResourceBuilding>(actor))
			continue;

		float distanceToAnotherRb = GetDistanceTo(result.Actor.Get());
		
		if (distanceToAnotherRb < 1) // hitted by himself
			continue;

		if (distanceToAnotherRb > resourceIncomeDistance)
			continue;

		for (const auto &point : circlePoints)
		{
			auto rbLocation = result.Actor.Get()->GetActorLocation();
			if (getDistance(point, Point(rbLocation.X, rbLocation.Y)) < resourceIncomeDistance / 2)
				if (std::find(wrongPoints.begin(), wrongPoints.end(), point) == wrongPoints.end())
					wrongPoints.emplace_back(point);

			float length = 0;

			// trying use recast nav mesh
			FNavLocation randomReachablePoint;
			navData->GetRandomPointInNavigableRadius(location, 500, randomReachablePoint);
			FVector start = location;
			FVector end(point.first, point.second, start.Z);
			auto queryResult = navData->CalcPathLength(start, end, length);
			if (length > 400)
				DrawDebugPoint(GetWorld(), end, 10, FColor::Red);
			// end trying use recast nav mesh
		}
		
	}
	income = circlePoints.size() - wrongPoints.size();

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

