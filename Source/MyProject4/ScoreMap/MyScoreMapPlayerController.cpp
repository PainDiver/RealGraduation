// Fill out your copyright notice in the Description page of Project Settings.


#include "MyScoreMapPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "../Common/MyGameInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyScoreMapGameStateBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AMyScoreMapPlayerController::AMyScoreMapPlayerController()
{
	_bChattable = true;
}

void AMyScoreMapPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction(TEXT("Chat"), IE_Pressed, this, &AMyScoreMapPlayerController::ShowChattingPannel);

}


void AMyScoreMapPlayerController::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
	
	FTimerHandle delayTimer;



	GetWorldTimerManager().SetTimer(delayTimer, FTimerDelegate::CreateLambda([&]()
		{
			Cast<AMyScoreMapGameStateBase>(UGameplayStatics::GetGameState(GetWorld()))->FindAllPlayerControllerHideAllWidget();
			bBlockInput = true;
			Cast<UMyGameInstance>(GetGameInstance())->_winner.Empty();
		}
	), 10, false);

}

void AMyScoreMapPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
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


void AMyScoreMapPlayerController::Initialize()
{
	AGameStateBase* gameState = UGameplayStatics::GetGameState(GetWorld());
	_gameInstance = Cast<UMyGameInstance>(GetGameInstance());

	if (IsLocalPlayerController())
	{
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
		if (_gameInstance->_scoreHUDAsset)
		{
			if (!_ScoreHUDOverlay)
			{
				_ScoreHUDOverlay = CreateWidget<UUserWidget>(this, _gameInstance->_scoreHUDAsset);
				_ScoreHUDOverlay->SetVisibility(ESlateVisibility::Visible);
				_WinnerScrollBox = Cast<UScrollBox>(_ScoreHUDOverlay->GetWidgetFromName(FName("WinnerScrollBox")));
				_ScoreHUDOverlay->AddToViewport();
			}
		}
	}
	DrawFromServerInstance();
	_bChattable = true;
}


void AMyScoreMapPlayerController::DrawFromServerInstance_Implementation()
{
	for (auto info : Cast<UMyGameInstance>(GetGameInstance())->_winner)
	{
		DrawWinners(info._number,info._CharacterName,info._characterColor);
	}

}

void AMyScoreMapPlayerController::DrawWinners_Implementation(const int& number,const FString& characterInfo, const FLinearColor& meshColor)
{
	UTextBlock* NewTextBlock = NewObject<UTextBlock>();
	NewTextBlock->Font.Size = 25;
	NewTextBlock->SetText(FText::FromString(FString::FromInt(number + 1) + FString("\t\t\t") + characterInfo));
	if (_WinnerScrollBox)
	{
		_WinnerScrollBox->AddChild(NewTextBlock);
	}
	if (number < 3)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("dummy spawned!"));
		auto actor = GetWorld()->SpawnActor<AActor>(_Dummy, _Steps[number], FRotator(0, 0, 0), FActorSpawnParameters());
		FLinearColor matColor;
		auto mesh = actor->FindComponentByClass(USkeletalMeshComponent::StaticClass());
		if (IsValid(Cast<USkeletalMeshComponent>(mesh)))
		{
			Cast<USkeletalMeshComponent>(mesh)->CreateDynamicMaterialInstance(0)->SetVectorParameterValue(TEXT("Color"), meshColor);
		}
	}
}



void AMyScoreMapPlayerController::ShowChattingPannel()
{
	this->bShowMouseCursor = false;
	
	if (_toggle == false && _bChattable)
	{
		ShowChatBox();
		GetWorld()->GetTimerManager().ClearTimer(_timerHandle);
		FSlateApplication::Get().SetKeyboardFocus(_chatBox->TakeWidget());
		_toggle = true;
	}
}

void AMyScoreMapPlayerController::ShowChatBox()
{
	
	if (_ChattingHUDOverlay)
	{
		if (_ChattingHUDOverlay->GetVisibility() == ESlateVisibility::Hidden)
		{
			_ChattingHUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

	_chattingPannel->GetCachedWidget()->SetRenderOpacity(1);
}



void AMyScoreMapPlayerController::commit_Implementation(const FString& message)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Commit in ScoreMpa!"));

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


void AMyScoreMapPlayerController::CreateTextHistoryFromServer_Implementation(const FString& _message)
{
	if (_message.IsEmpty())
	{
		return;
	}
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), OutActors);
	for (AActor* OutActor : OutActors)
	{
		AMyScoreMapPlayerController* playerController = Cast<AMyScoreMapPlayerController>(OutActor);
		if (playerController)
		{
			playerController->makeHistoryFromClient(_message);
		}
	}
}

void AMyScoreMapPlayerController::makeHistoryFromClient_Implementation(const FString& _message)
{
	ShowChatBox();

	UTextBlock* NewTextBlock = NewObject<UTextBlock>();

	NewTextBlock->Font.Size = 15;
	NewTextBlock->SetText(FText::FromString(_message));

	_scrollbox->AddChild(NewTextBlock);
	_scrollbox->ScrollToEnd();

}


