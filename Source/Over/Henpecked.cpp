// Fill out your copyright notice in the Description page of Project Settings.


#include "Henpecked.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet\GameplayStatics.h"

// Sets default values
AHenpecked::AHenpecked()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	this->SetActorRelativeScale3D(FVector(0.6, 0.6, 0.6));
	HaveTask = false;
}

// Called when the game starts or when spawned
void AHenpecked::BeginPlay()
{
	Super::BeginPlay();

	BotController = Cast<AAllyController>(this->Controller);
	if (BotController)
	{
		UE_LOG(LogTemp, Warning, TEXT("you eat!!!"));
		BotController->MoveToActor(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0),-20.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Jonny you eat shit!!!"));
	}
}

// Called every frame
void AHenpecked::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

// Called to bind functionality to input
void AHenpecked::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

