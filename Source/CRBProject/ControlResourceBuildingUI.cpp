// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlResourceBuildingUI.h"


void UControlResourceBuildingUI::attachRbToWidget(TScriptInterface<IResourceBuildingInterface> newRb)
{
	rb = newRb;
}

void UControlResourceBuildingUI::destroyRB()
{
	rb->destroyRB();
}
