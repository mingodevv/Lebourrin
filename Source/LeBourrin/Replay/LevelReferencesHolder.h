#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LevelReferencesHolder.generated.h"

class ABourrinActor;
class UDataTable;
struct FWeaponData;

UCLASS()
class LEBOURRIN_API ALevelReferencesHolder : public AActor
{
	GENERATED_BODY()

public:
	ALevelReferencesHolder();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="References")
	TArray<TObjectPtr<ABourrinActor>> Bourrins;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="References")
	TObjectPtr<UDataTable> WeaponsTable;

	const FWeaponData* FindWeaponData(const FString& WeaponName) const;

	UFUNCTION(BlueprintPure, Category="References")
	ABourrinActor* FindBourrin(const FString& BourrinName) const;

	UFUNCTION(BlueprintCallable, Category="References")
	void PrepareBourrins(const FString& Player1Name, const FString& Player2Name);

protected:
	virtual void BeginPlay() override;

private:
	void FindBourrinsInLevelIfNeeded();
};
