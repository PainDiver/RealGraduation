// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityManipulator.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT4_API UGravityManipulator : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UGravityManipulator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	



	UFUNCTION(BlueprintCallable)
		void SetWorldGravity(float gravity);

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float _gravityScale;
};
