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
#include "GenericPlatform/GenericPlatformMisc.h"
#include "JsonObjectConverter.h"

#include "GenericPlatform/GenericPlatformProcess.h"



const uint8 UMyGameInstance::MAX_PLAYER = 8;
const FName UMyGameInstance::SESSION_NAME = TEXT("TwistRunGame");


void UMyGameInstance::Init()
{
	_bIsMasterServer = false;

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


	_http = &FHttpModule::Get();

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

			//_lastSettings->NumPublicConnections = MAX_PLAYER;
			//_lastSettings->bIsLANMatch = false;       // If you want a lan match set this to true.
			//_lastSettings->bUsesPresence = false;    // Dedicated servers should always have this value set to false.
			//_lastSettings->bShouldAdvertise = true;
			//_lastSettings->bIsDedicated = true;
			//_lastSettings->Settings.Emplace(SESSION_NAME);
			//_lastSettings->BuildUniqueId = 1;
			//_lastSettings->bAllowInvites = true;

			_lastSettings->Set(SETTING_MAPNAME, SESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		}
		_IsDedicatedServer = false;
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
		//_sessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
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
	
	//world->ServerTravel("/Game/map/ReadyMap?listen");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Server Travel");
	//CheckOpenPublicConnection(true);
}

void UMyGameInstance::OnDestroySessionComplete(FName sessionName, bool success)
{
	//BindAltF4(false);
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
	//BindAltF4(false);
	ReturnToMainMenu();
	UE_LOG(LogTemp, Warning, TEXT("Returning To Menu"));
}


void UMyGameInstance::OnFindSessionComplete(bool success)
{
	if (success)
	{
		_IsDedicatedServer = false;
		if (success && _sessionSearch.IsValid())
		{
			for (const FOnlineSessionSearchResult& searchResult : _sessionSearch->SearchResults)
			{
				UE_LOG(LogTemp, Warning, TEXT("found : %s"), *searchResult.GetSessionIdStr());
			}
		}
	}
}

void UMyGameInstance::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	//SteamUser()->GetSteamID(), SteamP2pAddr->GetPort()

	//SteamUser()->GetSteamID(), SessionInfo->SteamP2PAddr->GetPort();
	//SessionInfo->HostAddr, SessionInfo->HostAddr->GetPort();

	_IsDedicatedServer = false;
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

//void UMyGameInstance::FindLowestPingAndNotify_Implementation()
//{
//	auto gameState = Cast<AGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
//	
//	auto readyRoomGameState = Cast<AReadyRoomGameStateBase>(gameState);
//	if (readyRoomGameState)
//	{
//		readyRoomGameState->FindAllPlayerControllerHideAllWidget();
//	}
//	
//	auto myGameState = Cast<AMyGameStateBase>(gameState);
//	bool gameStarted = false;
//	if (myGameState)
//	{
//		gameStarted = myGameState->_bGameStarted;
//	}
//
//
//	if (gameState->PlayerArray.IsValidIndex(1) && GetWorld()->IsServer())
//	{
//		auto players = gameState->PlayerArray;
//		players.Sort([](const APlayerState& a, const APlayerState& b)
//			{
//				return a.GetPing() < b.GetPing();
//			});
//
//		APlayerState* lowPingPlayer;
//		auto host = Cast<AMyPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AMyPlayerState>());
//		if (players[0] == host)
//		{
//			lowPingPlayer = players[1];
//		}
//		else
//		{
//			lowPingPlayer = players[0];
//		}
//		auto lowPing = Cast<AMyPlayerState>(lowPingPlayer);
//		auto mapname = GetWorld()->GetMapName();
//		mapname.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
//
//		auto name = lowPing->GetPlayerName();
//		if (name.IsEmpty())
//		{
//			name = "NewHost";
//		}
//
//		lowPing->SetMigrationInfo({true, name,mapname,gameStarted});
//		host->SaveBeforeExit();
//		
//		GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "Found low ping "+ mapname);
//	}
//
//	//Cast<AMyPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPlayerState<AMyPlayerState>())->SaveBeforeExit();
//
//}



