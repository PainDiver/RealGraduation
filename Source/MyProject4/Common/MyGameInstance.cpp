// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFrameWork/GameStateBase.h"
#include "GameFramework/OnlineSession.h"
#include "OnlineSubsystemTypes.h"
#include "../ReadyMap/ReadyRoomGameStateBase.h"
#include "../PlayMap/MyGameStateBase.h"
#include "../PlayMap/MyPlayerState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/Character.h"
#include "MySaveGame.h"
#include "SocketSubsystem.h"


#include "GenericPlatform/GenericPlatformProcess.h"


const uint8 UMyGameInstance::MAX_PLAYER = 8;
const FName UMyGameInstance::SESSION_NAME = TEXT("TwistRunGame");


void UMyGameInstance::Init()
{
	if (Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(HOST_MIGRATION, 0)))
	{
		UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);
	}


	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem != nullptr)
	{
		_sessionInterface = subsystem->GetSessionInterface();
		if (_sessionInterface.IsValid())
		{			
			_sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionComplete);
			_sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,&UMyGameInstance::OnCreateSessionComplete);
			_sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionComplete);
			_sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnJoinSessionComplete);
			
		}
	}

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMyGameInstance::OnNetworkFailure);
	}
}

void UMyGameInstance::Host(const bool& LanCheck)
{
	if (_sessionInterface.IsValid())
	{
		_lanCheck = LanCheck;

		auto existingSession = _sessionInterface->GetNamedSession(SESSION_NAME);
		if (existingSession != nullptr)
		{
			_sessionInterface->DestroySession(SESSION_NAME);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Destroy Session");
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue,"Session Created");
			CreateSession();
		}
	}
}

void UMyGameInstance::CreateSession()
{
	if (_sessionInterface.IsValid())
	{
		if (!_lastSettings.IsValid())
		{
			_lastSettings = MakeShareable(new FOnlineSessionSettings());
			_lastSettings->bIsLANMatch = _lanCheck;
			_lastSettings->NumPublicConnections = MAX_PLAYER;
			_lastSettings->bShouldAdvertise = true;
			_lastSettings->bUseLobbiesIfAvailable = true;
			_lastSettings->bUsesPresence = true;
			_lastSettings->bAllowJoinInProgress = true;
			_lastSettings->bAllowJoinViaPresence = true;
			_lastSettings->bAllowInvites = true;
			_lastSettings->Settings.Emplace(SESSION_NAME);
			_lastSettings->bUsesStats = true;

			_lastSettings->Set(SETTING_MAPNAME, SESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		}
		_currentSessionName = SESSION_NAME;
		_sessionInterface->CreateSession(0, SESSION_NAME, *(_lastSettings));	
	
	}
	
}


void UMyGameInstance::Join()
{

	UE_LOG(LogTemp, Warning, TEXT("Join Session Try"));

	if (!_sessionInterface.IsValid())
	{
		return;
	}
	if (!_sessionSearch.IsValid())
	{
		return;
	}

	if (_sessionSearch->SearchResults.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session with Search Result"));
		_sessionInterface->JoinSession(0, SESSION_NAME, _sessionSearch->SearchResults[_selectedIndex]);
	}
}

void UMyGameInstance::Find()
{
	if (_sessionSearch.IsValid())
	{
		_sessionSearch.Reset();
	}
	_sessionSearch = MakeShareable(new FOnlineSessionSearch());

	if(_sessionSearch.IsValid())
	{
		_sessionSearch->MaxSearchResults = 100;
		_sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		_sessionSearch->bIsLanQuery = _lanCheck;

		_sessionInterface->FindSessions(0, _sessionSearch.ToSharedRef());	
		UE_LOG(LogTemp, Warning, TEXT("Session Searched"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No SessionSearch"));
	}

}

TArray<FSessionInfo> UMyGameInstance::GetSessions()
{
	_sessionInfo.Empty();
	auto sessionName = SESSION_NAME.ToString();
	for (int i = 0; i < _sessionSearch->SearchResults.Num(); i++)
	{
		const auto& session = _sessionSearch->SearchResults[i];
		if (session.Session.SessionSettings.Get(SETTING_MAPNAME, sessionName) )
		{
			_sessionInfo.Add(FSessionInfo{ session.PingInMs,MAX_PLAYER - session.Session.NumOpenPublicConnections,session.Session.OwningUserName });
		}
	}
	return _sessionInfo;
}



void UMyGameInstance::SelectSession(const int& index)
{
	_selectedIndex=index;
}

void UMyGameInstance::DestroySession_Implementation()
{
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Destroy Session");
	_sessionInterface->DestroySession(_currentSessionName);
}


void UMyGameInstance::OnCreateSessionComplete(FName sessionName, bool success)
{
	UWorld* world = GetWorld();

	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
	world->ServerTravel("/Game/map/ReadyMap?listen");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Server Travel");
	CheckOpenPublicConnection(true);
}

void UMyGameInstance::OnDestroySessionComplete(FName sessionName, bool success)
{
	BindAltF4(false);
	if (_exitRequest)
	{
		//UGameplayStatics::DeleteGameInSlot(HOST_MIGRATION, 0);
		FGenericPlatformMisc::RequestExit(true);
	}
	if (success)
	{
		ReturnToMainMenu();
	}
}


void UMyGameInstance::OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error)
{
	BindAltF4(false);
	

	auto existingSession = _sessionInterface->GetNamedSession(SESSION_NAME);
	

	
	if (existingSession != nullptr && _sessionInterface)
	{
		UE_LOG(LogTemp, Warning, TEXT("Migrating Start"));
		if (_migration._IsHost == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "you're new host");
			_sessionInterface->OnDestroySessionCompleteDelegates.Clear();
			_sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionServerMigration);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "you're client traveling to new host");
			_sessionInterface->OnDestroySessionCompleteDelegates.Clear();
			_sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionClientMigration);
		}
		_sessionInterface->DestroySession(SESSION_NAME);
		UE_LOG(LogTemp, Warning, TEXT("Migrating Done"));
	}
	else
	{
		ReturnToMainMenu();
		UE_LOG(LogTemp, Warning, TEXT("Returning To Menu"));
	}
}


