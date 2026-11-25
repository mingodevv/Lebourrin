#pragma once
#include "CoreMinimal.h"

#include "AttackResult.generated.h"
UENUM(BlueprintType)
enum class EAttackResult : uint8
{
	Hit = 0 UMETA(DisplayName="Hit"),
	Dodged = 1 UMETA(DisplayName="Dodged"),
	Parried = 2 UMETA(DisplayName="Parried"),
};