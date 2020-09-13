// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

// Sets default values
AResourceBuilding::AResourceBuilding()
{
	// In your constructor
	scene_root = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent = scene_root;

	cube = CreateDefaultSubobject<UStaticMeshComponent>("Cube");
	cube->SetupAttachment(RootComponent);
	cube->SetRelativeLocation(FVector(0, 0, 0));
	
	cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create a decal in the world to show the cursor's location
	income_area_circle = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	income_area_circle->SetupAttachment(RootComponent);
	income_area_circle->SetRelativeLocation(FVector(0, 0, 0));

	income_text = CreateDefaultSubobject<UTextRenderComponent>("overlapPercents");
	income_text->SetupAttachment(RootComponent);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AResourceBuilding::IsPlaced() const
{
	return m_IsPlaced;
}

void AResourceBuilding::SetPlaced(bool isPlaced)
{
	m_IsPlaced = isPlaced;
}

bool AResourceBuilding::IsSelected() const
{
	return m_IsSelected;
}

void AResourceBuilding::Select()
{
	m_IsSelected = true;
	ShowIncomeArea(true);
	OnSelected();
}

void AResourceBuilding::Unselect()
{
	m_IsSelected = false;
	ShowIncomeArea(false);
	OnUnselected();
}

void AResourceBuilding::DestroyResourceBuilding()
{
	OnUnselected();
	Destroy();
}

void AResourceBuilding::ShowIncomeArea(bool show)
{
	income_area_circle->SetVisibility(show);
	income_text->SetVisibility(show);
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

	if (! m_IsPlaced)
	{
		auto new_location = CurrentLocation();
		if (new_location != FVector(0, 0, 0))
		{
			SetActorLocation(new_location);
		}
			
	}
	else
	{
		if (! m_PostPlacingActionsApplied)
		{
			cube->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			m_PostPlacingActionsApplied = true;
		}
		// some logic
	}

	auto income = ResourceBuildingIncome();
	if (m_IsPlaced)
	{
		if (income > m_CurrentIncome) // if near rb destroyed
			m_CurrentIncome = income;
	}
	else
		m_CurrentIncome = income;
	income_text->SetText(FString::FromInt(m_CurrentIncome));
}


using Point = std::pair<float, float>;

double GetDistance(const Point &a, const Point &b)
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
TArray<Point> GetCirclePoints(float x, float y, float resource_income_distance)
{
	TArray<Point> result;
	float cell_size = resource_income_distance / 6;

	result.Emplace(x, y + 6 * cell_size);
	result.Emplace(x, y - 6 * cell_size);

	auto FillRange = [&result, x, y, cell_size](int range, int step)
	{
		for (int iter = -range; iter <= range; ++iter)
		{
			result.Emplace(x + iter * cell_size, y + step * cell_size);
			if (step != 0) result.Emplace(x + iter * cell_size, y - step * cell_size);
		}
	};

	FillRange(2, 5);
	FillRange(3, 4);
	FillRange(4, 3);
	FillRange(5, 2);
	FillRange(5, 1);
	FillRange(6, 0);
	auto center_iter = result.Find(std::pair<float, float>(x, y));
	if (center_iter != INDEX_NONE)
		result.RemoveAt(center_iter);

	return result;
};

float AResourceBuilding::ResourceBuildingIncome() const
{
	auto income_area_points = GetCirclePoints(GetActorLocation().X, GetActorLocation().Y, income_area_radius);

	TArray<Point> wrong_points;

	// check if income points is in unreachable location
	for (const auto &point : income_area_points)
	{
		FVector start = GetActorLocation();
		FVector end(point.first, point.second, start.Z);
		/*if (! mIsPlaced)
			DrawDebugPoint(GetWorld(), end, 10.f, FColor::Red);*/

		auto world = GetWorld();
		if (!world)
			continue;
		
		auto nav_system = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
		if (!nav_system)
			continue;
		
		const ANavigationData* nav_data = nav_system->GetNavDataForProps(GetWorld()->GetFirstPlayerController()->GetNavAgentPropertiesRef());
		if (!nav_data)
			continue;
		
		FPathFindingQuery query(nav_system, *nav_data, GetActorLocation(), end);
		if (!Cast<UNavigationSystemV1>(nav_system)->TestPathSync(query))
		{
			if (wrong_points.Find(point) == INDEX_NONE)
			{
				wrong_points.Emplace(point);
				continue;
			}
		}
	}

	// check income based on another rb
	TArray<FHitResult> out_hits;
	FQuat rot = FQuat::Identity;
	FCollisionObjectQueryParams object_query_params;
	object_query_params.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	FCollisionShape collision_shape = FCollisionShape::MakeSphere(income_area_radius*2);

	GetWorld()->SweepMultiByObjectType(out_hits, GetActorLocation(), GetActorLocation() + income_area_radius*2, rot, object_query_params, collision_shape, {});

	for (const auto& hit : out_hits)
	{
		auto actor = hit.Actor.Get();
		if (!Cast<AResourceBuilding>(actor))
			continue;

		float distance_to_another_rb = GetDistanceTo(hit.Actor.Get());
		if (distance_to_another_rb < 1) // hitted by himself
			continue;
		if (distance_to_another_rb > income_area_radius*2) // income circles do not overlap each other
			continue;

		for (const auto &point : income_area_points)
		{
			auto rb_location = hit.Actor.Get()->GetActorLocation();
			if (GetDistance(point, Point(rb_location.X, rb_location.Y)) < income_area_radius)
				if (wrong_points.Find(point) == INDEX_NONE)
				{
					wrong_points.Emplace(point);
					continue;
				}
		}
	}
	float income = income_area_points.Num() - wrong_points.Num();

	return income;
}

FVector AResourceBuilding::CurrentLocation() const
{
	FHitResult hit_result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, hit_result);
	return hit_result.Location;
}

