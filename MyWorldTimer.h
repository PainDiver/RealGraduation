// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyWorldTimer.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API AMyWorldTimer : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyWorldTimer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CountTime(float DeltaTime);


	UFUNCTION(BlueprintCallable, Category = "Timer")
		inline float GetWorldTimer() { return _worldTimer; }

	UFUNCTION(BlueprintCallable, Category = "Timer")
		inline float GetBossTimer() { return _bossTimer; }

	UFUNCTION(BlueprintCallable, Category = "Timer")
		inline float GetFinalTimer() { return _finalTimer; }

	UFUNCTION(BlueprintCallable, Category = "Timer")
		void SpawnBoss();

	UFUNCTION(BlueprintCallable, Category = "Timer")
		void SetPause(bool pause) { _bPaused = pause; }

	UFUNCTION(BlueprintCallable, Category = "Timer")
		void SetBoss(bool boss) { _bBoss = boss; }

	UFUNCTION(BlueprintCallable, Category = "Timer")
		void SetFinal(bool final) { _bIsFinal = final; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _BossTimeHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		UUserWidget* _BossTimeHUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class AMyEnemy> _enemyAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
		AMyEnemy* _enemy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
		class AMyCharacter* _character;

	FTimerHandle _timerHandle;
	//AMyActorSubClass* MyActor = GetWorld()->SpawnActor<AMyActorSubClass>(AMyActorSubClass::StaticClass());


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
	float _worldTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
	float _bossTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
	float _finalTimer;

private:
	bool _bIsFinal;

	bool _bBoss;

	bool _bPaused;


};
