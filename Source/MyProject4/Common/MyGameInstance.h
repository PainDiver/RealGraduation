// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MyGameInstance.generated.h"


/**
 * 
 */
USTRUCT(Atomic, BlueprintType)
struct FCharacterInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor _characterMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString _CharacterName="DefaultPlayer";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int _number =100;
};


USTRUCT(Atomic, BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _ping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _publicConnection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _hostName;

};



UCLASS()
class MYPROJECT4_API UMyGameInstance : public UGameInstance
{	
	GENERATED_BODY()
public:

	virtual void Init();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void Host(bool LanCheck);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void Join();

	UFUNCTION(BlueprintCallable,Category ="Session")
	void Find();

	UFUNCTION()
	void CreateSession();

	UFUNCTION(BlueprintCallable,Category = "Session")
	void SelectSession(int index);


	UFUNCTION(BlueprintCallable,Category = "Session")
	TArray<FSessionInfo> GetSessions();


	void OnCreateSessionComplete(FName sessionName, bool success);

	void OnDestroySessionComplete(FName sessionName, bool success);

	void OnFindSessionComplete(bool success);

	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	void OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error);

	
	
	TSharedPtr<class FOnlineSessionSearch> _sessionSearch;

	IOnlineSessionPtr _sessionInterface;

	TOptional<int> _selectedIndex;

	bool _LanCheck;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> _ReadyHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> _ChattingHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> _StartHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _inventoryHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _SpectateHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _FinalTimerHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _ScoreHUDAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _StartTimerHUDAsset;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInfo")
		bool _gameStarted;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category ="GameInfo")
	FCharacterInfo _characterInfo;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInfo")
		TArray<FCharacterInfo> _winner;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
		TArray<FSessionInfo> _sessionInfo;



};
