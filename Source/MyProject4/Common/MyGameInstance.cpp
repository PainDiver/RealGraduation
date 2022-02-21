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
#include "../PlayMap/MyPlayerState.h"

uint8 UMyGameInstance::MAX_PLAYER = 8;
FName UMyGameInstance::SESSION_NAME = TEXT("TwistRunGame");


void UMyGameInstance::Init()
{
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
	for (int i = 0; i < _sessionSearch->SearchResults.Num(); i++)
	{
		const auto& session = _sessionSearch->SearchResults[i];

		_sessionInfo.Add(FSessionInfo{ session.PingInMs,MAX_PLAYER - session.Session.NumOpenPublicConnections,session.Session.OwningUserName });
	}

	return _sessionInfo;
}



void UMyGameInstance::SelectSession(const int& index)
{
	_selectedIndex=index;
}

void UMyGameInstance::DestroySession()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Destroy Session");
	_sessionInterface->DestroySession(_currentSessionName);

}


void UMyGameInstance::OnCreateSessionComplete(FName sessionName, bool success)
{
	UWorld* world = GetWorld();
	if (!ensure(world != nullptr))
		return;

	world->ServerTravel("/Game/map/ReadyMap?game=/Game/ReadyRoomMap/MyReadyRoomGameModeBase_BP.MyReadyRoomGameModeBase_BP_C?listen");

	CheckOpenPublicConnection(true);

}

void UMyGameInstance::OnDestroySessionComplete(FName sessionName, bool success)
{
	if (_exitRequest)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "OnDestroy");
		UGameViewportClient* viewPortClient = GetGameViewportClient();
		if (viewPortClient)
		{
			if (viewPortClient->OnWindowCloseRequested().IsBound())
			{
				viewPortClient->OnWindowCloseRequested().Unbind();
			}
			viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool {return true; });
			FGenericPlatformMisc::RequestExit(true);
		}
	}

	if (success)
	{
		APlayerController* playerController = GetFirstLocalPlayerController();
		if (!ensure(playerController != nullptr))
			return;

		playerController->ClientTravel("/Game/map/TitleMap?game=/Game/TitleMap/MyTitleGameModeBase_BP.MyTitleGameModeBase_BP_C", ETravelType::TRAVEL_Absolute);
	}
}

void UMyGameInstance::OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error)
{
	UGameViewportClient* viewPortClient = GetGameViewportClient();
	if (viewPortClient)
	{
		if (viewPortClient->OnWindowCloseRequested().IsBound())
		{
			viewPortClient->OnWindowCloseRequested().Unbind();
		}
		viewPortClient->OnWindowCloseRequested().BindLambda([&]()->bool {return true; });
	}
	
	APlayerController* playerController = GetFirstLocalPlayerController();
	if (!ensure(playerController != nullptr))
		return;
	
	auto existingSession = _sessionInterface->GetNamedSession(SESSION_NAME);
	if (existingSession != nullptr)
	{
		_sessionInterface->DestroySession(SESSION_NAME);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, "Session Destroyed for no network");
	}
	else
	{
		playerController->ClientTravel("/Game/map/TitleMap?game=/Game/TitleMap/MyTitleGameModeBase_BP.MyTitleGameModeBase_BP_C", ETravelType::TRAVEL_Absolute);
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
	if (!ensure(playerController != nullptr))
	{
		return;
	}

	_currentSessionName = sessionName;
	_currentSessionAddress = address;

	playerController->ClientTravel(_currentSessionAddress, ETravelType::TRAVEL_Absolute);
	
}

void UMyGameInstance::UpdateGameSession_Implementation(FName sessionName, bool advertise)
{
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem != nullptr)
	{
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
	}	
	
	GetCurrentSessionInterface()->CreateSession(0, _currentSessionName, *(_lastSettings));
}

void UMyGameInstance::OnDestroySessionClientMigration(FName sessionName, bool success)
{
	if (success)
	{
		_sessionSearch->MaxSearchResults = 1;
		_sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		_sessionSearch->bIsLanQuery = _lanCheck;

		GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionComplete);
		GetCurrentSessionInterface()->FindSessions(0, _sessionSearch.ToSharedRef());
	}
}



void UMyGameInstance::OnFindSessionMigration(bool success)
{
	GetCurrentSessionInterface()->OnFindSessionsCompleteDelegates.Clear();

	if (success && _sessionSearch.IsValid())
	{
		GetCurrentSessionInterface()->JoinSession(0, _currentSessionName, _sessionSearch->SearchResults[0]);
	}
	_sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstance::OnFindSessionComplete);
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
	if (GEngine->IsEditor())
	{
		return true;
	}

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