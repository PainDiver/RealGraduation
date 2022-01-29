// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyItem.generated.h"

UCLASS()
class MYPROJECT4_API AMyItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	class USphereComponent* _collisionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	class UStaticMeshComponent* _mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
		bool _bRotate;

	UPROPERTY(EditAnywhere, Category = "Toggle")
		float _rotatingRate;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
