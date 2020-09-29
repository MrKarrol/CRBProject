// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBNavigationQueryFilter.h"
#include "ResourceBuildingNavArea.h"

UResourceBNavigationQueryFilter::UResourceBNavigationQueryFilter()
	: UNavigationQueryFilter()
{
	AddExcludedArea(UResourceBuildingNavArea::StaticClass());
}

