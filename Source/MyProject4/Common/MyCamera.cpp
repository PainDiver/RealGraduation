// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCamera.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AMyCamera::AMyCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	PrimaryActorTick.bCanEverTick = true;
	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_springArm->SetupAttachment(GetRootComponent());
	_springArm->TargetArmLength = 10.f;
	_springArm->bUsePawnControlRotation = true;

	_springArm->SetupAttachment(GetRootComponent());

	RootComponent = _springArm;

	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	_camera->SetupAttachment(_springArm, USpringArmComponent::SocketName);
	_camera->bUsePawnControlRotation = false;

	_camera->SetupAttachment(_springArm);

}

// Called when the game starts or when spawned
void AMyCamera::BeginPlay()
{
	Super::BeginPlay();


}

