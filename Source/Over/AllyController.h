// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AllyController.generated.h"

/**
 * 
 */
UCLASS()
class OVER_API AAllyController : public AAIController
{
	GENERATED_BODY()

public:
	AAllyController();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UAISenseConfig_Sight* SightBot;

protected:

	

	virtual void OnPossess(APawn* InPawn) override;
};
