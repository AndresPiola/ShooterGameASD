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
	
	
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Plasma Grenade Settings")
	float PlasmaGrenadeSpeed=2000;
	

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EPlasmaGrenade;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FProjectileWeaponData ProjectileConfig;
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
	
	
};
