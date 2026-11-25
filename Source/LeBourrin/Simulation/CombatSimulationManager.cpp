// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSimulationManager.h"


bool ACombatSimulationManager::SimulationRan() const
{
	return _simulationRan;
}

// Sets default values
ACombatSimulationManager::ACombatSimulationManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Verbose, TEXT("Combat Simulation Manager built."))
}

// Called when the game starts or when spawned
void ACombatSimulationManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Verbose, TEXT("Combat Simulation Manager began play."))
	_simulationData = RunSimulation();
	_simulationRan = true;
	OnSimulationRan.Broadcast(_simulationData);
	UE_LOG(LogTemp, Verbose, TEXT("Combat Simulation Manager completed simulation."))
}

// Called every frame
void ACombatSimulationManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<FAttackData> ACombatSimulationManager::RunSimulation()
{
	const auto Bourrin1Data = GetBourrinDataByName(Player1Name);
	const auto Bourrin2Data = GetBourrinDataByName(Player2Name);
	
	auto Bourrin1RuntimeInfos = FBourrinRuntimeInfosData();
	Bourrin1RuntimeInfos.LifeLeft = Bourrin1Data.Life;
	Bourrin1RuntimeInfos.InInventoryWeapons = Bourrin1Data.Weapons;
	
	auto Bourrin2RuntimeInfos = FBourrinRuntimeInfosData();
	Bourrin2RuntimeInfos.LifeLeft = Bourrin2Data.Life;
	Bourrin2RuntimeInfos.InInventoryWeapons = Bourrin2Data.Weapons;
	
	TArray<FAttackData> AttackDatas = TArray<FAttackData>();
	
	while (Bourrin1RuntimeInfos.LifeLeft > 0 && Bourrin2RuntimeInfos.LifeLeft > 0)
	{
		FAttackTargetData targetData;
		if (AttackDatas.Num() > 0)
		{
			targetData = DecideNextAttackToRun(AttackDatas.Last());
		}
		else
		{
			// first round
			const auto rand = FMath::Rand();
			if (rand > 0.5f)
			{
				targetData.AttackerName = Player1Name;
				targetData.AttackerRuntimeInfosData = Bourrin1RuntimeInfos;
				targetData.Attacker = Bourrin1Data;
				
				targetData.TargetName = Player2Name;
				targetData.TargetRuntimeInfosData = Bourrin2RuntimeInfos;
				targetData.Target = Bourrin2Data;
			}
			else
			{
				targetData.AttackerName = Player2Name;
				targetData.AttackerRuntimeInfosData = Bourrin2RuntimeInfos;
				targetData.Attacker = Bourrin2Data;
				
				targetData.TargetName = Player1Name;
				targetData.TargetRuntimeInfosData = Bourrin1RuntimeInfos;
				targetData.Target = Bourrin1Data;
			}
		}
		
		AttackDatas.Add(RunAttack(targetData));
		
		const auto LastAttackData = AttackDatas.Last();
		Bourrin1RuntimeInfos = LastAttackData.BourrinsStatesAtTheEnd[Player1Name];
		Bourrin2RuntimeInfos = LastAttackData.BourrinsStatesAtTheEnd[Player2Name];
	}
	
	return AttackDatas;
}

TArray<FAttackData> ACombatSimulationManager::GetSimulationData() const
{
	return _simulationData;
}

