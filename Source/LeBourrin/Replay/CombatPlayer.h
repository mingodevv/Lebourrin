#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/AttackData.h"

#include "CombatPlayer.generated.h"

class ACombatSimulationManager;
class ALevelReferencesHolder;
class UCombatAction;
class UDataTable;

UCLASS()
class LEBOURRIN_API ACombatPlayer : public AActor
{
	GENERATED_BODY()

public:
	ACombatPlayer();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="CombatPlayer")
	TObjectPtr<ACombatSimulationManager> CombatSimulationManager;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="CombatPlayer")
	TObjectPtr<ALevelReferencesHolder> ReferencesHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CombatPlayer")
	TObjectPtr<UDataTable> BourrinVisualsTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CombatPlayer")
	float StartDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CombatPlayer")
	bool bRandomFighters = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CombatPlayer")
	TArray<TObjectPtr<UCombatAction>> ActionsQueue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CombatPlayer")
	TObjectPtr<UCombatAction> CurrentAction;

	UFUNCTION()
	void HandleSimulationReady(TArray<FAttackData> SimulationData);

	UFUNCTION(BlueprintCallable, Category="CombatPlayer")
	void PlayNext();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleActionFinished(UCombatAction* Action);

	void PickRandomFighters();
	void BuildQueue(const TArray<FAttackData>& SimulationData);
	void ShowStartInfos(const TArray<FAttackData>& SimulationData);
	void ApplyVisuals();
	void ShowWinner();

	TArray<FAttackData> PlayedData;
	FTimerHandle StartTimerHandle;
};
