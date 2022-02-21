// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../Common/MyGameInstance.h"
#include "MyPlayerState.generated.h"

/**
 *
 */

USTRUCT()
struct FMigrationPacket
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	bool _IsHost = false;

	UPROPERTY()
	bool _IsStarted = false;
};


UCLASS()
class MYPROJECT4_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

protected:
	virtual void BeginPlay();


public:

	
	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		bool _IsSpectating;

	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		bool _IsFirst;

	UPROPERTY(replicated, BlueprintReadOnly, Category = "Spectate")
		TArray<class AMyPickups*> _Inventory;


	UPROPERTY(replicatedUsing = OnRep_InitializeColorAndNotifyConnection)
	FCharacterInfo _characterInfo;


	UPROPERTY(replicated)
		bool _Initialized;

	UPROPERTY(replicated)
		bool _allMightyMode;


	UFUNCTION(Server, Reliable)
		void SetCharacterInfo_Server(FCharacterInfo info);

	UFUNCTION(Server,Reliable)
	void OnRep_InitializeColorAndNotifyConnection();

	UFUNCTION(Server, Reliable)
		void NotifyConnection();


	UFUNCTION(Client, Reliable)
	void BuildSessionForMigration();

	UFUNCTION(Client, Reliable)
	void ClientMigration();

	UFUNCTION(Client, UnReliable)
	void OnRep_ServerMigration();

	void SetMigrationInfo(FMigrationPacket info) { _migrationInfo=info; }

	inline FMigrationPacket GetMigrationInfo() { return _migrationInfo; }

	

private:

	UPROPERTY(replicatedUsing = OnRep_ServerMigration)
		FMigrationPacket _migrationInfo;


};
