// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"



ATitleController::ATitleController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATitleController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (_MenuHUDAsset)
		{
			_MenuHUDOverlay = CreateWidget<UUserWidget>(this, _MenuHUDAsset);
			_MenuHUDOverlay->SetVisibility(ESlateVisibility::Visible);
			bShowMouseCursor = true;
			_MenuHUDOverlay->AddToViewport();
		}


	}

}

