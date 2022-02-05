// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"  
#include "Interactable.h"
#include "ShooterProjectile.h"
#include "PlasmaGrenade.generated.h"

class UWidgetComponent;
class UCapsuleComponent;
static struct   
{
	 
	FGameplayTag Launched=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Launched"));
	FGameplayTag Pickable=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Pickable"));
	FGameplayTag PickableIgnited=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Pickable.Ignited"));
	FGameplayTag Stuck=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Stuck"));
	FGameplayTag StuckIgnited=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Stuck.Ignited")); 
	FGameplayTag Recovered=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Recovered")); 
	FGameplayTag Explode=FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb.State.Explode")); 

	
}FPlasmaGrenadeGTStates;

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

	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Shooter ASD | UI")
	UWidgetComponent* WidgetComponent;
    
	UPROPERTY(EditDefaultsOnly,Category="Shooter ASD | UI")
	TSubclassOf<UUserWidget> PickUpMessageClass;

	UPROPERTY(  ReplicatedUsing=OnRep_GameplayTagChanged)
	FGameplayTag PlasmaGrenadeState;
 	UFUNCTION()
	void OnRep_GameplayTagChanged();
 	 
	UPROPERTY(Transient, ReplicatedUsing=OnRep_InsideArea)
	bool bPlayerIsInsideArea;

	UFUNCTION()
	void OnRep_InsideArea();

	

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
	
	
	
};
