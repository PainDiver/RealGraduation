// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWorldTimer.h"
#include "Blueprint/UserWidget.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "../Common/MyCharacter.h"
#include "../Common/MyGameInstance.h"
#include "MyPlayerState.h"
#include "MyProject4GameModeBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyGameStateBase.h"
#include "MySpectatorPawn.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "UMG.h"
#include "Styling/SlateBrush.h"
#include "MyPickUps.h"
#include "Animation/SkeletalMeshActor.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../ReadyMap/ReadyRoomPlayerController.h"


// Sets default values
AMyWorldTimer::AMyWorldTimer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
	_currentIndex = 0;
	
}

void AMyWorldTimer::SetupInputComponent()
{
	Super::SetupInputComponent();
}

// Called when the game starts or when spawned
void AMyWorldTimer::BeginPlay()
{
	Super::BeginPlay();
	InitializeInstance();
	InitializeHUD();
}

// Called every frame
void AMyWorldTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyWorldTimer::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	if (IsLocalPlayerController())
	{
		TArray<UUserWidget*> widgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), widgets, UUserWidget::StaticClass());

		for (auto widget : widgets)
		{
			widget->RemoveFromParent();
		}
	}
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}




void AMyWorldTimer::NotifyArrival_Implementation()
{
	EnsureFirstCall();

	if (!_gameMode)
	{
		_gameMode = Cast<AMyProject4GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	if (!_gameState)
	{
		_gameState = Cast<AMyGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	}
	if (!_playerState)
	{
		_playerState = Cast<AMyPlayerState>(PlayerState);
	}

	_gameMode->SetIsFinal(true);
	NotifyFinalCountToAllClients();
	SaveWinnerInfo();
	TurnOnSpectateUI();

	if (_gameMode->GetNumOfFinished() <= 2)
	{
		_playerState->_IsFirst = true;
		if (_playerState->_IsFirst)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hello Winner"));
			_playerState->_allMightyMode = true;
			AllMightyMode();
		}
	}
	else
	{
		ChangeBindAction_Spectate();
		SpectateMode(true);
	}

	_gameMode->IncreaseNumOfFinished();

}







void AMyWorldTimer::Spectate_Implementation()
{
	_playerState = GetPlayerState<AMyPlayerState>();

	if (_playerState->_IsSpectating)
	{
		_playerArray = UGameplayStatics::GetGameState(GetWorld())->PlayerArray;

		bool found = false;
		for (int i = 0; i < _playerArray.Num(); i++)
		{
			if (!(Cast<AMyPlayerState>(_playerArray[(++_currentIndex) % (_playerArray.Num())])->_IsSpectating) && !(Cast<AMyPlayerState>(_playerArray[(_currentIndex) % (_playerArray.Num())])->_IsFirst))
			{
				_currentSpectated = _playerArray[(_currentIndex) % (_playerArray.Num())]->GetOwner<AMyWorldTimer>();
				found = true;
				break;
			}
		}
		if (found)
		{
			SetViewTargetWithBlend(Cast<AMyCharacter>(_currentSpectated->GetCharacter()), 0.5);
		}
	}
}


void AMyWorldTimer::ChangeView_Implementation()
{
	_playerState = GetPlayerState<AMyPlayerState>();
	if (_playerState->_IsSpectating)
	{
		SetViewTarget(nullptr);
	}
}

void AMyWorldTimer::UpdateHUD_Implementation()
{

	TArray<AMyPickups*> inventory = GetPlayerState<AMyPlayerState>()->_Inventory;


	if (inventory.Num() >= 3)
		return;


	int i;
	for (i = 0; i < inventory.Num(); i++)
	{
		BrushSockets(i, static_cast<uint8>(inventory[i]->_ePickUpType));
	}

	ClearSockets(i);

}


void AMyWorldTimer::InitializeInstance()
{
	_gameMode = Cast<AMyProject4GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	_gameState = Cast<AMyGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	_playerState = Cast<AMyPlayerState>(PlayerState);
	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
	_character = Cast<AMyCharacter>(GetCharacter());
}