void UMyGameInstance::OnFindSessionComplete(bool success)
{
	if (success &&_sessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& searchResult: _sessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("found : %s"), *searchResult.GetSessionIdStr());
		}
	}

}

void UMyGameInstance::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	//SteamUser()->GetSteamID(), SteamP2pAddr->GetPort()

	//SteamUser()->GetSteamID(), SessionInfo->SteamP2PAddr->GetPort();
	//SessionInfo->HostAddr, SessionInfo->HostAddr->GetPort();


	FString address;
	if(!_sessionInterface->GetResolvedConnectString(sessionName, address))
	{
		return;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString("Joining :")+address);
	}

	APlayerController* playerController = GetFirstLocalPlayerController();

	_currentSessionName = sessionName;
	_currentSessionAddress = address;

	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
	playerController->ClientTravel(_currentSessionAddress, ETravelType::TRAVEL_Absolute);
	
}

void UMyGameInstance::UpdateGameSession_Implementation(FName sessionName, bool advertise)
{
	if (_sessionInterface != nullptr)
	{
		IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
		_sessionInterface = subsystem->GetSessionInterface();
	}
	if(_lanCheck)
	{ 
		return;
	}
	if (_lastSettings)
	{
		FOnlineSessionSettings updatedSession = *(_lastSettings);
		updatedSession.bShouldAdvertise = advertise;
		updatedSession.bAllowJoinInProgress = advertise;
		updatedSession.NumPublicConnections = UGameplayStatics::GetGameState(GetWorld())->PlayerArray.Num();
		

		_sessionInterface->UpdateSession(_currentSessionName, updatedSession);

		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "Session Updating");
	}
}


