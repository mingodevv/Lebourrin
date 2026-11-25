#include "Replay/CombatAction.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Replay/BourrinActor.h"
#include "Replay/LevelReferencesHolder.h"
#include "Animation/AnimSequence.h"

void UCombatAction::Init(const FAttackData& InAttackData, ALevelReferencesHolder* InReferencesHolder)
{
	AttackData = InAttackData;
	ReferencesHolder = InReferencesHolder;
}

void UCombatAction::Start()
{
	Finish();
}

FAttackData UCombatAction::GetAttackData() const
{
	return AttackData;
}

UWorld* UCombatAction::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !ReferencesHolder)
	{
		return nullptr;
	}
	return ReferencesHolder->GetWorld();
}

void UCombatAction::Finish()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	OnFinished.Broadcast(this);
}

void UCombatAction::Wait(float Seconds, TFunction<void()> Then)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Then();
		return;
	}
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [Then]() { Then(); }), Seconds, false);
}

void UCombatAction::WaitUntilArrived(ABourrinActor* Bourrin, TFunction<void()> Then)
{
	WaitUntilBothArrived(Bourrin, nullptr, MoveTemp(Then));
}

void UCombatAction::WaitUntilBothArrived(ABourrinActor* BourrinA, ABourrinActor* BourrinB, TFunction<void()> Then)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Then();
		return;
	}
	WaitedTime = 0.f;
	const float Step = 0.05f;
	World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, BourrinA, BourrinB, Then, Step]()
	{
		WaitedTime += Step;
		const bool bArrivedA = !BourrinA || BourrinA->HasArrived();
		const bool bArrivedB = !BourrinB || BourrinB->HasArrived();
		if ((bArrivedA && bArrivedB) || WaitedTime > 4.f)
		{
			if (!bArrivedA) { BourrinA->TeleportToMoveTarget(); }
			if (!bArrivedB) { BourrinB->TeleportToMoveTarget(); }
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			Then();
		}
	}), Step, true);
}

ABourrinActor* UCombatAction::GetAttacker() const
{
	return ReferencesHolder ? ReferencesHolder->FindBourrin(AttackData.AttackerName) : nullptr;
}

ABourrinActor* UCombatAction::GetTarget() const
{
	return ReferencesHolder ? ReferencesHolder->FindBourrin(AttackData.TargetName) : nullptr;
}

void UCombatAction::RefreshInfo(ABourrinActor* Bourrin, const FString& BourrinName) const
{
	const FBourrinRuntimeInfosData* State = AttackData.BourrinsStatesAtTheEnd.Find(BourrinName);
	if (Bourrin && State)
	{
		Bourrin->UpdateInfo(State->LifeLeft, State->EquippedWeaponName);
	}
}

void UDrawWeaponCombatAction::Start()
{
	ABourrinActor* Attacker = GetAttacker();
	if (!Attacker)
	{
		Finish();
		return;
	}
	const float Duration = Attacker->PlayCombatAnimation(Attacker->DrawWeaponAnimation);
	Attacker->ShowPopup(FString::Printf(TEXT("sort %s !"), *ABourrinActor::WeaponLabel(AttackData.AttackerWeaponNameDrawed)), FColor::Yellow, 1.f);
	Wait(FMath::Max(0.1f, Duration * 0.45f), [this]()
	{
		if (ABourrinActor* Drawer = GetAttacker())
		{
			Drawer->EquipWeapon(ReferencesHolder->FindWeaponData(AttackData.AttackerWeaponNameDrawed));
			Drawer->UpdateWeapon(AttackData.AttackerWeaponNameDrawed);
		}
		Wait(0.6f, [this]() { Finish(); });
	});
}

void UAttackCombatAction::Start()
{
	ABourrinActor* Attacker = GetAttacker();
	ABourrinActor* Target = GetTarget();
	if (!Attacker || !Target)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackCombatAction : Bourrin introuvable (%s ou %s)."), *AttackData.AttackerName, *AttackData.TargetName);
		Finish();
		return;
	}
	Attacker->MoveToLocation(Target->GetOpponentLocation());
	WaitUntilArrived(Attacker, [this]() { PlayAttack(); });
}

