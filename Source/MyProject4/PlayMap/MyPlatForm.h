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

	UFUNCTION(NetMulticast, Unreliable)
	void Transform_Multi(float delta);

	UFUNCTION(NetMulticast, Unreliable)
		void SetTransparency();

	UFUNCTION(NetMulticast, Unreliable)
	void TurnOnToggle_Multi();


	UFUNCTION(BlueprintCallable)
		void GenerateTimer();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StaticMesh")
	class UStaticMeshComponent* _staticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlatformType")
	EPlatFormType _platformType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor")
	class UBoxComponent* _boxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor")
	float _speed;

	UPROPERTY(replicated)
	FLinearColor _platFormColor;

private:

	bool _toggle;

	UMaterialInstanceDynamic* _dynamicMatInstance;

	FTimerHandle _timerHandle;

	FTimerHandle _timerHandle2;

	class AMyWorldTimer* _client;

	class AMyGameStateBase* _gameState;
};
