#include "Replay/CombatPlayer.h"

#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Data/BourrinData.h"
#include "Simulation/CombatSimulationManager.h"
#include "Replay/BourrinActor.h"
#include "Replay/CombatAction.h"
#include "Replay/LevelReferencesHolder.h"
#include "Replay/BourrinVisualData.h"

ACombatPlayer::ACombatPlayer()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACombatPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (bRandomFighters && GetWorld() && GetWorld()->IsGameWorld())
	{
		PickRandomFighters();
	}
}

void ACombatPlayer::PickRandomFighters()
{
	ACombatSimulationManager* Manager = CombatSimulationManager;
	if (!Manager)
	{
		TActorIterator<ACombatSimulationManager> It(GetWorld());
		Manager = It ? *It : nullptr;
	}
	if (!Manager || !Manager->BourrinsTable)
	{
		return;
	}
	const TArray<FName> RowNames = Manager->BourrinsTable->GetRowNames();
	if (RowNames.Num() < 2)
	{
		return;
	}
	const int32 First = FMath::RandRange(0, RowNames.Num() - 1);
	int32 Second = FMath::RandRange(0, RowNames.Num() - 2);
	if (Second >= First)
	{
		++Second;
	}
	Manager->Player1Name = RowNames[First].ToString();
	Manager->Player2Name = RowNames[Second].ToString();
	UE_LOG(LogTemp, Warning, TEXT("CombatPlayer : combat tire au sort, %s contre %s."), *Manager->Player1Name, *Manager->Player2Name);
}

void ACombatPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (!CombatSimulationManager)
	{
		TActorIterator<ACombatSimulationManager> It(GetWorld());
		CombatSimulationManager = It ? *It : nullptr;
	}
	if (!ReferencesHolder)
	{
		TActorIterator<ALevelReferencesHolder> It(GetWorld());
		ReferencesHolder = It ? *It : nullptr;
	}
	if (!CombatSimulationManager || !ReferencesHolder)
	{
		UE_LOG(LogTemp, Error, TEXT("CombatPlayer : il manque le CombatSimulationManager ou le LevelReferencesHolder dans le niveau."));
		return;
	}

	if (CombatSimulationManager->SimulationRan())
	{
		HandleSimulationReady(CombatSimulationManager->GetSimulationData());
	}
	else
	{
		CombatSimulationManager->OnSimulationRan.AddDynamic(this, &ACombatPlayer::HandleSimulationReady);
	}
}

void ACombatPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CombatSimulationManager)
	{
		CombatSimulationManager->OnSimulationRan.RemoveDynamic(this, &ACombatPlayer::HandleSimulationReady);
	}
	Super::EndPlay(EndPlayReason);
}

void ACombatPlayer::HandleSimulationReady(TArray<FAttackData> SimulationData)
{
	PlayedData = SimulationData;
	ReferencesHolder->WeaponsTable = CombatSimulationManager->WeaponsTable;
	ReferencesHolder->PrepareBourrins(CombatSimulationManager->Player1Name, CombatSimulationManager->Player2Name);
	ApplyVisuals();
	ReferencesHolder->PrepareBourrins(CombatSimulationManager->Player1Name, CombatSimulationManager->Player2Name);
	ShowStartInfos(SimulationData);
	BuildQueue(SimulationData);
	UE_LOG(LogTemp, Warning, TEXT("CombatPlayer : %d etapes -> %d actions dans la file."), SimulationData.Num(), ActionsQueue.Num());
	GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &ACombatPlayer::PlayNext, StartDelay, false);
}

void ACombatPlayer::ShowStartInfos(const TArray<FAttackData>& SimulationData)
{
	for (const FString& Name : { CombatSimulationManager->Player1Name, CombatSimulationManager->Player2Name })
	{
		ABourrinActor* Bourrin = ReferencesHolder->FindBourrin(Name);
		if (!Bourrin)
		{
			continue;
		}
		int32 Life = 0;
		if (CombatSimulationManager->BourrinsTable)
		{
			if (const FBourrinData* Data = CombatSimulationManager->BourrinsTable->FindRow<FBourrinData>(FName(*Name), TEXT("CombatPlayer"), false))
			{
				Life = Data->Life;
			}
		}
		Bourrin->UpdateInfo(Life, FString());
	}
}

void ACombatPlayer::ApplyVisuals()
{
	if (!BourrinVisualsTable)
	{
		BourrinVisualsTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Characters/DT_BourrinVisuals.DT_BourrinVisuals"));
	}
	if (!BourrinVisualsTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatPlayer : pas de DT_BourrinVisuals, les Bourrins gardent le mannequin."));
		return;
	}
	for (const FString& Name : { CombatSimulationManager->Player1Name, CombatSimulationManager->Player2Name })
	{
		ABourrinActor* Bourrin = ReferencesHolder->FindBourrin(Name);
		const FBourrinVisualData* Visual = BourrinVisualsTable->FindRow<FBourrinVisualData>(FName(*Name), TEXT("CombatPlayer"), false);
		if (Bourrin && Visual)
		{
			Bourrin->ApplyVisual(*Visual);
		}
	}
}

void ACombatPlayer::BuildQueue(const TArray<FAttackData>& SimulationData)
{
	ActionsQueue.Empty();
	for (const FAttackData& Step : SimulationData)
	{
		if (!Step.AttackerWeaponNameDrawed.IsEmpty() && Step.AttackerWeaponNameDrawed != TEXT("Hands"))
		{
			UDrawWeaponCombatAction* DrawAction = NewObject<UDrawWeaponCombatAction>(this);
			DrawAction->Init(Step, ReferencesHolder);
			ActionsQueue.Add(DrawAction);
		}
		UAttackCombatAction* AttackAction = NewObject<UAttackCombatAction>(this);
		AttackAction->Init(Step, ReferencesHolder);
		ActionsQueue.Add(AttackAction);
	}
}

void ACombatPlayer::PlayNext()
{
	if (ActionsQueue.Num() == 0)
	{
		CurrentAction = nullptr;
		ShowWinner();
		return;
	}
	CurrentAction = ActionsQueue[0];
	ActionsQueue.RemoveAt(0);
	CurrentAction->OnFinished.AddDynamic(this, &ACombatPlayer::HandleActionFinished);
	CurrentAction->Start();
}

void ACombatPlayer::HandleActionFinished(UCombatAction* Action)
{
	if (Action)
	{
		Action->OnFinished.RemoveDynamic(this, &ACombatPlayer::HandleActionFinished);
	}
	PlayNext();
}

void ACombatPlayer::ShowWinner()
{
	if (PlayedData.Num() == 0)
	{
		return;
	}
	for (const TPair<FString, FBourrinRuntimeInfosData>& Pair : PlayedData.Last().BourrinsStatesAtTheEnd)
	{
		if (Pair.Value.LifeLeft > 0)
		{
			if (ABourrinActor* Winner = ReferencesHolder->FindBourrin(Pair.Key))
			{
				Winner->PlayVictory();
			}
			UE_LOG(LogTemp, Warning, TEXT("CombatPlayer : %s gagne le combat."), *Pair.Key);
		}
	}
}