void AMyWorldTimer::InitializeHUD()
{
	if (IsLocalPlayerController())
	{
		_gameInstance = Cast<UMyGameInstance>(GetGameInstance());


		if (_gameInstance->_inventoryHUDAsset)
		{
			_inventoryHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_inventoryHUDAsset);
			if (_inventoryHUDOverlay)
			{
				_inventoryHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_inventoryHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_spectateHUDAsset)
		{
			_SpectateHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_spectateHUDAsset);
			if (_SpectateHUDOverlay)
			{
				_SpectateHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
				_SpectateHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_finalTimerHUDAsset)
		{
			_FinalTimerHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_finalTimerHUDAsset);
			if (_FinalTimerHUDOverlay)
			{
				_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
				_FinalTimerHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_startTimerHUDAsset)
		{
			_StartTimerHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_startTimerHUDAsset);
			if (_StartTimerHUDOverlay)
			{
				_StartTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_StartTimerHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_UIHUDAsset)
		{
			_UIHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_UIHUDAsset);
			if (_UIHUDOverlay)
			{
				_UIHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
				_UIHUDOverlay->AddToViewport();
			}
		}

		if (_gameInstance->_crossHairHUDAsset)
		{
			auto crossHair = CreateWidget<UUserWidget>(this, _gameInstance->_crossHairHUDAsset);
			if (crossHair)
			{
				crossHair->SetVisibility(ESlateVisibility::Visible);
				crossHair->AddToViewport();
			}
		}
		
		if (_gameInstance->_connectionUIHUDAsset)
		{
			auto connection = CreateWidget<UUserWidget>(this, _gameInstance->_connectionUIHUDAsset);
			if (connection)
			{
				connection->SetVisibility(ESlateVisibility::Visible);
				connection->AddToViewport();
			}
		}

		
		if (_gameInstance->_alarmUIHUDAsset)
		{
			 _alarmUIHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_alarmUIHUDAsset);
			if (_alarmUIHUDOverlay)
			{
				_alarmUIHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_alarmUIHUDOverlay->AddToViewport();
			}
		}

		if (_gameInstance->_gaugeUIHUDAsset)
		{
			auto gauge = CreateWidget<UUserWidget>(this, _gameInstance->_gaugeUIHUDAsset);
			if (gauge)
			{
				gauge->SetVisibility(ESlateVisibility::Visible);
				gauge->AddToViewport();
			}
		}

		if (_gameInstance->_crosshairUIHUDAsset)
		{
			auto crosshair = CreateWidget<UUserWidget>(this, _gameInstance->_crosshairUIHUDAsset);
			if (crosshair)
			{
				crosshair->SetVisibility(ESlateVisibility::Visible);
				crosshair->AddToViewport();
			}
		}
		if (_gameInstance->_characterStateUIHUDAsset)
		{
			auto state = CreateWidget<UUserWidget>(this, _gameInstance->_characterStateUIHUDAsset);
			if (state)
			{
				state->SetVisibility(ESlateVisibility::Visible);
				state->AddToViewport();
			}
		}

		if (_gameInstance->_minimapUIHUDAseet)
		{
			auto minimap = CreateWidget<UUserWidget>(this, _gameInstance->_minimapUIHUDAseet);
			if (minimap)
			{
				minimap->SetVisibility(ESlateVisibility::Visible);
				minimap->AddToViewport();
			}
		}

		if (_gameInstance->_realtimeRankUIHUDAsset)
		{
			_realtimeRankUIHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_realtimeRankUIHUDAsset);
			if (_realtimeRankUIHUDOverlay)
			{
				_realtimeRankUIHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_realtimeRankUIHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_gameTimerUIHUDAsset)
		{
			auto gameTimer = CreateWidget<UUserWidget>(this, _gameInstance->_gameTimerUIHUDAsset);
			if (gameTimer)
			{
				gameTimer->SetVisibility(ESlateVisibility::Visible);
				gameTimer->AddToViewport();
			}
		}
		



		GetWorldTimerManager().SetTimer(UpdateHandle, this, &AMyWorldTimer::UpdateHUD, 0.2, true, 0.2);
		SetInputMode(FInputModeUIOnly());
	}
}


void AMyWorldTimer::EnsureFirstCall()
{
	AController* now = this;
	if (_previous == now)
	{
		return;
	}
	_previous = now;
}

void AMyWorldTimer::SaveWinnerInfo()
{
	if (_gameInstance)
	{
		FCharacterInfo characterInfo = FCharacterInfo{ _playerState->_characterInfo._characterColor,_playerState->_characterInfo._CharacterName,_gameMode->GetNumOfFinished()};
		_gameInstance->_winner.Push(characterInfo);
	}
	else
	{
		_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
		FCharacterInfo characterInfo = FCharacterInfo{ _playerState->_characterInfo._characterColor,_playerState->_characterInfo._CharacterName,_gameMode->GetNumOfFinished() };
		_gameInstance->_winner.Push(characterInfo);
	}
	
}


