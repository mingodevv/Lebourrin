#pragma once

#include "CoreMinimal.h"

#include "WeaponAppearanceData.generated.h"
class UStaticMesh;
USTRUCT(BlueprintType)
struct FWeaponAppearanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData")
	UStaticMesh* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WeaponData")
	FString SocketName;
};