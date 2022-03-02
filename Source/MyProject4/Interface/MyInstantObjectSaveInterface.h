// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MyInstantObjectSaveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UMyInstantObjectSaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MYPROJECT4_API IMyInstantObjectSaveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Loot")
	void SaveData();

	virtual void SaveData_Implementation()=0;
};