void AMyWorldTimer::AllMightyMode_Implementation()
{
	if (!_character)
	{
		_character = Cast<AMyCharacter>(GetCharacter());
		if (!_character)
			return;
	}
	_character->GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	_character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	_character->GetCharacterMovement()->GravityScale = 0.0f;
	_character->GetCharacterMovement()->MaxAcceleration = 200000.f;
	
	_character->GetCharacterMovement()->MaxFlySpeed = 3000.f;
	//_character->GetCharacterMovement()->DisableMovement();
	//_character->SetReplicateMovement(false);

	//GEngine->AddOnScreenDebugMessage(0, 15, FColor::Blue, "AllMighty!");
	ChangeBindAction_AllMighty();
}

void AMyWorldTimer::SpectateMode(bool value)
{
	_playerState->SetIsSpectator(value);
	_playerState->_IsSpectating = value;
	
	if (value)
	{
		ChangeState(NAME_Spectating);
		ClientGotoState(NAME_Spectating);
	}
	else
	{
		if (_character)
		{
			Possess(_character);
			_character->SetActorTickEnabled(false);
		}
		ChangeState(NAME_Playing);
		ClientGotoState(NAME_Playing);
	}
}

void AMyWorldTimer::NotifyFinalCountToAllClients()
{
	TArray<AActor*> outactors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyWorldTimer::StaticClass(), outactors);
	for (auto players : outactors)
	{
		Cast<AMyWorldTimer>(players)->TurnOnFinalTimerUI();
	}
}


void AMyWorldTimer::BrushSockets_Implementation(int index, uint8 ept)
{
	if (!_inventoryHUDOverlay)
	{
		_inventoryHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_inventoryHUDAsset);
		_inventoryHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_inventoryHUDOverlay->AddToViewport();
	}

	UWidget* soc1 = _inventoryHUDOverlay->GetWidgetFromName(FName("ImageSock1"));
	UWidget* soc2 = _inventoryHUDOverlay->GetWidgetFromName(FName("ImageSock2"));

	UImage* socket1 = Cast<UImage>(soc1);
	UImage* socket2 = Cast<UImage>(soc2);

	UImage* temp[2];
	temp[0] = socket1;
	temp[1] = socket2;

	
	if (!(temp[index] == nullptr))
	{
		temp[index]->SetBrush(_sockImages[ept]);
	}

}

void AMyWorldTimer::ClearSockets_Implementation(int index)
{
	if (index >= 2)
		return;

	if (!_inventoryHUDOverlay)
	{
		_inventoryHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_inventoryHUDAsset);
		_inventoryHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_inventoryHUDOverlay->AddToViewport();
	}

	UWidget* soc1 = _inventoryHUDOverlay->GetWidgetFromName(FName("ImageSock1"));
	UWidget* soc2 = _inventoryHUDOverlay->GetWidgetFromName(FName("ImageSock2"));

	UImage* socket1 = Cast<UImage>(soc1);
	UImage* socket2 = Cast<UImage>(soc2);

	UImage* temp[2];
	temp[0] = socket1;
	temp[1] = socket2;



	for (; index < 2; index++)
	{
		if (temp[index] != nullptr)
		{
			temp[index]->SetBrush(_defaultImage);
		}
	}

}


void AMyWorldTimer::TurnOnSpectateUI_Implementation()
{

	if (_SpectateHUDOverlay)
	{
		_SpectateHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		_SpectateHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_spectateHUDAsset);
		_SpectateHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_SpectateHUDOverlay->AddToViewport();
	}
}



void AMyWorldTimer::TurnOnFinalTimerUI_Implementation()
{

	if (_FinalTimerHUDOverlay && _FinalTimerHUDOverlay->GetVisibility()==ESlateVisibility::Hidden)
	{
		_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	/*else
	{
		_FinalTimerHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_finalTimerHUDAsset);
		_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_FinalTimerHUDOverlay->AddToViewport();
	}*/
}

void AMyWorldTimer::ChangeBindAction_AllMighty()
{
	if (!IsLocalController())
	{
		return;
	}
	auto character = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)); 
	if (!character)
	{
		return;
	}
	/*auto ps = character->GetPlayerState();
	if (!ps)
	{
		return;
	}
	auto mps = Cast<AMyPlayerState>(ps);
	if (!mps)
	{
		return;
	}
	if (!mps->_allMightyMode)
	{
		return;
	}*/

	
	character->InputComponent->RemoveActionBinding(TEXT("Jump"), EInputEvent::IE_Pressed);
	character->InputComponent->RemoveActionBinding(TEXT("Jump"), EInputEvent::IE_Released);
	
	character->AllMightyModeBinding();
}




void AMyWorldTimer::ChangeBindAction_Spectate()
{
	if (!IsLocalController())
	{
		return;
	}
	InputComponent->BindAction(TEXT("Spectate"), IE_Pressed, this, &AMyWorldTimer::Spectate);
	InputComponent->BindAction(TEXT("ViewChange"), IE_Pressed, this, &AMyWorldTimer::ChangeView);
}
