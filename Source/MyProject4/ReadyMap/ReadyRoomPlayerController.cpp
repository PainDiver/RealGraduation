// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyRoomPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ReadyRoomGameStateBase.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "../Common/MyGameInstance.h"
#include "GameFramework/Character.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../Common/MyCharacter.h"


AReadyRoomPlayerController::AReadyRoomPlayerController()
{
	_bChattable = true;
}

void AReadyRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	Initialize();
	UE_LOG(LogTemp, Warning, TEXT("controller okay"));
}



void AReadyRoomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction(TEXT("Chat"), IE_Pressed, this, &AReadyRoomPlayerController::ShowChattingPannel);
}

void AReadyRoomPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	InputComponent->ClearActionBindings();
	if (IsLocalPlayerController())
	{
		TArray<UUserWidget*> widgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), widgets, UUserWidget::StaticClass());

		for (auto widget : widgets)
		{
			widget->RemoveFromParent();
		}
	}

	Super::PreClientTravel(PendingURL,TravelType,bIsSeamlessTravel);	
}





void AReadyRoomPlayerController::commit_Implementation(const FString& message)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Commit in ReadyRoom!"));

	if (!message.IsEmpty())
	{
		_messageSent = _gameInstance->_characterInfo._CharacterName + FString(": ") + message;
	}
	else
	{
		_messageSent = "";
	}


	CreateTextHistoryFromServer(_messageSent);

	_toggle = false;


	//Only for fading effect


	GetWorldTimerManager().SetTimer(_timerHandle, FTimerDelegate::CreateLambda([&]()
		{
			_chattingPannel->GetCachedWidget()->SetRenderOpacity(_chattingPannel->GetCachedWidget()->GetRenderOpacity() - 0.05);
			if (_chattingPannel->GetCachedWidget()->GetRenderOpacity() < 0)
			{
				GetWorldTimerManager().ClearTimer(_timerHandle);
			}
		}), 0.3, true, 0.3);
}


void AReadyRoomPlayerController::ShowChatBox()
{
	if (!_ChattingHUDOverlay)
	{
		return;
	}
	else if (_ChattingHUDOverlay->GetVisibility() == ESlateVisibility::Hidden)
	{
		_ChattingHUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}

	if (_chattingPannel)
		_chattingPannel->GetCachedWidget()->SetRenderOpacity(1);
}


void AReadyRoomPlayerController::CreateTextHistoryFromServer_Implementation(const FString& _message)
{
	if (_message.IsEmpty())
	{
		return;
	}
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetPawn()->GetWorld(), APlayerController::StaticClass(), OutActors);
	for (AActor* OutActor : OutActors)
	{
		AReadyRoomPlayerController* playerController = Cast<AReadyRoomPlayerController>(OutActor);
		if (playerController)
		{
			playerController->makeHistoryFromClient(_message);
		}
	}
}

void AReadyRoomPlayerController::makeHistoryFromClient_Implementation(const FString& _message)
{
	ShowChatBox();

	UTextBlock* NewTextBlock = NewObject<UTextBlock>();

	NewTextBlock->Font.Size = 15;
	NewTextBlock->SetText(FText::FromString(_message));

	_scrollbox->AddChild(NewTextBlock);
	_scrollbox->ScrollToEnd();

}

void AReadyRoomPlayerController::Initialize()
{
	AGameStateBase* gameState = UGameplayStatics::GetGameState(GetWorld());
	_gameState = Cast<AReadyRoomGameStateBase>(gameState);
	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());


	if (!IsLocalPlayerController())
	{
		return;
	}

	if (GetWorld()->IsServer())
	{
		if (_MapSelectionHUDAsset)
		{
			auto mapSelection = CreateWidget<UUserWidget>(this, _MapSelectionHUDAsset);
			if (mapSelection)
			{
				mapSelection->SetVisibility(ESlateVisibility::Visible);
				mapSelection->AddToViewport();
			}
		}
	}

	if (_gameInstance->_readyHUDAsset)
	{
		if (!_ReadyHUDOverlay)
		{
			_ReadyHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_readyHUDAsset);
			_ReadyHUDOverlay->SetVisibility(ESlateVisibility::Visible);
			_ReadyHUDOverlay->AddToViewport();
		}
	}

	if (_gameInstance->_chattingHUDAsset)
	{
		if (!_ChattingHUDOverlay)
		{
			_ChattingHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_chattingHUDAsset);
			_ChattingHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
			_chatBox = Cast<UEditableText>(_ChattingHUDOverlay->GetWidgetFromName(FName("chattingBox")));
			_scrollbox = Cast<UScrollBox>(_ChattingHUDOverlay->GetWidgetFromName(FName("Scroller")));
			_ChattingHUDOverlay->AddToViewport();
			_chattingPannel = _ChattingHUDOverlay->GetRootWidget();
		}
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		if (_gameInstance->_startHUDAsset)
		{
			if (!_StartHUDOverlay)
			{
				_StartHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_startHUDAsset);
				_StartHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_StartHUDOverlay->AddToViewport();
			}
		}

		GetWorld()->GetTimerManager().SetTimer(_timerHandle2, FTimerDelegate::CreateLambda([&]()
			{
				if (_StartHUDOverlay)
				{
					if (_gameState->GetIsStartable())
					{

						if (_StartHUDOverlay->GetVisibility() == ESlateVisibility::Hidden)
							_StartHUDOverlay->SetVisibility(ESlateVisibility::Visible);
					}
					else
					{
						if (_StartHUDOverlay->GetVisibility() == ESlateVisibility::Visible)
							_StartHUDOverlay->SetVisibility(ESlateVisibility::Hidden);
					}
				}
			}
		), 2.f, true, 0);
	}

	_bChattable = true;
}


void AReadyRoomPlayerController::ShowChattingPannel()
{
	this->bShowMouseCursor = false;
	if (!_ChattingHUDOverlay)
	{
		return;
	}

	if (_toggle == false && _bChattable)
	{
		ShowChatBox();
		GetWorld()->GetTimerManager().ClearTimer(_timerHandle);
		FSlateApplication::Get().SetKeyboardFocus(_chatBox->TakeWidget());
		_toggle = true;
	}
}