//void UMyGameInstance::BindAltF4(bool on)
//{
//	UGameViewportClient* viewPortClient = GetGameViewportClient();
//
//	if (on)
//	{
//		if (viewPortClient)
//		{
//			if (viewPortClient->OnWindowCloseRequested().IsBound())
//			{
//				viewPortClient->OnWindowCloseRequested().Unbind();
//			}
//			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool
//				{	
//					CheckOpenPublicConnection(false);
//					FindLowestPingAndNotify();
//					_exitRequest = true;
//					FTimerHandle delay;
//					GetTimerManager().SetTimer(delay, FTimerDelegate::CreateLambda([&]() {DestroySession(); }), 1.5, false);			
//					return false;
//				});
//		}
//	}
//	else
//	{
//		if (viewPortClient)
//		{
//			if (viewPortClient->OnWindowCloseRequested().IsBound())
//			{
//				viewPortClient->OnWindowCloseRequested().Unbind();
//			}
//			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool {return true; });
//		}
//	}
//}


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


void UMyGameInstance::CreateDedicatedServers(FString url)
{
	static int port = GetWorld()->URL.Port;
	static int queryPort = 27015;
	static int childPort = port;

	FString dir = FPaths::ProjectDir();
	
	//FString ExecuterPathDebug = FPaths::ConvertRelativePathToFull(dir) + "Saved/StagedBuilds/WindowsServer/MyProject4Server";
	FString ExecuterPathDebug = FPaths::EngineDir() + "/Binaries/Win64/UE4Editor";


	FString ExecuterPathRelease = dir + "../MyProject4Server";
	
	if (!url.IsEmpty())
	{
		ExecuterPathDebug = url;
		ExecuterPathRelease = url;
	}

	uint32 OutProcessID = 0;
	queryPort++;
	childPort++;

	FString Attribute = "-log -Port=" + FString::FromInt(childPort) + " -QueryPort=" + FString::FromInt(queryPort);
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*ExecuterPathRelease, *Attribute, true, false, false, &OutProcessID, 0, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		_childProcess.Add(ProcHandle);
	}
	Attribute = "C:\\Users\\hyo29\\Desktop\\FirstStep\\GradProj\\MyProject4.uproject -log -server -SteamServerName = TwistRun -Port=" +FString::FromInt(childPort) +  "-QueryPort = "  + FString::FromInt(queryPort);
	ProcHandle = FPlatformProcess::CreateProc(*ExecuterPathDebug, *Attribute, true, false, false, &OutProcessID, 0, nullptr, nullptr);
	if (ProcHandle.IsValid())
	{
		_childProcess.Add(ProcHandle);
	}
	
	
}

void UMyGameInstance::InitializeBranch(const FString& WebServerIP)
{
	auto request = _http->CreateRequest();

	request->OnProcessRequestComplete().BindUObject(this, &UMyGameInstance::OnBranchProcessComplete);
	
	if (WebServerIP.IsEmpty())
	{
		request->SetURL("http://115.21.133.38:8335/api/Host");
		_webServerIP = "http://115.21.133.38:8335/";
	}
	else
	{
		request->SetURL(WebServerIP+"api/Host");
		_webServerIP = WebServerIP;
	}
	
	request->SetVerb("GET");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->ProcessRequest();
}

void UMyGameInstance::WriteMasterServerInfo(const FString& masterServerSteamIP)
{
	
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	
	jsonObject->SetNumberField("ServerID", 0);
	jsonObject->SetStringField("IPAddress", masterServerSteamIP);
	jsonObject->SetStringField("ServerName", "Test Server Name");
	jsonObject->SetStringField("MapName", "Test Map name");
	jsonObject->SetNumberField("CurrentPlayers", 0);
	jsonObject->SetNumberField("MaxPlayers", 1000);


	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);
	
	auto request = _http->CreateRequest();

	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);

	request->OnProcessRequestComplete().BindUObject(this, &UMyGameInstance::OnWriteProcessRequestComplete);
	request->SetURL(_webServerIP+"api/Host");
	request->SetVerb("POST");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->SetContentAsString(jsonString);
	request->ProcessRequest();
}

