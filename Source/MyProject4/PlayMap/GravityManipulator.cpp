// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityManipulator.h"

// Sets default values
UGravityManipulator::UGravityManipulator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
}

// Called when the game starts or when spawned
void UGravityManipulator::BeginPlay()
{
	Super::BeginPlay();
}



void UGravityManipulator::SetWorldGravity(float gravity)
{
	AWorldSettings* MyWorldSetting = GetWorld()->GetWorldSettings();
	MyWorldSetting->bGlobalGravitySet = true;
	MyWorldSetting->GlobalGravityZ = gravity;
}