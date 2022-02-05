// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_PlasmaGrenade.h"


#include "PlasmaGrenade.h"
#include "ShooterProjectile.h" 
#include "Kismet/KismetMathLibrary.h"

AShooterWeapon_PlasmaGrenade::AShooterWeapon_PlasmaGrenade()
{
	WeaponConfig.InitialClips=1;

	PlasmaBombRecoveryAudio=CreateDefaultSubobject<UAudioComponent>(TEXT("PlasmaBombRecoveryAudio"));
	 PlasmaBombRecoveryAudio->SetupAttachment(RootComponent);
 
	PlasmaBombRecoveryAudio->Deactivate();
}

void AShooterWeapon_PlasmaGrenade::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}

void AShooterWeapon_PlasmaGrenade::FireWeapon()
{ 
	
	FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();

	// trace from camera to check what's under crosshair
	const float ProjectileAdjustRange = 10000.0f;
	const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	
	// and adjust directions to hit that actor
	if (Impact.bBlockingHit)
	{
		const FVector AdjustedDir = (Impact.ImpactPoint - Origin).GetSafeNormal();
		bool bWeaponPenetration = false;

		const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
		if (DirectionDot < 0.0f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (DirectionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
			FVector MuzzleEndTrace = Origin;
			FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			Origin = Impact.ImpactPoint - ShootDir * 10.0f;
		}
		else
		{
			// adjust direction to hit
			ShootDir = AdjustedDir;
		}
	}
	
	const FVector FinalLocation=Impact.bBlockingHit?Impact.ImpactPoint:EndTrace;
	
	FVector LaunchVelocity;
		
 

	FVector EndPoint=Origin+(ShootDir*2000000.0f);
	FHitResult HitResult2=  WeaponTrace(Origin,EndPoint);
	
	
	UGameplayStatics::SuggestProjectileVelocity_CustomArc(this,LaunchVelocity,Origin,HitResult2.ImpactPoint,0,0.5);
	OnWeaponFired(LaunchVelocity,Impact,Origin,FinalLocation,LaunchVelocity.GetSafeNormal(),LaunchVelocity.Size());
	 ServerFireProjectileWithVelocity(Origin,LaunchVelocity.GetSafeNormal(),LaunchVelocity.Size());
}

void AShooterWeapon_PlasmaGrenade::PlayPickupAnimationFX_Implementation()
{
	if(!PlasmaBombRecoveryAudio->IsActive())
		PlasmaBombRecoveryAudio->Activate();
	PlasmaBombRecoveryAudio->Play();
	
}

void AShooterWeapon_PlasmaGrenade::ServerFireProjectileWithVelocity_Implementation(FVector Origin,
                                                                                   FVector_NetQuantizeNormal Direction, float LaunchSpeed)
{
	FTransform SpawnTM(GetAdjustedAim().Rotation(), Origin);
	APlasmaGrenade* Projectile = Cast<APlasmaGrenade>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);
		Projectile->ShootWithVelocity(Direction*LaunchSpeed); 

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
		Projectile->ShootWithVelocity(Direction*LaunchSpeed);
	}
}

bool AShooterWeapon_PlasmaGrenade::ServerFireProjectileWithVelocity_Validate(FVector Origin,
	FVector_NetQuantizeNormal Direction, float LaunchSpeed)
{
	return true;
}

 