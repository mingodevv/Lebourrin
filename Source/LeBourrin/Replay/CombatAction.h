#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/AttackData.h"

#include "CombatAction.generated.h"

class ALevelReferencesHolder;
class ABourrinActor;
class UCombatAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatActionFinished, UCombatAction*, Action);

UCLASS(Abstract, Blueprintable)
class LEBOURRIN_API UCombatAction : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="CombatAction")
	FOnCombatActionFinished OnFinished;

	void Init(const FAttackData& InAttackData, ALevelReferencesHolder* InReferencesHolder);

	virtual void Start();

	UFUNCTION(BlueprintPure, Category="CombatAction")
	FAttackData GetAttackData() const;

	virtual UWorld* GetWorld() const override;

protected:
	void Finish();
	void Wait(float Seconds, TFunction<void()> Then);
	void WaitUntilArrived(ABourrinActor* Bourrin, TFunction<void()> Then);
	void WaitUntilBothArrived(ABourrinActor* BourrinA, ABourrinActor* BourrinB, TFunction<void()> Then);

	ABourrinActor* GetAttacker() const;
	ABourrinActor* GetTarget() const;
	void RefreshInfo(ABourrinActor* Bourrin, const FString& BourrinName) const;

	UPROPERTY()
	FAttackData AttackData;

	UPROPERTY()
	TObjectPtr<ALevelReferencesHolder> ReferencesHolder;

	FTimerHandle TimerHandle;
	float WaitedTime = 0.f;
};

UCLASS()
class LEBOURRIN_API UDrawWeaponCombatAction : public UCombatAction
{
	GENERATED_BODY()

public:
	virtual void Start() override;
};

UCLASS()
class LEBOURRIN_API UAttackCombatAction : public UCombatAction
{
	GENERATED_BODY()

public:
	virtual void Start() override;

private:
	static bool IsRealWeapon(const FString& WeaponName);
	void PlayAttack();
	void Impact();
	void GoBack();
	void End();

	float AttackRemainingTime = 0.f;
};
