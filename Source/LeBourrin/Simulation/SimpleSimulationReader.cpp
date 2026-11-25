// Fill out your copyright notice in the Description page of Project Settings.
#include "SimpleSimulationReader.h"

DEFINE_LOG_CATEGORY(LogSimulationReader);

ASimpleSimulationReader::ASimpleSimulationReader()
{
}

ASimpleSimulationReader::~ASimpleSimulationReader()
{
	if (CombatSimulationManager)
	{
		CombatSimulationManager->OnSimulationRan.RemoveDynamic(this, &ASimpleSimulationReader::HandleSimulationRan);
	}
}

void ASimpleSimulationReader::ReadSimulationData() const
{
	const auto SimulationData = CombatSimulationManager->GetSimulationData();
	UE_LOG(LogSimulationReader, Display, TEXT("Steps count in simulation: %d"), SimulationData.Num());
	FString Log = "\n";
	FAttackData PreviousAttackData = FAttackData();
		
	for (int i = 0; i < SimulationData.Num(); i++)
	{
		Log += ParseAttackData(PreviousAttackData, SimulationData[i]);
		Log += "-------\n";
		
		PreviousAttackData = SimulationData[i];
	}
	
	Log += PreviousAttackData.TargetName + " is dead. " + PreviousAttackData.AttackerName + " wins!";
	
	UE_LOG(LogSimulationReader, Display, TEXT("%s"), *Log);
}

void ASimpleSimulationReader::HandleSimulationRan(TArray<FAttackData> SimulationData)
{
	ReadSimulationData();
}

void ASimpleSimulationReader::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Error, TEXT("Simple simulation reader began play."))
	if (!CombatSimulationManager->SimulationRan())
	{
		CombatSimulationManager->OnSimulationRan.AddDynamic(this, &ASimpleSimulationReader::HandleSimulationRan);
	}
	else
	{
		HandleSimulationRan(CombatSimulationManager->GetSimulationData());
	}
}

FString ASimpleSimulationReader::ParseAttackData(const FAttackData PreviousAttackData, const FAttackData AttackData) const
{
	FString Log;
	
	if (PreviousAttackData.AttackerName == AttackData.AttackerName)
	{
		Log += AttackData.AttackerName + " is about to attack again!\n";
	}
	
	Log += ProcessWeaponDrawing(AttackData);
	Log += ProcessResult(AttackData);
	Log += ProcessParrierWeaponLost(AttackData);
	Log += ProcessAttackerWeaponLost(AttackData);
	
	return Log;
}

FString ASimpleSimulationReader::ProcessWeaponDrawing(const FAttackData AttackData) const
{
	if (AttackData.AttackerWeaponNameDrawed.IsEmpty())
		return "";
		
	return AttackData.AttackerName + " draws " + AttackData.AttackerWeaponNameDrawed +"\n";
}

FString ASimpleSimulationReader::ProcessResult(const FAttackData AttackData) const
{
	switch (AttackData.AttackResult)
	{
	case EAttackResult::Hit:
		return AttackData.AttackerName + " hits " + AttackData.TargetName + " for " + FString::FromInt(AttackData.InflictedDamage) + "damage." + "\n";
	case EAttackResult::Dodged:
		return AttackData.AttackerName + " tries to hit " + AttackData.TargetName + ", but " + AttackData.TargetName + " dodges it. \n";
	case EAttackResult::Parried:
		return AttackData.AttackerName + " tries to hit " + AttackData.TargetName + ", but " + AttackData.TargetName + " parries it. \n";
	}
	return "";
}

FString ASimpleSimulationReader::ProcessParrierWeaponLost(const FAttackData AttackData) const
{
	if (!AttackData.TargetLostWeaponAfterParry)
		return "";
	
	return AttackData.TargetName + " dropped its when weapon when parrying.\n";
}

bool IsWeaponNameValid(FString WeaponName)
{
	return !WeaponName.IsEmpty() && WeaponName != "Hands";
}

FString ASimpleSimulationReader::ProcessAttackerWeaponLost(const FAttackData AttackData) const
{
	if (IsWeaponNameValid(AttackData.AttackerWeaponNameOnEnd))
		return "";
	
	if (!IsWeaponNameValid(AttackData.AttackerWeaponNameOnStart) && !IsWeaponNameValid(AttackData.AttackerWeaponNameDrawed))
		return "";
		
	return AttackData.AttackerName + " dropped its weapon when attacking.\n";
}

