// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyScoreMapGameMode.generated.h"

/**
 * 
 */



UCLASS()
class MYPROJECT4_API AMyScoreMapGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AMyScoreMapGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

	virtual void Logout(AController* Exiting);

	UFUNCTION(BlueprintCallable)
	int GetServerPort();
	

	class AMyScoreMapGameStateBase* _gameState;
	class UMyGameInstance* _gameInstance;
};
