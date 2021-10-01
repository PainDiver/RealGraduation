// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPlatForm.generated.h"


UENUM(BlueprintType)
enum class EPlatFormType : uint8
{
	EPF_RotatingPlatform	UMETA(DisplayName = "RotatingPlatform"),
	EPF_TranslatingPlatform	UMETA(DisplayName = "TranslatingPlatform"),
	EPF_InvisiblePlatform	UMETA(DisplayName = "InvisiblePlatform"),
	EPF_MAX					UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class MYPROJECT4_API AMyPlatForm : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyPlatForm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Transform(float deltaTime);

	UFUNCTION()
	void TurnOnToggle();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "StaticMesh")
	class UStaticMeshComponent* _staticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlatformType")
	EPlatFormType _platformType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor")
	class UBoxComponent* _BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor")
	float _speed;

	bool _toggle;

	FTimerHandle _timerHandle;

	UMaterialInstanceDynamic* _dynamicMatInstance;

	float _alphaStack;

	FLinearColor _platFormColor;
};