void UMyGameInstance::OnDestroySessionServerMigration(FName sessionName, bool success)
{

	if (!_lastSettings.IsValid())
	{
		_lastSettings = MakeShareable(new FOnlineSessionSettings());
		_lastSettings->bIsLANMatch = _lanCheck;
		_lastSettings->NumPublicConnections =MAX_PLAYER;
		_lastSettings->bShouldAdvertise = true;
		_lastSettings->bUseLobbiesIfAvailable = true;
		_lastSettings->bUsesPresence = true;
		_lastSettings->bAllowJoinInProgress = true;
		_lastSettings->bAllowJoinViaPresence = true;
		_lastSettings->bAllowInvites = true;
		_lastSettings->Settings.Emplace(SESSION_NAME);
		_lastSettings->bUsesStats = true;

		_lastSettings->Set(SETTING_MAPNAME, SESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		_lastSettings->Set(SETTING_CUSTOMSEARCHINT1, _migration._hostName, EOnlineDataAdvertisementType::ViaOnlineService);
	
	}
	GetCurrentSessionInterface()->OnCreateSessionCompleteDelegates.Clear();
	GetCurrentSessionInterface()->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnMigrationCreateSessionComplete);

	FTimerHandle delay;
	GetTimerManager().SetTimer(delay, FTimerDelegate::CreateLambda([&]()
	{
		GetCurrentSessionInterface()->CreateSession(0,SESSION_NAME, *(_lastSettings));
	}
	), 0.5,false);
}

void UMyGameInstance::OnMigrationCreateSessionComplete(FName sessionName, bool success)
{
	GetCurrentSessionInterface()->OnCreateSessionCompleteDelegates.Clear();
	GetCurrentSessionInterface()->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnCreateSessionComplete);

	UWorld* world = GetWorld();
	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("/Game/map/%s?listen"),*(_migration._currentMap));
	world->ServerTravel("/Game/map/"+_migration._currentMap+"?listen");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Server Travel");
	CheckOpenPublicConnection(true);

	GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.Clear();
	GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionComplete);
}


void UMyGameInstance::OnDestroySessionClientMigration(FName sessionName, bool success)
{
	GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.Clear();
	GetCurrentSessionInterface()->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstance::OnDestroySessionComplete);

	if (success)
	{
		_sessionSearch->MaxSearchResults = 100;
		_sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		_sessionSearch->bIsLanQuery = _lanCheck;
		_sessionSearch->TimeoutInSeconds = 2.0f;
		GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.Clear();
		GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionMigration);
		GetCurrentSessionInterface()->FindSessions(0, _sessionSearch.ToSharedRef());
	}	
}



void UMyGameInstance::OnFindSessionMigration(bool success)
{
	GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.Clear();
	GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionComplete);
	
	if (success && _sessionSearch.IsValid() && _sessionSearch->SearchResults.Num())
	{
		auto hostName = _migration._hostName;
		UE_LOG(LogTemp, Warning, TEXT("hostName :%s"), *hostName);
		auto sessionName = SESSION_NAME.ToString();
		for (const auto& session : _sessionSearch->SearchResults)
		{
			if (session.Session.SessionSettings.Get(SETTING_CUSTOMSEARCHINT1, hostName) && session.Session.SessionSettings.Get(SETTING_MAPNAME,sessionName))
			{
				GetCurrentSessionInterface()->JoinSession(0, _currentSessionName, session);
				return;
			}
		}
	}

}



void UMyGameInstance::MyServerTravel_Implementation(const FString& mapPath, const FString& additionalOption, bool bAbsolute)
{
	//Default paths to Maps folder and GameModes folder
	//Game/map/simplemap?game=Game/path?listen
	//game = / Game / Blueprints / GameModes / TDMGM_BP.TDMGM_BP_C

	FString travelURL = mapPath + additionalOption;
	UGameplayStatics::GetGameMode(GetWorld())->ProcessServerTravel(travelURL, bAbsolute);

	
	//TArray<UUserWidget*> widgets;
	//UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), widgets, UUserWidget::StaticClass());
	//for (auto widget : widgets)
	//{
	//	widget->RemoveFromParent();
	//}

	//GetWorld()->SeamlessTravel(travelURL, bAbsolute);

}

