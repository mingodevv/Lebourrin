#pragma once
#include "CoreMinimal.h"
#include "AttackResult.h"
#include "BourrinRuntimeInfosData.h"


#include "AttackData.generated.h"
USTRUCT(BlueprintType)
struct FAttackData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	bool IsValid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	FString AttackerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	FString AttackerWeaponNameOnStart;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	FString AttackerWeaponNameDrawed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	FString AttackerWeaponNameOnEnd;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	int32 InflictedDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	FString TargetName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	bool TargetLostWeaponAfterParry;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	EAttackResult AttackResult;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AttackData")
	TMap<FString /*name*/, FBourrinRuntimeInfosData/*Bourrin's state*/> BourrinsStatesAtTheEnd;
};