void UMyGameInstance::WriteDedicatedServerInfo(const FString& masterServerSteamIP)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);

	jsonObject->SetNumberField("ServerID", 0);
	jsonObject->SetStringField("IPAddress", masterServerSteamIP+":"+FString::FromInt(GetWorld()->URL.Port));
	UE_LOG(LogTemp, Warning, TEXT("ip:%s"),*(masterServerSteamIP + ":" + FString::FromInt(GetWorld()->URL.Port)));
	jsonObject->SetStringField("ServerName", "Test Server Name");
	jsonObject->SetStringField("MapName", GetWorld()->GetName());
	jsonObject->SetNumberField("CurrentPlayers", 0);
	jsonObject->SetNumberField("MaxPlayers", 8);
	jsonObject->SetBoolField("IsStarted", false);


	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);

	auto request = _http->CreateRequest();

	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);

	request->OnProcessRequestComplete().BindUObject(this, &UMyGameInstance::OnWriteProcessRequestComplete);
	request->SetURL(_webServerIP+"api/Host");
	request->SetVerb("PUT");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->SetContentAsString(jsonString);
	
	request->ProcessRequest();
}

void UMyGameInstance::ReadMasterServerInfo(const FString& webServerIP)
{
	if (_lanCheck)
	{
		return;
	}

	auto request = _http->CreateRequest();

	request->OnProcessRequestComplete().BindUObject(this, &UMyGameInstance::OnReadProcessRequestComplete);
	
	request->SetURL(webServerIP + "api/Host");
	request->SetVerb("GET");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->ProcessRequest();
}


void UMyGameInstance::ReadAllDedicatedServerInfo()
{
	auto request = _http->CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &UMyGameInstance::OnDediReadProcessRequestComplete);
	request->SetURL(_webServerIP+"api/Host/5");
	request->SetVerb("GET");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->ProcessRequest();
}

void UMyGameInstance::LinkMasterServerInfo(bool enter)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	jsonObject->SetStringField("IPAddress", " ");
	jsonObject->SetNumberField("Enter", enter?1:-1);
	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);


	auto request = _http->CreateRequest();
	request->SetURL(_webServerIP+"api/Values");
	request->SetVerb("POST");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(jsonString);

	request->ProcessRequest();
}

void UMyGameInstance::LinkDedicatedServerInfo(const FString& steamSessionID,bool enter)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	jsonObject->SetStringField("IPAddress",steamSessionID+":"+ FString::FromInt(GetWorld()->URL.Port));
	jsonObject->SetNumberField("Enter", enter ? 1 : -1);
	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);

	auto request = _http->CreateRequest();
	request->SetURL(_webServerIP+"api/Values");
	request->SetVerb("PUT");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(jsonString);

	request->ProcessRequest();
}


void UMyGameInstance::LogOutMasterServer()
{
	auto request = _http->CreateRequest();
	request->SetURL(_webServerIP+"api/Host");
	request->SetVerb("DELETE");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	request->ProcessRequest();
}

void UMyGameInstance::LogOutDediServer(const FString& currentSessionIP)
{	
	auto request = _http->CreateRequest();

	int port = GetWorld()->URL.Port;
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	FString IP = currentSessionIP + ":" + FString::FromInt(port);
	jsonObject->SetStringField("IPAddress", IP);
	jsonObject->SetBoolField("Start", false);
	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);

	UE_LOG(LogTemp, Warning, TEXT("Sent:%s"),*jsonString);

	//{"Address" : "~~"} -> {"Address" = "~~"}

	request->SetURL(_webServerIP+"api/Values");
	request->SetVerb("DELETE");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(jsonString);

	request->ProcessRequest();
}






void UMyGameInstance::OnWriteProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	if (success)
	{
		_IsDedicatedServer = true;
		UE_LOG(LogTemp, Warning, TEXT("Http Request Result:%s"),*response->GetContentAsString());
	}
	else
	{

	}
}


