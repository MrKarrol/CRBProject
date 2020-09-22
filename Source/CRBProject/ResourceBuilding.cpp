// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "ResourceBuildingNavArea.h"
#include "ResourceBNavigationQueryFilter.h"

// Sets default values
AResourceBuilding::AResourceBuilding()
{
	// In your constructor
	scene_root = CreateDefaultSubobject<USceneComponent>(TEXT("scene_root"));
	RootComponent = scene_root;

	cube = CreateDefaultSubobject<UStaticMeshComponent>("cube");
	cube->SetupAttachment(RootComponent);
	cube->SetRelativeLocation(FVector(0, 0, 0));
	
	cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create a decal in the world to show the cursor's location
	income_area_circle = CreateDefaultSubobject<UDecalComponent>("income_area_circle");
	income_area_circle->SetupAttachment(RootComponent);
	income_area_circle->SetRelativeLocation(FVector(0, 0, 0));

	income_area = CreateDefaultSubobject<UResourceBuildingNavModComponent>("income_area");
	income_area->SetAreaClass(UResourceBuildingNavArea::StaticClass());

	income_text = CreateDefaultSubobject<UTextRenderComponent>("income_text");
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

// Acceptable angles - integer obtained by dividing 90 on some integer, i.e. 1, 5, 10 etc.
TArray<Point> GetCirclePoints2(const Point &center, const float resource_income_distance, const int linearDepth = 10, const int addedAngle = 15)
{
	TArray<Point> result;
	
	const float step = resource_income_distance / linearDepth;
	for (int depth = 0; depth < linearDepth; ++depth)
	{
		const float radius = resource_income_distance - step * depth;
		for (int angle = 0; angle < 90; angle += addedAngle)
		{
			const float xInc = radius * FMath::Sin(90 - angle);
			const float yInc = radius * FMath::Sin(angle);

			result.Emplace(center.first + xInc, center.second + yInc);
			result.Emplace(center.first + yInc, center.second - xInc);
			result.Emplace(center.first - xInc, center.second - yInc);
			result.Emplace(center.first - yInc, center.second + xInc);
		}
	}
		
	return result;
}

TArray<Point> GetCirclePoints3(const Point &center, const float resource_income_distance, const int depth = 20)
{
	TArray<Point> result;

	const float step = resource_income_distance / depth;
	for (int depth_index = 0; depth_index < depth; ++depth_index)
	{
		const float y = center.second + step * depth_index;
		const float y_minus = center.second - step * depth_index;
		const float x = FMath::Sqrt(FMath::Pow(resource_income_distance, 2) - FMath::Pow(step * depth_index, 2)) + center.first;

		int step_inc = 1;
		while (center.first + step * step_inc < x)
		{
			result.Emplace(center.first + step_inc * step, y);
			result.Emplace(center.first - step_inc * step, y);
			if (depth_index != 0)
			{
				result.Emplace(center.first + step_inc * step, y_minus);
				result.Emplace(center.first - step_inc * step, y_minus);
			}
			++step_inc;
		}
	}

	return result;
}

float AResourceBuilding::ResourceBuildingIncome() const
{
	auto income_area_points = GetCirclePoints3({ GetActorLocation().X, GetActorLocation().Y }, income_area_radius);
	/*for (auto point : income_area_points)
		DrawDebugPoint(GetWorld(), FVector(point.first, point.second, GetActorLocation().Z), 10.f, FColor::Red);*/


	auto world = GetWorld();
	if (!world)
		return 0;

	auto nav_system = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	if (!nav_system)
		return 0;

	const ANavigationData* nav_data = nav_system->GetNavDataForProps(GetWorld()->GetFirstPlayerController()->GetNavAgentPropertiesRef());
	if (!nav_data)
		return 0;

	auto filter = UResourceBNavigationQueryFilter::GetQueryFilter<UResourceBNavigationQueryFilter>(*nav_data, UResourceBNavigationQueryFilter::StaticClass());
	
	
	FVector start = GetActorLocation();

	// find another resource buildings
	/*TArray<FHitResult> out_hits;
	FQuat rot = FQuat::Identity;
	FCollisionObjectQueryParams object_query_params;
	object_query_params.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	FCollisionShape collision_shape = FCollisionShape::MakeSphere(income_area_radius * 2);

	GetWorld()->SweepMultiByObjectType(out_hits, GetActorLocation(), GetActorLocation() + income_area_radius * 2, rot, object_query_params, collision_shape, {});
*/

	TArray<Point> wrong_points;

	for (const auto &point : income_area_points)
	{
		FVector end(point.first, point.second, start.Z);

		FPathFindingQuery query(nav_system, *nav_data, start, end, filter);
		if (!Cast<UNavigationSystemV1>(nav_system)->TestPathSync(query))
		{
			if (wrong_points.Find(point) == INDEX_NONE)
			{
				wrong_points.Emplace(point);
				continue;
			}
		}
	}

	
	//for (const auto& hit : out_hits)
	//{
	//	auto actor = hit.Actor.Get();
	//	if (!Cast<AResourceBuilding>(actor))
	//		continue;

	//	float distance_to_another_rb = GetDistanceTo(hit.Actor.Get());
	//	if (distance_to_another_rb < 1) // hitted by himself
	//		continue;
	//	if (distance_to_another_rb > income_area_radius*2) // income circles do not overlap each other
	//		continue;

	//	for (const auto &point : income_area_points)
	//	{
	//		auto rb_location = hit.Actor.Get()->GetActorLocation();
	//		if (GetDistance(point, Point(rb_location.X, rb_location.Y)) < income_area_radius)
	//			if (wrong_points.Find(point) == INDEX_NONE)
	//			{
	//				wrong_points.Emplace(point);
	//				continue;
	//			}
	//	}
	//}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("good: %d, wrong: %d"), income_area_points.Num(), wrong_points.Num()));

	float income = float(income_area_points.Num() - wrong_points.Num()) / income_area_points.Num() * 100;

	return income;
}

FVector AResourceBuilding::CurrentLocation() const
{
	FHitResult hit_result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, hit_result);
	return hit_result.Location;
}

