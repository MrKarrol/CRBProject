// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

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
	showIncomeArea(true);
	onTargeted();
}

void AResourceBuilding::untargetRB()
{
	mIsTargeted = false;
	showIncomeArea(false);
	onUntargeted();
}

void AResourceBuilding::destroyRB()
{
	onUntargeted();
	Destroy();
}

void AResourceBuilding::showIncomeArea(bool show)
{
	CursorToWorld->SetVisibility(show);
	overlapPercents->SetVisibility(show);
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
		}
			
	}
	else
	{
		if (!postPlacingActionsApplied)
		{
			cube->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			postPlacingActionsApplied = true;
		}
		// some logic
	}

	auto income = resourceBuildingIncome();
	if (mIsPlaced)
	{
		if (income > currentIncome) // if near rb destroyed
			currentIncome = income;
	}
	else
		currentIncome = income;
	overlapPercents->SetText(FString::FromInt(currentIncome));
}


using Point = std::pair<float, float>;

double getDistance(const Point &a, const Point &b)
{
	double result = 0;

	float x_diff = a.first - b.first;
	float y_diff = a.second - b.second;
	result = std::sqrt(x_diff * x_diff + y_diff * y_diff);

	return result;
};

// getting income circle points like so (on paper looks better))
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
std::vector<Point> getCirclePoints(float x, float y, float resourceIncomeDistance)
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

	return result;
};

float AResourceBuilding::resourceBuildingIncome()
{
	auto incomeAreaPoints = getCirclePoints(GetActorLocation().X, GetActorLocation().Y, incomeAreaRadius);

	std::vector<Point> wrongPoints;

	// check if income points is in unreachable location
	for (const auto &point : incomeAreaPoints)
	{
		FVector start = GetActorLocation();
		FVector end(point.first, point.second, start.Z);
		/*if (! mIsPlaced)
			DrawDebugPoint(GetWorld(), end, 10.f, FColor::Red);*/

		auto world = GetWorld();
		if (!world)
			continue;
		
		auto NavSystem = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
		if (!NavSystem)
			continue;
		
		const ANavigationData* NavData = NavSystem->GetNavDataForProps(GetWorld()->GetFirstPlayerController()->GetNavAgentPropertiesRef());
		if (!NavData)
			continue;
		
		FPathFindingQuery query(NavSystem, *NavData, GetActorLocation(), end);
		if (!Cast<UNavigationSystemV1>(NavSystem)->TestPathSync(query))
		{
			if (std::find(wrongPoints.begin(), wrongPoints.end(), point) == wrongPoints.end())
			{
				wrongPoints.emplace_back(point);
				continue;
			}
		}
	}

	// check income based on another rb
	TArray<FHitResult> OutHits;
	FQuat rot = FQuat::Identity;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(incomeAreaRadius*2);

	GetWorld()->SweepMultiByObjectType(OutHits, GetActorLocation(), GetActorLocation() + incomeAreaRadius*2, rot, ObjectQueryParams, CollisionShape, {});

	for (const auto& hit : OutHits)
	{
		auto actor = hit.Actor.Get();
		if (!Cast<AResourceBuilding>(actor))
			continue;

		float distanceToAnotherRb = GetDistanceTo(hit.Actor.Get());
		if (distanceToAnotherRb < 1) // hitted by himself
			continue;
		if (distanceToAnotherRb > incomeAreaRadius*2) // income circles do not overlap each other
			continue;

		for (const auto &point : incomeAreaPoints)
		{
			auto rbLocation = hit.Actor.Get()->GetActorLocation();
			if (getDistance(point, Point(rbLocation.X, rbLocation.Y)) < incomeAreaRadius)
				if (std::find(wrongPoints.begin(), wrongPoints.end(), point) == wrongPoints.end())
				{
					wrongPoints.emplace_back(point);
					continue;
				}
		}
	}
	float income = incomeAreaPoints.size() - wrongPoints.size();

	return income;
}

FVector AResourceBuilding::currentLocation()
{
	FHitResult result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, result);
	return result.Location;
}

