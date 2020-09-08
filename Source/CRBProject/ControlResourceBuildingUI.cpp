// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlResourceBuildingUI.h"


void UControlResourceBuildingUI::AttachRbToWidget(TScriptInterface<IResourceBuildingInterface> rb)
{
	m_Rb = rb;
}

void UControlResourceBuildingUI::DestroyRb()
{
	m_Rb->DestroyResourceBuilding();
}