bool UMyGameInstance::CheckOpenPublicConnection(bool isIn)
{
	
	if (!GetCurrentSessionInterface())
	{
		IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
		if (subsystem != nullptr)
		{
			_sessionInterface = subsystem->GetSessionInterface();
		}
	}
	FNamedOnlineSession* session = _sessionInterface->GetNamedSession(_currentSessionName);
	
	if (!session)
	{
		return false;
	}
	if (isIn)
	{
		if (session->NumOpenPublicConnections <=0)
		{
			return false;
		}
		session->NumOpenPublicConnections -= 1;
		return true;
	}
	else
	{
		session->NumOpenPublicConnections += 1;
		return true;
	}

}

void UMyGameInstance::FindLowestPingAndNotify_Implementation()
{
	auto gameState = Cast<AGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	
	auto readyRoomGameState = Cast<AReadyRoomGameStateBase>(gameState);
	if (readyRoomGameState)
	{
		readyRoomGameState->FindAllPlayerControllerHideAllWidget();
	}
	
	auto myGameState = Cast<AMyGameStateBase>(gameState);
	bool gameStarted = false;
	if (myGameState)
	{
		gameStarted = myGameState->_bGameStarted;
	}


	if (gameState->PlayerArray.IsValidIndex(1) && GetWorld()->IsServer())
	{
		auto players = gameState->PlayerArray;
		players.Sort([](const APlayerState& a, const APlayerState& b)
			{
				return a.GetPing() < b.GetPing();
			});

		APlayerState* lowPingPlayer;
		auto host = Cast<AMyPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AMyPlayerState>());
		if (players[0] == host)
		{
			lowPingPlayer = players[1];
		}
		else
		{
			lowPingPlayer = players[0];
		}
		auto lowPing = Cast<AMyPlayerState>(lowPingPlayer);
		auto mapname = GetWorld()->GetMapName();
		mapname.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		auto name = lowPing->GetPlayerName();
		if (name.IsEmpty())
		{
			name = "NewHost";
		}

		lowPing->SetMigrationInfo({true, name,mapname,gameStarted});
		host->SaveBeforeExit();
		
		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "Found low ping "+ mapname);
	}

	//Cast<AMyPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AMyPlayerState>())->SaveBeforeExit();

}



void UMyGameInstance::BindAltF4(bool on)
{
	UGameViewportClient* viewPortClient = GetGameViewportClient();

	if (on)
	{
		if (viewPortClient)
		{
			if (viewPortClient->OnWindowCloseRequested().IsBound())
			{
				viewPortClient->OnWindowCloseRequested().Unbind();
			}
			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool
				{	
					CheckOpenPublicConnection(false);
					FindLowestPingAndNotify();
					_exitRequest = true;
					FTimerHandle delay;
					GetTimerManager().SetTimer(delay, FTimerDelegate::CreateLambda([&]() {DestroySession(); }), 1.5, false);			
					return false;
				});
		}
	}
	else
	{
		if (viewPortClient)
		{
			if (viewPortClient->OnWindowCloseRequested().IsBound())
			{
				viewPortClient->OnWindowCloseRequested().Unbind();
			}
			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool {return true; });
		}
	}
}


FString UMyGameInstance::GetMyIpAddress()
{
	FString IpAddr("NONE");
	bool canBind = false;
	TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);
	if (LocalIp->IsValid())
	{
		IpAddr = LocalIp->ToString(false);
	}
	return IpAddr;
}


int UMyGameInstance::CreateNewProcess(FString url, FString Attributes)
{
	static int port = GetWorld()->URL.Port;
	static int childPort = port;


	FString ExecuterPath = FPaths::ProjectDir()+"Saved\\StagedBuilds\\WindowsNoEditor\\MyProject4.exe";

	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecuterPath);

	uint32 OutProcessID = 0;

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*ExecuterPath, *Attributes, true, false, false, &OutProcessID, 0, nullptr,nullptr);	
	
	childPort++;
	_childProcesses.Add({ ProcHandle, childPort });

	return ProcHandle.IsValid() ? OutProcessID : -1;
}

