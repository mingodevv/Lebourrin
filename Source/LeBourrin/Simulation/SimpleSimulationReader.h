// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatSimulationManager.h"
#include "UObject/Object.h"
#include "SimpleSimulationReader.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimulationReader, Log, All);

/**
 * 
 */
UCLASS()
class LEBOURRIN_API ASimpleSimulationReader : public AActor
{
	GENERATED_BODY()
	
public:
	ASimpleSimulationReader();
	~ASimpleSimulationReader();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Simulation Reading")
	TObjectPtr<ACombatSimulationManager> CombatSimulationManager;
	
	UFUNCTION(BlueprintCallable, Category="Simulation Reading")
	void ReadSimulationData() const;
	
	UFUNCTION()
	void HandleSimulationRan(TArray<FAttackData> SimulationData);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
	FString ParseAttackData(const FAttackData PreviousAttack, const FAttackData AttackData) const;
	FString ProcessWeaponDrawing(const FAttackData AttackData) const;
	FString ProcessResult(const FAttackData AttackData) const;
	FString ProcessParrierWeaponLost(const FAttackData AttackData) const;
	FString ProcessAttackerWeaponLost(const FAttackData AttackData) const;
};
