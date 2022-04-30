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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
		TSubclassOf<class AMyEnemy> _enemyAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
		float _bossTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "timer")
		float _finalTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AIRespawn")
		FVector _AIRespawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	float _StartTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TImer")
	float _abandonUnconnectedPlayersWithIn;
	
	
	UFUNCTION(BlueprintCallable)
	void SetIsFinal(const bool& Final) { _bIsFinal = Final; }

	UFUNCTION(BlueprintCallable)
	inline bool GetIsFinal() { return _bIsFinal; }

	UFUNCTION(BlueprintCallable)
	void IncreaseNumOfFinished() { _NumOfFinished++; }

	UFUNCTION(BlueprintCallable)
	inline uint8 GetNumOfFinished() { return _NumOfFinished; }

	UFUNCTION(BlueprintCallable)
		TArray<FTimerHandle> GetTickTimers();

	UFUNCTION(BlueprintCallable)
		void RegisterTickTimers(const FTimerHandle& timer);


	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)override;

	virtual void Logout(AController* Exiting)override;

private:
	void CountEnemyTimer(const float& DeltaTime);

	void CountStartTimer(const float& DeltaTime);

	void CountFinalTimer(const float& DeltaTime);

	void SpawnBoss();



	
	uint8 _NumOfFinished;

	class AMyEnemy* _enemy;

	class AMyGameStateBase* _gameState;

	bool _bIsFinal;

	bool _bPaused;

	bool _bBoss;

	bool _bIsStarted;

	FTimerHandle _timerForCheckConnection;

	FTimerHandle _timerForControl;

	FTimerHandle _delayTimer;


	UPROPERTY()
		TArray<FTimerHandle> _tickTimers;
};
