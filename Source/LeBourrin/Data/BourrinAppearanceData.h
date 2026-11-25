#pragma once
#include "CoreMinimal.h"

#include "BourrinAppearanceData.generated.h"
USTRUCT(BlueprintType)
struct FBourrinAppearanceData
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
    FColor Color;
};