void UMyGameInstance::OnReadProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	static int limit = 4;
	if (success)
	{
		FString masterServerInfo = response->GetContentAsString();
		masterServerInfo.InsertAt(0, FString("{\"Response\":"));
		masterServerInfo.AppendChar('}');
		//UE_LOG(LogTemp, Warning, TEXT("Response %s"), *masterServerInfo);


		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<>::Create(masterServerInfo);

		if (FJsonSerializer::Deserialize(jsonReader,jsonObject) && jsonObject.IsValid())
		{
			TArray<TSharedPtr<FJsonValue>> jsonValue = jsonObject->GetArrayField(TEXT("Response"));
			
			FMasterServerData serverData = FMasterServerData();
			jsonObject = jsonValue.Pop()->AsObject();

			serverData._serverID = jsonObject->GetIntegerField("ServerID");
			serverData._serverName = jsonObject->GetStringField("ServerName");
			serverData._mapName = jsonObject->GetStringField("MapName");
			serverData._IPAddress = jsonObject->GetStringField("IPAddress");
			serverData._currentPlayers = jsonObject->GetIntegerField("CurrentPlayers");
			serverData._maxPlayers = jsonObject->GetIntegerField("MaxPlayers");
			

			//UE_LOG(LogTemp, Warning, TEXT("ServerID:%d"), serverData._serverID);
			//UE_LOG(LogTemp, Warning, TEXT("SteamIP:%s"), *serverData._IPAddress);
			//UE_LOG(LogTemp, Warning, TEXT("ServerName:%s"), *serverData._serverName);
			//UE_LOG(LogTemp, Warning, TEXT("MapName:%s"), *serverData._mapName);
			//UE_LOG(LogTemp, Warning, TEXT("CurrentPlayers:%d"), serverData._currentPlayers);
			//UE_LOG(LogTemp, Warning, TEXT("MaxPlayers:%d"), serverData._maxPlayers);
		
			_masterServerAddress = serverData._IPAddress;
			FString url = "steam." + _masterServerAddress;
			UGameplayStatics::OpenLevel(GetWorld(),FName(url));
		}
	}
	else
	{

		UE_LOG(LogTemp, Warning, TEXT("Http Read Request Failed"));
		ReadMasterServerInfo("https://localhost:44333/");
		//ReadMasterServerInfo("http://172.30.1.4:8335/"); //  IP
		//ReadMasterServerInfo("http://192.168.35.82:8335/"); // My Lan IP
		//ReadMasterServerInfo("http://172.30.1.43:8335/"); 
	}



}

void UMyGameInstance::OnBranchProcessComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	if (success)
	{
		FString masterServerInfo = response->GetContentAsString();
		masterServerInfo.InsertAt(0, FString("{\"Response\":"));
		masterServerInfo.AppendChar('}');
		UE_LOG(LogTemp, Warning, TEXT("Response %s"), *masterServerInfo);

		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<>::Create(masterServerInfo);

		if (masterServerInfo.Len()>20)
		{
			UE_LOG(LogTemp, Warning, TEXT("ServerID exists, this server will be dedicated server"));
			_bIsMasterServer = false;
			if (FJsonSerializer::Deserialize(jsonReader, jsonObject) && jsonObject.IsValid())
			{
				TArray<TSharedPtr<FJsonValue>> jsonValue = jsonObject->GetArrayField(TEXT("Response"));
				FMasterServerData serverData = FMasterServerData();
				jsonObject = jsonValue.Pop()->AsObject();
				serverData._IPAddress = jsonObject->GetStringField("IPAddress");
				UE_LOG(LogTemp, Warning, TEXT("SteamIP:%s"), *serverData._IPAddress);
				_masterServerAddress = serverData._IPAddress;
			}

			//GetWorld()->ServerTravel("/Game/map/ReadyMap",true);
		}
		else
		{
			FCoreDelegates::OnEnginePreExit.AddLambda([&]()
				{
					LogOutMasterServer();
					ExitAllServer();
				});
			_bIsMasterServer = true;
			UE_LOG(LogTemp, Warning, TEXT("this server will be master server"));
		}
	}
	else
	{
			UE_LOG(LogTemp, Warning, TEXT("Http Read Request Failed"));
			InitializeBranch("https://localhost:44333/");
			//InitializeBranch("http://172.30.1.4:8335/"); // My Lan IP
			//InitializeBranch("http://192.168.35.82:8335/"); // My Lan IP
			//InitializeBranch("http://172.30.1.43:8335/"); // My Lan IP
	}
}


