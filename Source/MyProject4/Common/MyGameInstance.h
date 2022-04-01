// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Http.h"
#include "OnlineJSonSerializer.h"

#include "MyGameInstance.generated.h"



UENUM(BlueprintType)
enum class EMapSelection : uint8
{
	EMS_Waikiki				UMETA(DisplayName = "Waikiki"),
	EMS_SpaceStation		UMETA(DisplayName = "SpaceStation"),
	EMS_MAX					UMETA(DisplayName = "DefaultMax")
};



USTRUCT(BlueprintType)
struct FMasterServerData
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _serverID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _IPAddress;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _serverName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _mapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _currentPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _maxPlayers;

};

USTRUCT(BlueprintType)
struct FDediServerData
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _serverID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _IPAddress;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _serverName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString _mapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _currentPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int _maxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool _bIsStarted;
};

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
		FLinearColor _characterColor = {1.0,1.0,1.0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString _CharacterName="DefaultPlayer";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int _rank =100;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _crossHairHUDAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		TSubclassOf<class UUserWidget> _connectionUIHUDAsset;






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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	EMapSelection _mapSelection;


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


	UFUNCTION(BlueprintCallable)
	void CreateDedicatedServers(FString url);
	
	bool _IsDedicatedServer = false;

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


///////////////////////////////////////////////Master Server////////////////

public:
	
	UFUNCTION(BlueprintCallable)
	void InitializeBranch(const FString& WebServerIP="");
	
	UFUNCTION(BlueprintCallable)
		EMapSelection BranchMap();

	UFUNCTION(BlueprintCallable)
		void RequestExit(bool force = false);


	UFUNCTION(BlueprintCallable)
	void WriteMasterServerInfo(const FString& maserServerSteamIP);

	UFUNCTION(BlueprintCallable)
	void WriteDedicatedServerInfo(const FString& maserServerSteamIP);


	UFUNCTION(BlueprintCallable)
	void ReadMasterServerInfo(const FString& webServerIP);

	UFUNCTION(BlueprintCallable)
	void ReadAllDedicatedServerInfo();


	UFUNCTION(BlueprintCallable)
		void LinkMasterServerInfo(bool enter);

	UFUNCTION(BlueprintCallable)
		void LinkDedicatedServerInfo(const FString& steamSessionID,bool enter);

	UFUNCTION(BlueprintCallable)
		void UpdateDedicatedServerState(const FString& steamSessionID, bool started);

	const FString GetBiggestServer(TArray<FDediServerData> container);

	UFUNCTION(BlueprintCallable)
		const FString GetEnterableWaikikiDediServer();

	UFUNCTION(BlueprintCallable)
		const FString GetEnterableSpacestationDediServer();

	UFUNCTION(BlueprintCallable)
	void LogOutMasterServer();

	UFUNCTION(BlueprintCallable)
	void LogOutDediServer(const FString& currentSessionIP);

	
	void OnBranchProcessComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success);

	void OnWriteProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success);

	void OnReadProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success);

	void OnDediReadProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success);

	UFUNCTION(BlueprintCallable)
	const FString GetMasterServerAddress()const;

	UFUNCTION(BlueprintCallable)
	void ExitAllServer();

	UFUNCTION(BlueprintCallable)
		bool DediCheck();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool _bIsMasterServer;


private:
	UPROPERTY()
	FString _webServerIP;

	TArray<FProcHandle> _childProcess;

	FHttpModule* _http;

	UPROPERTY()
	FString _masterServerAddress;

	TArray<FDediServerData> _waikikiDediServers;

	TArray<FDediServerData> _spaceStationDediServers;
};

