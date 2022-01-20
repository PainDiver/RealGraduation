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

	InputComponent->BindAction(TEXT("Spectate"), IE_Pressed, this, &AMyWorldTimer::Spectate);
	InputComponent->BindAction(TEXT("ViewChange"), IE_Pressed, this, &AMyWorldTimer::ChangeView);
}



// Called when the game starts or when spawned
void AMyWorldTimer::BeginPlay()
{
	Super::BeginPlay();

	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());
	_gameState = Cast< AMyGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));

	if (IsLocalPlayerController())
	{
		if (_gameInstance->_inventoryHUDAsset)
		{
			_inventoryHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_inventoryHUDAsset);
			if (_inventoryHUDOverlay)
			{
				_inventoryHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_inventoryHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_SpectateHUDAsset)
		{
			_SpectateHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_SpectateHUDAsset);
			if (_SpectateHUDOverlay)
			{
				_SpectateHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
				_SpectateHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_FinalTimerHUDAsset)
		{
			_FinalTimerHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_FinalTimerHUDAsset);
			if (_FinalTimerHUDOverlay)
			{
				_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
				_FinalTimerHUDOverlay->AddToViewport();
			}
		}
		if (_gameInstance->_StartTimerHUDAsset)
		{
			_StartTimerHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_StartTimerHUDAsset);
			if (_StartTimerHUDOverlay)
			{
				_StartTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_StartTimerHUDOverlay->AddToViewport();
			}
		}

		SetCharacterName(_gameInstance->_characterInfo._CharacterName);
	}
	GetWorldTimerManager().SetTimer(UpdateHandle,this,&AMyWorldTimer::UpdateHUD,0.2,true,0.2);
	SetInputMode(FInputModeUIOnly());
}








// Called every frame
void AMyWorldTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMyWorldTimer::NotifyArrival_Implementation()
{
	AController* now = this;
	if (_previous == now)
	{
		return;
	}
	_previous = now;

	_gameMode = Cast<AMyProject4GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	_gameMode->_bIsFinal = true;
	
	_gameState = Cast<AMyGameStateBase>(UGameplayStatics::GetGameState( GetWorld() ) );
	_playerState = Cast<AMyPlayerState>(PlayerState);


	UMyGameInstance* game = Cast<UMyGameInstance>(GetGameInstance());

	FCharacterInfo characterInfo = FCharacterInfo{ _playerState->_CharacterColor,_playerState->_CharacterName,_gameMode->_NumOfFinished };
	game->_winner.Push(characterInfo);
	
	TurnOnSpectateUI();
	TArray<AActor*> outactors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyWorldTimer::StaticClass(), outactors);
	for (auto players : outactors)
	{
		Cast<AMyWorldTimer>(players)->TurnOnFinalTimerUI();
	}

	if (++(_gameMode->_NumOfFinished) == 1)
	{
		_playerState->_IsFirst = true;
		GetCharacter()->GetCharacterMovement()->Deactivate();
		GetCharacter()->SetActorTickEnabled(false);
		SetActorHiddenInGame(true);

		//Allmighty
		if (_playerState->_IsFirst)
		{
			AllMightyMode();
			// get into allmighty mode
		}
	}
	else
	{
		_playerState->SetIsSpectator(true);
		_playerState->_IsSpectating = true;
		SetActorHiddenInGame(true);

		GetWorldTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([&]()
			{
				GetCharacter()->GetCharacterMovement()->Deactivate();
				GetCharacter()->SetActorTickEnabled(false);
				ChangeState(NAME_Spectating);
				ClientGotoState(NAME_Spectating);
				GetWorldTimerManager().ClearTimer(DelayHandle);

			}), 1, false);
	}
	
}




void AMyWorldTimer::Spectate_Implementation()
{
	_playerState = GetPlayerState<AMyPlayerState>();

	if (_playerState->_IsSpectating)
	{
		_playerArray = UGameplayStatics::GetGameState(GetWorld())->PlayerArray;

		bool found=false;
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


void AMyWorldTimer::AllMightyMode_Implementation()
{

}


void AMyWorldTimer::UpdateHUD_Implementation()
{
	
	TArray<AMyPickups*> inventory = GetPlayerState<AMyPlayerState>()->_Inventory;

	
	if (inventory.Num() >= 3 || _sockImages.Num() < 2)
		return;
	

	int i;
	for (i = 0; i < inventory.Num(); i++)
	{
		BrushSockets(i, static_cast<uint8>(inventory[i]->_ept));
	}

	ClearSockets(i);

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
		_SpectateHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_SpectateHUDAsset);
		_SpectateHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_SpectateHUDOverlay->AddToViewport();
	}
}



void AMyWorldTimer::TurnOnFinalTimerUI_Implementation()
{

	if (_FinalTimerHUDOverlay)
	{
		_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		_FinalTimerHUDOverlay = CreateWidget<UUserWidget>(this, Cast<UMyGameInstance>(GetGameInstance())->_FinalTimerHUDAsset);
		_FinalTimerHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		_FinalTimerHUDOverlay->AddToViewport();
	}
}

void AMyWorldTimer::SetCharacterName_Implementation(const FString& name)
{
	_playerState = GetPlayerState<AMyPlayerState>();
	_playerState->_CharacterName = name;
}
