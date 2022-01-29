// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"


const static FName SESSION_NAME = TEXT("TwistRunGame");
const static uint8 MAX_PLAYER = 8;


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
		FOnlineSessionSettings sessionSettings;
		sessionSettings.bIsLANMatch = _lanCheck;
		sessionSettings.NumPublicConnections = MAX_PLAYER;
		sessionSettings.bShouldAdvertise = true;
		sessionSettings.bUseLobbiesIfAvailable = true;
		sessionSettings.bUsesPresence = true;
		

		_sessionInterface->CreateSession(0, SESSION_NAME, sessionSettings);
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

	if (_selectedIndex.IsSet() &&_sessionSearch->SearchResults.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Join Session with Search Result"));
		_sessionInterface->JoinSession(0, SESSION_NAME, _sessionSearch->SearchResults[_selectedIndex.GetValue()]);
	}

}

void UMyGameInstance::Find()
{
	_sessionSearch = MakeShareable(new FOnlineSessionSearch());
	if(_sessionSearch.IsValid())
	{
		_sessionSearch->MaxSearchResults = 100;
		_sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
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
		_sessionInfo.Add(FSessionInfo{ session.PingInMs, MAX_PLAYER - session.Session.NumOpenPublicConnections,session.Session.OwningUserName });
	}

	return _sessionInfo;
}



void UMyGameInstance::SelectSession(const int& index)
{
	_selectedIndex=index;
}


void UMyGameInstance::OnCreateSessionComplete(FName sessionName, bool success)
{
	UWorld* world = GetWorld();
	if (!ensure(world != nullptr))
		return;

	world->ServerTravel("/Game/map/ReadyMap?listen");
}

void UMyGameInstance::OnDestroySessionComplete(FName sessionName, bool success)
{
	if (success)
	{
	}
}

void UMyGameInstance::OnNetworkFailure(UWorld* world, UNetDriver* netDriver, ENetworkFailure::Type failureType, const FString& error)
{
	APlayerController* playerController = GetFirstLocalPlayerController();
	if (!ensure(playerController != nullptr))
		return;

	playerController->ClientTravel("/Game/map/TitleMap",ETravelType::TRAVEL_Absolute);
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

	UE_LOG(LogTemp, Warning, TEXT("Joined Session"));

	playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);

}
