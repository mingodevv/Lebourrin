#pragma once
#include "CoreMinimal.h"
#include "BourrinAppearanceData.h"
#include "WeaponData.h"
#include "Engine/DataTable.h"

#include "BourrinData.generated.h"
USTRUCT(BlueprintType)
struct FBourrinData : public FTableRowBase
{
	GENERATED_BODY()	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	int32 Life = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	int32 Force = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	int32 Agility = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	int32 Speed = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChanceToDrawANewWeaponWhenEmptyHanded = 0.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	TArray<FString> Weapons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinData")
	FBourrinAppearanceData AppearanceData;
};
