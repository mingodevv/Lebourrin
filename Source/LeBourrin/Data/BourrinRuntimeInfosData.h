#pragma once

#include "CoreMinimal.h"

#include "BourrinRuntimeInfosData.generated.h"
USTRUCT(BlueprintType)
struct FBourrinRuntimeInfosData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinRuntimeData")
	int32 LifeLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinRuntimeData")
	int32 Exhaust;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinRuntimeData")
	FString EquippedWeaponName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinRuntimeData")
	TArray<FString> InInventoryWeapons;
};