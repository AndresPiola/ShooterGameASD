// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_PlasmaGrenade.h"

#include "ShooterProjectile.h"
#include "Kismet/KismetMathLibrary.h"

AShooterWeapon_PlasmaGrenade::AShooterWeapon_PlasmaGrenade()
{
	WeaponConfig.InitialClips=1;
	
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
		FVector LaunchVelocity;
	const FVector FinalLocation=Impact.bBlockingHit?Impact.ImpactPoint:EndTrace;
	
	bool bHasLaunchVelocity= UGameplayStatics::SuggestProjectileVelocity(this,LaunchVelocity,Origin,FinalLocation,PlasmaGrenadeSpeed,false,0,0,	ESuggestProjVelocityTraceOption::DoNotTrace);


	FVector TargetPosition = Origin;

	const FRotator LaunchRotation = UKismetMathLibrary::FindLookAtRotation(Origin, Impact.ImpactPoint);

	const float Gravity = FMath::Abs(GetWorld()->GetGravityZ()) ;  
  
	OnWeaponFired(LaunchVelocity,Impact,Origin,FinalLocation,ShootDir,2000);
	
}

void AShooterWeapon_PlasmaGrenade::ServerFireProjectileWithVelocity_Implementation(FVector Origin,
	FVector_NetQuantizeNormal ShootVelocity)
{
	FTransform SpawnTM(ShootVelocity.GetSafeNormal().Rotation(), Origin);
	AShooterProjectile* Projectile = Cast<AShooterProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);
		//Projectile->MovementComp->MaxSpeed=PlasmaGrenadeSpeed;
		//Projectile->MovementComp->InitialSpeed=PlasmaGrenadeSpeed;
		Projectile->InitVelocity(ShootVelocity);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

bool AShooterWeapon_PlasmaGrenade::ServerFireProjectileWithVelocity_Validate(FVector Origin,
	FVector_NetQuantizeNormal ShootVelocity)
{
	return true;
}
