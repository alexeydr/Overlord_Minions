// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyController.h"
#include "Engine/World.h"
#include "Kismet\GameplayStatics.h"


AAllyController::AAllyController()
{
	PrimaryActorTick.bCanEverTick = true;
	SightBot = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>("Perception"));
}

void AAllyController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

}


void AAllyController::BeginPlay()
{
	Super::BeginPlay();
}

void AAllyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
