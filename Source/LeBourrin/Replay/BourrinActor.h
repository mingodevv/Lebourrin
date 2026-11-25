#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "BourrinActor.generated.h"

class UTextRenderComponent;
class UStaticMeshComponent;
class UAnimSequence;
class UAnimInstance;
struct FBourrinVisualData;
struct FWeaponData;

UCLASS()
class LEBOURRIN_API ABourrinActor : public ACharacter
{
	GENERATED_BODY()

public:
	ABourrinActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin")
	FString BourrinName;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Bourrin")
	TObjectPtr<AActor> AnchorPoint;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Bourrin")
	TObjectPtr<AActor> OpponentPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin")
	float OpponentDistance = 130.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TSubclassOf<UAnimInstance> BourrinAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> PunchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> WeaponAttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> HitAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> DodgeAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> ParryAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> DrawWeaponAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> DeathAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Animations")
	TObjectPtr<UAnimSequence> VictoryAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Weapon")
	FName DefaultWeaponSocket = TEXT("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Weapon")
	FVector WeaponOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bourrin|Weapon")
	FRotator WeaponRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Bourrin")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Bourrin")
	TObjectPtr<UTextRenderComponent> InfoText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Bourrin")
	TObjectPtr<UTextRenderComponent> PopupText;

	void ApplyVisual(const FBourrinVisualData& Visual);

	void EquipWeapon(const FWeaponData* Weapon);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	float PlayCombatAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category="Bourrin")
	FVector GetAnchorLocation() const;

	UFUNCTION(BlueprintPure, Category="Bourrin")
	FVector GetOpponentLocation() const;

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void SetDefaultOpponentLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void MoveToLocation(FVector Target);

	UFUNCTION(BlueprintPure, Category="Bourrin")
	bool HasArrived() const;

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void TeleportToMoveTarget();

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void FaceLocation(FVector Target);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void ShowPopup(const FString& Text, FColor Color, float Duration = 1.2f);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void UpdateInfo(int32 LifeLeft, const FString& WeaponName);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void UpdateWeapon(const FString& WeaponName);

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void PlayDeath();

	UFUNCTION(BlueprintCallable, Category="Bourrin")
	void PlayVictory();

	static FString WeaponLabel(const FString& WeaponName);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void FaceTextToCamera(UTextRenderComponent* Text) const;
	void PlaceTextsAboveHead();

	FVector StartLocation = FVector::ZeroVector;
	bool bHasStartLocation = false;
	FVector DefaultOpponentLocation = FVector::ZeroVector;
	bool bHasDefaultOpponentLocation = false;
	FVector MoveTarget = FVector::ZeroVector;
	bool bMoving = false;
	float PopupTimeLeft = 0.f;
	int32 DisplayedLife = 0;
};
