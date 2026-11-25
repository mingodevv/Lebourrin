#pragma once
#include "CoreMinimal.h"
#include "WeaponAppearanceData.h"
#include "Engine/DataTable.h"

#include "WeaponData.generated.h"
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChanceToKeepWeaponAfterAttacking = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData")
	FVector2D DamageToDeal = FVector2D(30, 40);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData")
	int32 SpeedIncrement = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChanceToParry = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChanceToKeepWeaponAfterParrying = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData")
	FWeaponAppearanceData AppearanceData;
};