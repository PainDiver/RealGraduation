// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Interface/MyChattingControllerInterface.h"
#include "MyScoreMapPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT4_API AMyScoreMapPlayerController : public APlayerController, public IMyChattingControllerInterface
{
	GENERATED_BODY()

public:
	AMyScoreMapPlayerController();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void SetupInputComponent() override;

	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;


	void Initialize();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interface")
	void commit(const FString& message);
	virtual void commit_Implementation(const FString& message);

	virtual void ShowChattingPannel();

	UFUNCTION(BlueprintCallable, Category = "Chat")
		virtual void ShowChatBox();

	UFUNCTION(Server, Reliable)
		virtual void CreateTextHistoryFromServer(const FString& _message);

	UFUNCTION(Client, Reliable)
		virtual void makeHistoryFromClient(const FString& _message);


	UFUNCTION(Server, Unreliable)
		void DrawFromServerInstance();

	UFUNCTION(Client,Unreliable)
	void DrawWinners(const int& number, const FString& characterInfo,const FLinearColor& meshColor);




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		UUserWidget* _ChattingHUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
		UUserWidget* _ScoreHUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "characterInfo")
		FString _characterName;


	class UMyGameInstance* _gameInstance;

	bool _bChattable;

	bool _toggle;

	class UEditableText* _chatBox;

	class UWidget* _chattingPannel;

	class UScrollBox* _scrollbox;

	class UScrollBox* _WinnerScrollBox;


	FString _messageSent;

	FTimerHandle _timerHandle;
	FTimerHandle _timerHandle2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UClass* _Dummy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> _Steps;
};
