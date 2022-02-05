// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"  
#include "Interactable.h"
#include "ShooterProjectile.h"
#include "PlasmaGrenade.generated.h"

class UWidgetComponent;
class UCapsuleComponent;
class UNiagaraComponent;

#define GT_WEAPONS_PLASMA_BOMB FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb"))
#define GT_WEAPONS_PLASMA_BOMB_LAUNCHED FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Launched"))
#define GT_WEAPONS_PLASMA_BOMB_PICKABLE FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Pickable"))
#define GT_WEAPONS_PLASMA_BOMB_PICKABLE_IGNITED FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Pickable.Ignited"))
#define GT_WEAPONS_PLASMA_BOMB_STUCK FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Stuck"))
#define GT_WEAPONS_PLASMA_BOMB_STUCK_IGNITED FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Stuck.Ignited"))
#define GT_WEAPONS_PLASMA_BOMB_RECOVERED FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Recovered"))
#define GT_WEAPONS_PLASMA_BOMB_EXPLODE FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Explode"))

 
/**
 * 
 */
UCLASS( Blueprintable)
class SHOOTERGAME_API APlasmaGrenade : public AShooterProjectile ,public IInteractable
{
	GENERATED_BODY()

public:
	APlasmaGrenade();

	
	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly,Category="Shooter ASD")
	USphereComponent* PlayerDetector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly,Category="Shooter ASD")
	UNiagaraComponent* NiagaraComponent;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Shooter ASD | UI")
	UWidgetComponent* WidgetComponent;
    
	UPROPERTY(EditDefaultsOnly,Category="Shooter ASD | UI")
	TSubclassOf<UUserWidget> PickUpMessageClass;

	UPROPERTY(EditDefaultsOnly,Category="Shooter ASD | FX")
	USoundCue* ActivationSoundCue;

	UPROPERTY()
	UTimelineComponent* TimelineComponent;

	UPROPERTY()
	FOnTimelineFloat OnTimelineFloat;

	UPROPERTY()
	FOnTimelineEvent OnTimeLineFinish;
	
	UPROPERTY(ReplicatedUsing=OnRep_GameplayTagChanged)
	FGameplayTag PlasmaGrenadeState;
 	UFUNCTION()
	void OnRep_GameplayTagChanged();
 	 
	UPROPERTY(Transient, ReplicatedUsing=OnRep_InsideArea)
	bool bPlayerIsInsideArea;

	UFUNCTION()
	void OnRep_InsideArea();

	UPROPERTY(Transient, ReplicatedUsing=OnRep_FuseProgress)
	float FuseProgress;
	
	UFUNCTION()
	void OnRep_FuseProgress();


protected:
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

  
	
	UFUNCTION(Client,Reliable)
	void CreateMessageInteractionWidget();
	 
 
	UFUNCTION()
 	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	 void OnPlayerDetectorOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnPlayerDetectorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex );

	virtual void OnImpact(const FHitResult& HitResult) override;

	virtual void BeginDetonationCountDown();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnBeginDetonationCountDown(); 


	UFUNCTION()
	virtual  void PlasmaDetonation();

public:
	UFUNCTION(Server,Reliable)
	void ShootWithVelocity(FVector ShootVelocity);
	
	UFUNCTION(BlueprintCallable)
	 void ShootTo(FVector ShootDirection);
	 
	UFUNCTION(NetMulticast,Reliable)
	virtual void TryToStickToTarget(const FHitResult& HitResult);
	
	 
	virtual bool TryToPickUp(AShooterCharacter* PickInstigator) override;
	

	UFUNCTION()
	void OnFuseUpdate(float T);

	UFUNCTION()
   void OnFuseFinished()  ;
	
};