FAttackTargetData ACombatSimulationManager::DecideNextAttackToRun(FAttackData LastAttackData) const
{
	const auto LastAttackerData = GetBourrinDataByName(LastAttackData.AttackerName);
	const auto AttackerWeapon = GetWeaponDataByName(LastAttackData.AttackerWeaponNameOnEnd);
	const auto AttackerExhaust = LastAttackData.BourrinsStatesAtTheEnd[LastAttackData.AttackerName].Exhaust;
	const auto AttackerSpeed = FMath::Max(0, LastAttackerData.Speed + AttackerWeapon.SpeedIncrement - AttackerExhaust);
	
	const auto LastTargetData = GetBourrinDataByName(LastAttackData.TargetName);
	const auto TargetSpeed = LastTargetData.Speed;
	
	const float ChanceToReattack = FMath::Max(0, (AttackerSpeed - TargetSpeed)) / static_cast<float>(TargetSpeed);
	const float randNumber = FMath::FRand();
	
	if (randNumber < ChanceToReattack)
	{
		// Reattack!
		auto attackTargetData = FAttackTargetData();
		
		attackTargetData.Attacker = LastAttackerData;
		attackTargetData.AttackerName = LastAttackData.AttackerName;
		attackTargetData.AttackerRuntimeInfosData = LastAttackData.BourrinsStatesAtTheEnd[LastAttackData.AttackerName];
		attackTargetData.AttackerRuntimeInfosData.Exhaust += TargetSpeed;
		
		attackTargetData.Target = LastTargetData;
		attackTargetData.TargetName = LastAttackData.TargetName;
		attackTargetData.TargetRuntimeInfosData = LastAttackData.BourrinsStatesAtTheEnd[LastAttackData.TargetName];
		
		return attackTargetData;
	}
	else
	{
		// Moving to next player...
		auto attackTargetData = FAttackTargetData();
		
		attackTargetData.Attacker = LastTargetData;
		attackTargetData.AttackerName = LastAttackData.TargetName;
		attackTargetData.AttackerRuntimeInfosData = LastAttackData.BourrinsStatesAtTheEnd[LastAttackData.TargetName];
		attackTargetData.AttackerRuntimeInfosData.Exhaust = 0;
		
		attackTargetData.Target = LastAttackerData;
		attackTargetData.TargetName = LastAttackData.AttackerName;
		attackTargetData.TargetRuntimeInfosData = LastAttackData.BourrinsStatesAtTheEnd[LastAttackData.AttackerName];
		attackTargetData.TargetRuntimeInfosData.Exhaust = 0;
		
		return attackTargetData;
	}
}

FAttackData ACombatSimulationManager::RunAttack(FAttackTargetData AttackTargetData)
{
	FAttackData AttackData = FAttackData();
	
	auto AttackerRuntimeInfos = AttackTargetData.AttackerRuntimeInfosData;
	auto AttackerData = AttackTargetData.Attacker;
	auto AttackerWeaponData = GetWeaponDataByName(AttackerRuntimeInfos.EquippedWeaponName);
	
	auto TargetRuntimeInfos = AttackTargetData.TargetRuntimeInfosData;
	auto TargetData = AttackTargetData.Target;
	auto TargetWeaponData = GetWeaponDataByName(TargetRuntimeInfos.EquippedWeaponName);
	
	AttackData.AttackerName = AttackTargetData.AttackerName;
	AttackData.AttackerWeaponNameOnStart = AttackerRuntimeInfos.EquippedWeaponName;
	
	AttackData.TargetName = AttackTargetData.TargetName;
	
	// If empty handed, try to draw weapons
	if (AttackerRuntimeInfos.EquippedWeaponName.IsEmpty() || AttackerRuntimeInfos.EquippedWeaponName == "Hands")
	{
		const auto ChanceToDrawWeapon = AttackerData.ChanceToDrawANewWeaponWhenEmptyHanded;
		const float randNumber = FMath::FRand();
		
		if (ChanceToDrawWeapon > randNumber)
		{
			AttackerRuntimeInfos = DrawWeapon(AttackerRuntimeInfos);
			AttackerWeaponData = GetWeaponDataByName(AttackerRuntimeInfos.EquippedWeaponName);
			AttackData.AttackerWeaponNameDrawed = AttackerRuntimeInfos.EquippedWeaponName;
		}
	}
	
	// Calculate damage
	int32 weaponDamage = FMath::RandRange(AttackerWeaponData.DamageToDeal.X, AttackerWeaponData.DamageToDeal.Y);
	int32 damage = AttackerData.Force +  weaponDamage;
	EAttackResult AttackResult = EAttackResult::Hit;
	
	// Dodge chance
	float ChanceToDodge = FMath::Max(0, TargetData.Agility - AttackerData.Agility) / AttackerData.Agility;
	float RandNumber = FMath::FRand();
	if (ChanceToDodge > RandNumber)
	{
		damage = 0;
		AttackResult = EAttackResult::Dodged;
	}
	// Parry chance
	else
	{
		float ChanceToParry = TargetWeaponData.ChanceToParry;
		RandNumber = FMath::FRand();
		if (ChanceToParry > RandNumber)
		{
			damage = 0;
			AttackResult = EAttackResult::Parried;
			
			float chanceToKeepWeaponAfterParrying = TargetWeaponData.ChanceToKeepWeaponAfterParrying;
			RandNumber = FMath::FRand();
			
			if (chanceToKeepWeaponAfterParrying < RandNumber)
			{
				// Lose weapon
				TargetRuntimeInfos = LoseWeapon(TargetRuntimeInfos);
				TargetWeaponData = GetWeaponDataByName(TargetRuntimeInfos.EquippedWeaponName);
				AttackData.TargetLostWeaponAfterParry = true;
			}
		}
	}
	
	// Deal damage
	if (AttackResult == EAttackResult::Hit)
	{
		TargetRuntimeInfos = DealDamage(TargetRuntimeInfos, damage);
	}
	
	// Attacker can lose weapon after hit or parry
	if (AttackResult == EAttackResult::Hit || AttackResult == EAttackResult::Parried)
	{
		float ChanceToKeepWeapon = AttackerWeaponData.ChanceToKeepWeaponAfterAttacking;
		RandNumber = FMath::FRand();
		
		if (ChanceToKeepWeapon < RandNumber)
		{
			// Lose Weapon
			AttackerRuntimeInfos = LoseWeapon(AttackerRuntimeInfos);
			AttackerWeaponData = GetWeaponDataByName(AttackerRuntimeInfos.EquippedWeaponName);
		}
	}
	
	// Constructing AttackData
	AttackData.AttackerWeaponNameOnEnd = AttackerRuntimeInfos.EquippedWeaponName;
	AttackData.InflictedDamage = damage;
	AttackData.AttackResult = AttackResult;
	
	AttackData.BourrinsStatesAtTheEnd.Add(AttackData.AttackerName, AttackerRuntimeInfos);
	AttackData.BourrinsStatesAtTheEnd.Add(AttackData.TargetName, TargetRuntimeInfos);
	
	AttackData.IsValid = true;
	
	return AttackData;
}

