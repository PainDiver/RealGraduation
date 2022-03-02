// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MyGameInstance.generated.h"




USTRUCT(Atomic, BlueprintType)
struct FMigrationPacket
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	bool _IsHost = false;

	UPROPERTY()
	FString _hostName = "NewHost";

	UPROPERTY()
	FString _currentMap = "/Game/map/ReadyMap";

	UPROPERTY()
	bool _IsStarted = false;

};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool OnRepToggle = false;
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
	bool _gameEnterClosed;


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
		FMigrationPacket _migration;

	FName _currentSessionName;

	int _selectedIndex;

	FString _currentSessionAddress;

	TSharedPtr<FOnlineSessionSettings> _lastSettings;

	TSharedPtr<class FOnlineSessionSearch> _sessionSearch;

	const static FName SESSION_NAME;

	const static uint8 MAX_PLAYER;



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

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void FindLowestPingAndNotify();

	UFUNCTION(Client,Reliable,BlueprintCallable)
		void DestroySession();

	UFUNCTION(BlueprintCallable)
	bool CheckOpenPublicConnection(bool isIn);

	
	FString GetMyIpAddress();

	void SetMigartionInfo(FMigrationPacket info) { _migration = info; }

	inline FMigrationPacket GetMigrationInfo() { return _migration; }

	inline IOnlineSessionPtr GetCurrentSessionInterface() { return _sessionInterface; }

	void BindAltF4(bool on);

	bool _exitRequest =false;



private:
	//callback for server hosting, joining, destroying


	void OnCreateSessionComplete(FName sessionName, bool success);

	void OnDestroySessionComplete(FName sessionName, bool success);

	void OnMigrationCreateSessionComplete(FName sessionName, bool success);

	void OnFindSessionComplete(bool success);

	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	void OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error);

	void OnDestroySessionServerMigration(FName sessionName, bool success);

	void OnDestroySessionClientMigration(FName sessionName, bool success);

	void OnFindSessionMigration(bool success);


	IOnlineSessionPtr _sessionInterface;

	FString _saveSlot;

};
