// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HenpeckedEnum.generated.h"

/**
 * 
 */
UENUM(BlueprintType) //Blueprint Temporaire
enum class EHenpeckedEnum : uint8
{
	HE_White       UMETA(DisplayName = "White"),
	HE_Red         UMETA(DisplayName = "Red"),
	HE_Green        UMETA(DisplayName = "Green"),
	HE_Blue            UMETA(DisplayName = "Blue"),
	HE_All             UMETA(DisplayName = "All"),
	HE_None            UMETA(DisplayName = "None")
};