FBourrinRuntimeInfosData ACombatSimulationManager::DrawWeapon(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData)
{
	auto Weapons = BourrinRuntimeInfosData.InInventoryWeapons;
	if (Weapons.Num() == 0)
		return BourrinRuntimeInfosData;
	const int32 RandomWeaponIndex = FMath::RandRange(0, Weapons.Num() - 1);
	const auto WeaponName = Weapons[RandomWeaponIndex];
	Weapons.RemoveAt(RandomWeaponIndex);
	
	auto InfosData = BourrinRuntimeInfosData;
	InfosData.EquippedWeaponName = WeaponName;
	InfosData.InInventoryWeapons = Weapons;
	
	return InfosData;
}

FBourrinRuntimeInfosData ACombatSimulationManager::LoseWeapon(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData)
{
	auto InfosData = BourrinRuntimeInfosData;
	InfosData.EquippedWeaponName = "Hands";
	return InfosData;
}

FBourrinRuntimeInfosData ACombatSimulationManager::DealDamage(const FBourrinRuntimeInfosData& BourrinRuntimeInfosData,
	int Damage)
{
	auto InfosData = BourrinRuntimeInfosData;
	InfosData.LifeLeft = FMath::Max(InfosData.LifeLeft - Damage, 0);
	return InfosData;
}

FBourrinData ACombatSimulationManager::GetBourrinDataByName(const FString& Name) const
{
	if (BourrinsTable)
	{
		auto bourrinDataPtr = BourrinsTable->FindRow<FBourrinData>(FName(Name), FString::Printf(TEXT("Reading bourrin named %s"), *Name));
		if (bourrinDataPtr)
		{
			return *bourrinDataPtr;
		}
	}
	return FBourrinData();
}

FWeaponData ACombatSimulationManager::GetWeaponDataByName(FString Name) const
{
	if (WeaponsTable)
	{
		auto weaponDataPtr = WeaponsTable->FindRow<FWeaponData>(FName(Name), FString::Printf(TEXT("Reading weapon named %s"), *Name));
		if (weaponDataPtr)
		{
			return *weaponDataPtr;
		}
	}
	return FWeaponData();
}

