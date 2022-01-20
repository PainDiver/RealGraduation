// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TitleActor.generated.h"

UCLASS()
class MYPROJECT4_API ATitleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATitleActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "staticMesh")
	class UStaticMeshComponent* _staticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	float Force;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	float time;


	FTimerHandle _timerHandle;



	void JumpOnTime();

	
};
