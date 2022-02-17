// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitleController.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API ATitleController : public APlayerController
{
	GENERATED_BODY()
	

public:
	// Sets default values for this actor's properties
	ATitleController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

};
