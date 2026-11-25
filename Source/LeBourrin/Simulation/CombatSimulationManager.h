// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/AttackData.h"
#include "Data/AttackTargetData.h"
#include "GameFramework/Actor.h"


#include "CombatSimulationManager.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimulationRan, TArray<FAttackData>, SimulationData);
UCLASS()
class LEBOURRIN_API ACombatSimulationManager : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnSimulationRan OnSimulationRan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SimulationData")
	TObjectPtr<UDataTable> BourrinsTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SimulationData")
	TObjectPtr<UDataTable> WeaponsTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SimulationData")
	FString Player1Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SimulationData")
	FString Player2Name;
	UFUNCTION(BlueprintCallable, Category="SimulationData")
	bool SimulationRan() const;
	
	// Sets default values for this actor's properties
	ACombatSimulationManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="SimulationData")
	TArray<FAttackData> RunSimulation();
	UFUNCTION(BlueprintCallable, Category="SimulationData")
	TArray<FAttackData> GetSimulationData() const;
	
private:
	bool _simulationRan;
	TArray<FAttackData> _simulationData;
	FAttackTargetData DecideNextAttackToRun(FAttackData LastAttackData) const;
	FAttackData RunAttack(FAttackTargetData AttackTargetData);
	static FBourrinRuntimeInfosData DrawWeapon(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData);
	static FBourrinRuntimeInfosData LoseWeapon(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData);
	static FBourrinRuntimeInfosData DealDamage(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData, int Damage);
	
	FBourrinData GetBourrinDataByName(const FString& Name) const;
	FWeaponData GetWeaponDataByName(FString Name) const;
};
