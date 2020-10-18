// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBuilding.h"

#include "Components/DecalComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "ResourceBuildingNavArea.h"

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
	DrawDebugPoint(GetWorld(), FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 130), 10.f, FColor::Red);

	const auto world = GetWorld();
	if (!world)
		return 0;

	const auto nav_system = Cast<UNavigationSystemV1>(GetWorld()->GetNavigationSystem());
	if (!nav_system)
		return 0;

	const ANavigationData* nav_data = nav_system->GetNavDataForProps(GetWorld()->GetFirstPlayerController()->GetNavAgentPropertiesRef());
	if (!nav_data)
		return 0;

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("num: %d"), filter->GetExcludeFlags()));

	FVector start = GetActorLocation();

	// find another resource buildings
	TArray<FHitResult> out_hits;
	FQuat rot = FQuat::Identity;
	FCollisionObjectQueryParams object_query_params;
	object_query_params.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	FCollisionShape collision_shape = FCollisionShape::MakeSphere(income_area_radius * 2);

	GetWorld()->SweepMultiByObjectType(out_hits, GetActorLocation(), GetActorLocation() + income_area_radius * 2, rot, object_query_params, collision_shape, {});


	TArray<Point> wrong_points;
	auto income_area_points_size = income_area_points.Num();
	{
		auto income_area_points_dub = income_area_points;
		for (const auto &point : income_area_points)
		{
			FVector end(point.first, point.second, start.Z);

			FPathFindingQuery query(nav_system, *nav_data, start, end);
			if (!nav_system->TestPathSync(query))
			{
				income_area_points_dub.Remove(point);
			}
		}
		income_area_points = std::move(income_area_points_dub);
	}
	

	
	for (const auto& hit : out_hits)
	{
		auto actor = hit.Actor.Get();
		if (!Cast<AResourceBuilding>(actor))
			continue;

		const float distance_to_another_rb = GetDistanceTo(hit.Actor.Get());
		if (distance_to_another_rb < 1) // hitted by himself
			continue;
		if (distance_to_another_rb > income_area_radius*2) // income circles do not overlap each other
			continue;

		auto income_area_points_dub = income_area_points;
		for (const auto &point : income_area_points)
		{
			auto rb_location = hit.Actor.Get()->GetActorLocation();
			if (GetDistance(point, Point(rb_location.X, rb_location.Y)) < income_area_radius)
				income_area_points_dub.Remove(point);
		}
		income_area_points = std::move(income_area_points_dub);
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("income: %f, good: %d, wrong: %d"), m_CurrentIncome, income_area_points.Num(), income_area_points_size - income_area_points.Num()));

	//float income = float(income_area_points.Num() - wrong_points.Num()) / income_area_points.Num() * 100;
	float income = float(income_area_points.Num()) / income_area_points_size * 100;

	return income;
}

FVector AResourceBuilding::CurrentLocation() const
{
	FHitResult hit_result;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel({}, false, hit_result);
	return hit_result.Location;
}

