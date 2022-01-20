// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyProject4GameModeBase.generated.h"


UCLASS()
class MYPROJECT4_API AMyProject4GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMyProject4GameModeBase();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime);


	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;

	virtual void Logout(AController* Exiting)override;

	void CountTime(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Timer")
		void SpawnBoss();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSubclassOf<class AMyEnemy> _enemyAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
		float _bossTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
		float _finalTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIRespawn")
		FVector _AIRespawnPoint;

	

	


	FTimerHandle _MapChangeHandler;
	
	uint8 _NumOfFinished;

	class AMyEnemy* _enemy;

	class AMyGameStateBase* _gameState;

	bool _gameStarted;

	bool _bIsFinal;

	bool _bPaused;

	bool _bBoss;

	bool _bIsStarted;

	float _StartTimer;

};