void UMyGameInstance::OnDediReadProcessRequestComplete(FHttpRequestPtr request, FHttpResponsePtr response, bool success)
{
	
	if (success)
	{
		FString masterServerInfo = response->GetContentAsString();
		masterServerInfo.InsertAt(0, FString("{\"Response\":"));
		masterServerInfo.AppendChar('}');

		TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
		TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<>::Create(masterServerInfo);

		if (FJsonSerializer::Deserialize(jsonReader, jsonObject) && jsonObject.IsValid())
		{
			_waikikiDediServers.Empty();
			_spaceStationDediServers.Empty();
			TArray<TSharedPtr<FJsonValue>> jsonValue = jsonObject->GetArrayField(TEXT("Response"));
			
			for (auto value : jsonValue)
			{
				FDediServerData serverData = FDediServerData();
				auto jsonObj = value->AsObject();
				if (FJsonObjectConverter::JsonObjectToUStruct(jsonObj.ToSharedRef(), &serverData, 0, 0))
				{
					serverData._serverID = jsonObj->GetIntegerField("ServerID");
					serverData._serverName = jsonObj->GetStringField("ServerName");
					serverData._mapName = jsonObj->GetStringField("MapName");
					serverData._IPAddress = jsonObj->GetStringField("IPAddress");
					serverData._currentPlayers = jsonObj->GetIntegerField("CurrentPlayers");
					serverData._maxPlayers = jsonObj->GetIntegerField("MaxPlayers");
					serverData._bIsStarted = jsonObj->GetBoolField("IsStarted");

					if (serverData._mapName.Equals("ReadyMap"))
					{
						_waikikiDediServers.Emplace(serverData);
						
					}
					else if (serverData._mapName.Equals("SpaceStationReadyMap"))
					{
						_spaceStationDediServers.Emplace(serverData);
					}

					_IsDedicatedServer = true;
				}
			}
		}
		for (auto server : _waikikiDediServers)
		{
			UE_LOG(LogTemp, Warning, TEXT("enterable waikiki servers: %s"), *server._IPAddress);
		}
		for (auto server : _spaceStationDediServers)
		{
			UE_LOG(LogTemp, Warning, TEXT("enterable SpaceStation servers: %s"), *server._IPAddress);
		}

	}
	else
	{

	}
}

void UMyGameInstance::UpdateDedicatedServerState(const FString& steamSessionID, bool started)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	jsonObject->SetStringField("IPAddress", steamSessionID + ":"+FString::FromInt(GetWorld()->URL.Port));
	jsonObject->SetNumberField("Start", started);
	FString jsonString;
	TSharedRef<TJsonWriter<TCHAR>> jsonWriter = TJsonWriterFactory<>::Create(&jsonString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), jsonWriter);

	auto request = _http->CreateRequest();
	request->SetURL(_webServerIP+"api/GamePlay");
	request->SetVerb("POST");
	request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	request->SetContentAsString(jsonString);

	request->ProcessRequest();
}

const FString UMyGameInstance::GetBiggestServer(TArray<FDediServerData> container)
{
	TArray<FDediServerData> servers;
	for (auto server : container)
	{
		if (!server._bIsStarted && server._currentPlayers<8)
		{
			servers.Emplace(server);
			UE_LOG(LogTemp, Warning, TEXT("servers:%s is in server queue"),*server._IPAddress);
		}
	}

	FDediServerData biggest = FDediServerData();
	biggest._currentPlayers = -1;
	biggest._IPAddress = "Default";
	for (auto server : servers)
	{
		if (server._currentPlayers > biggest._currentPlayers)
		{
			biggest._IPAddress = server._IPAddress;
		}
	}

	return biggest._IPAddress;
}

const FString UMyGameInstance::GetEnterableWaikikiDediServer() 
{
	return GetBiggestServer(_waikikiDediServers);
}

const FString UMyGameInstance::GetEnterableSpacestationDediServer() 
{
	return GetBiggestServer(_spaceStationDediServers);
}


const FString UMyGameInstance::GetMasterServerAddress() const
{
	return _masterServerAddress;
}

void UMyGameInstance::ExitAllServer()
{	
	for (auto child : _childProcess)
	{
		FPlatformProcess::TerminateProc(child,true);
		UE_LOG(LogTemp, Warning, TEXT("Kill all dedciated Server!"));
	}

}

EMapSelection UMyGameInstance::BranchMap()
{
	
	if ((GetWorld()->URL.Port) % 2 == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Branch to Waikiki"));
		return EMapSelection::EMS_Waikiki;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Branch to SpaceStation"));
		return EMapSelection::EMS_SpaceStation;
	}

}

bool UMyGameInstance::DediCheck()
{
	return GetWorld()->GetNetMode() != ENetMode::NM_ListenServer;
}

FString UMyGameInstance::GetWorldName()
{
	FString mapname = GetWorld()->GetMapName();
	mapname.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	return mapname;
}

void UMyGameInstance::RequestExit(bool force)
{
	FGenericPlatformMisc::RequestExit(force);
}
