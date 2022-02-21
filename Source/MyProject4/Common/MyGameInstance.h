// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MyGameInstance.generated.h"


DECLARE_DELEGATE(FDisconnectDelegate)

/**
 * 
 */
USTRUCT(Atomic, BlueprintType)
struct FCharacterInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor _characterColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString _CharacterName="DefaultPlayer";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int _number =100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool initialized = false;
};


USTRUCT(Atomic, BlueprintType)
struct FSessionInfo
{
	GENERATED_USTRUCT_BODY()

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


	//UI Resources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _readyHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _chattingHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _startHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _inventoryHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _spectateHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _finalTimerHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _scoreHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _startTimerHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _UIHUDAsset;



	//In Game Values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInfo")
	bool _gameStarted;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInfo")
	FCharacterInfo _characterInfo;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInfo")
	TArray<FCharacterInfo> _winner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	TArray<FSessionInfo> _sessionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
		bool _lanCheck = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
		int _numOfPlayerInCurrentSession;


	FName _currentSessionName;


	int _selectedIndex;


	FString _currentSessionAddress;

	TSharedPtr<FOnlineSessionSettings> _lastSettings;

	TSharedPtr<class FOnlineSessionSearch> _sessionSearch;

	static FName SESSION_NAME;

	static uint8 MAX_PLAYER;


	FDisconnectDelegate _disconnectDelegate;


	UFUNCTION(BlueprintCallable, Category = "Session")
		void Host(const bool& LanCheck);

	UFUNCTION(BlueprintCallable, Category = "Session")
		void Join();

	UFUNCTION(BlueprintCallable, Category = "Session")
		void Find();

	UFUNCTION()
		void CreateSession();


	UFUNCTION(BlueprintCallable, Category = "Session")
		void SelectSession(const int& index);

	UFUNCTION(BlueprintCallable, Category = "Session")
		TArray<FSessionInfo> GetSessions();

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void UpdateGameSession(FName sessionName, bool advertise = true);

	UFUNCTION(Server,Reliable,BlueprintCallable, Category = "Transition")
		void MyServerTravel(const FString& mapName, const FString& additionalOption, bool bAbsolute);


	UFUNCTION(BlueprintCallable)
		void DestroySession();

	bool CheckOpenPublicConnection(bool isIn);

	
	inline IOnlineSessionPtr GetCurrentSessionInterface() { return _sessionInterface; }

	void OnCreateSessionComplete(FName sessionName, bool success);

	void OnDestroySessionComplete(FName sessionName, bool success);


	void OnFindSessionComplete(bool success);

	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	void OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error);

	void OnDestroySessionServerMigration(FName sessionName, bool success);

	void OnDestroySessionClientMigration(FName sessionName, bool success);

	void OnFindSessionMigration(bool success);

	bool _exitRequest =false;

private:
	//callback for server hosting, joining, destroying
	
	IOnlineSessionPtr _sessionInterface;

	

	

};