bool UAttackCombatAction::IsRealWeapon(const FString& WeaponName)
{
	return !WeaponName.IsEmpty() && WeaponName != TEXT("Hands");
}

void UAttackCombatAction::PlayAttack()
{
	ABourrinActor* Attacker = GetAttacker();
	ABourrinActor* Target = GetTarget();
	Attacker->FaceLocation(Target->GetActorLocation());
	Target->FaceLocation(Attacker->GetActorLocation());

	const FString& WeaponUsed = AttackData.AttackerWeaponNameDrawed.IsEmpty() ? AttackData.AttackerWeaponNameOnStart : AttackData.AttackerWeaponNameDrawed;
	UAnimSequence* Animation = IsRealWeapon(WeaponUsed) ? Attacker->WeaponAttackAnimation : Attacker->PunchAnimation;
	const float Duration = Attacker->PlayCombatAnimation(Animation);
	const float ImpactTime = Duration > 0.f ? Duration * 0.45f : 0.3f;
	AttackRemainingTime = FMath::Max(0.3f, Duration - ImpactTime);
	Wait(ImpactTime, [this]() { Impact(); });
}

void UAttackCombatAction::Impact()
{
	ABourrinActor* Attacker = GetAttacker();
	ABourrinActor* Target = GetTarget();

	const FBourrinRuntimeInfosData* TargetState = AttackData.BourrinsStatesAtTheEnd.Find(AttackData.TargetName);
	const bool bTargetDead = TargetState && TargetState->LifeLeft <= 0;

	switch (AttackData.AttackResult)
	{
	case EAttackResult::Hit:
		Target->ShowPopup(FString::Printf(TEXT("-%d"), AttackData.InflictedDamage), FColor::Red);
		if (bTargetDead)
		{
			Target->PlayDeath();
		}
		else
		{
			Target->PlayCombatAnimation(Target->HitAnimation);
		}
		break;
	case EAttackResult::Dodged:
		Target->ShowPopup(TEXT("Esquive !"), FColor::Cyan);
		Target->PlayCombatAnimation(Target->DodgeAnimation);
		break;
	case EAttackResult::Parried:
		Target->ShowPopup(AttackData.TargetLostWeaponAfterParry ? TEXT("Parade ! (perd son arme)") : TEXT("Parade !"), FColor::White);
		Target->PlayCombatAnimation(Target->ParryAnimation);
		if (AttackData.TargetLostWeaponAfterParry)
		{
			Target->UnequipWeapon();
		}
		break;
	}

	const FString& WeaponUsed = AttackData.AttackerWeaponNameDrawed.IsEmpty() ? AttackData.AttackerWeaponNameOnStart : AttackData.AttackerWeaponNameDrawed;
	if (IsRealWeapon(WeaponUsed) && !IsRealWeapon(AttackData.AttackerWeaponNameOnEnd))
	{
		Attacker->ShowPopup(TEXT("perd son arme !"), FColor::Orange);
		Attacker->UnequipWeapon();
	}

	RefreshInfo(Target, AttackData.TargetName);
	RefreshInfo(Attacker, AttackData.AttackerName);

	Wait(AttackRemainingTime + 0.3f, [this]() { GoBack(); });
}

void UAttackCombatAction::GoBack()
{
	ABourrinActor* Attacker = GetAttacker();
	Attacker->MoveToLocation(Attacker->GetAnchorLocation());
	WaitUntilArrived(Attacker, [this]() { End(); });
}

void UAttackCombatAction::End()
{
	ABourrinActor* Attacker = GetAttacker();
	ABourrinActor* Target = GetTarget();
	const FBourrinRuntimeInfosData* TargetState = AttackData.BourrinsStatesAtTheEnd.Find(AttackData.TargetName);
	Attacker->FaceLocation(Target->GetAnchorLocation());
	if (!TargetState || TargetState->LifeLeft > 0)
	{
		Target->FaceLocation(Attacker->GetAnchorLocation());
	}
	Wait(0.3f, [this]() { Finish(); });
}
