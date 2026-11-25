#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "BourrinVisualData.generated.h"

class USkeletalMesh;
class UAnimSequence;

USTRUCT(BlueprintType)
struct FBourrinVisualData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinVisualData")
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinVisualData")
	TObjectPtr<UAnimSequence> WeaponAttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BourrinVisualData")
	float OpponentDistance = 130.f;
};
