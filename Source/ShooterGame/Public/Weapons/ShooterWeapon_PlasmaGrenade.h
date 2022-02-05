// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "ShooterWeapon_PlasmaGrenade.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AShooterWeapon_PlasmaGrenade : public AShooterWeapon
{
	GENERATED_BODY()

	AShooterWeapon_PlasmaGrenade();
	UPROPERTY(VisibleDefaultsOnly,Category="Plasma Grenade Settings")
	UAudioComponent* PlasmaBombRecoveryAudio;
 
	//for recovery info support
	UPROPERTY()
    UTimelineComponent* TimelineComponent;
    
	UPROPERTY()
    FOnTimelineFloat OnTimelineFloat;

	UPROPERTY()
    FOnTimelineEvent OnTimeLineFinish;

	 
protected:
	virtual void PostInitializeComponents() override;
	
public:
	
	UPROPERTY(EditDefaultsOnly,Category="Plasma Grenade Settings")
	UCurveFloat* RecoveryPlasmaBombCurve;
	

	
	 

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EPlasmaGrenade;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FProjectileWeaponData ProjectileConfig;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_RecoveryAnimationFuseProgress)
	float RecoveryAnimationProgress;
	UFUNCTION()
	void OnRep_RecoveryAnimationFuseProgress();

 
	
public:
	/** apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponFired(FVector ProjectileVelocity,FHitResult WeaponTraceResult,FVector Origin,FVector Target,FVector ShootDirection,float SpeedLaunch);
 
	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectileWithVelocity(FVector Origin, FVector_NetQuantizeNormal Direction,float LaunchSpeed);

	UFUNCTION(Client,Unreliable)
	void PlayPickupAnimationFX();
	
	UFUNCTION()
	void OnRecoveryUpdate(float T);

};
