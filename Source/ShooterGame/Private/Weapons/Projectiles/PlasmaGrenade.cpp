// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "Weapons/Projectiles/PlasmaGrenade.h"
#include "Components/CapsuleComponent.h"
#include "ShooterExplosionEffect.h"
#include "NiagaraComponent.h"

#include "ShooterWeapon_PlasmaGrenade.h"
#include "WidgetComponent.h"

APlasmaGrenade::APlasmaGrenade()
{
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PlayerDetector=CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetector"));
	PlayerDetector->SetupAttachment(RootComponent);
	PlayerDetector->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlayerDetector->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	PlayerDetector->SetSphereRadius(120);

	WidgetComponent= CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponet"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetOnlyOwnerSee(true);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetIsReplicated(false);
	WidgetComponent->SetVisibility(false);

	MovementComp->bShouldBounce=true;
	MovementComp->MaxSpeed=0;
	MovementComp->UpdatedComponent = CollisionComp;
	PlasmaGrenadeState=FPlasmaGrenadeGTStates.Launched;

	NiagaraComponent=CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraSystem"));
	NiagaraComponent->SetupAttachment(RootComponent);

	TimelineComponent=CreateDefaultSubobject<UTimelineComponent>(TEXT("ActivationTL")); 
	TimelineComponent->SetTimelineLength(1.0f);
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);
	SetActorTickEnabled(false);
}




void APlasmaGrenade::PostInitializeComponents()
{
	Super::PostInitializeComponents();


	CollisionComp->MoveIgnoreActors.Add(GetInstigator());
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this,&APlasmaGrenade::OnOverlapBegin);

	PlayerDetector->OnComponentBeginOverlap.AddDynamic(this,&APlasmaGrenade::OnPlayerDetectorOverlapBegin);
	PlayerDetector->OnComponentEndOverlap.AddDynamic(this,&APlasmaGrenade::OnPlayerDetectorOverlapEnd);

	 UCurveFloat* CurveFloat= NewObject<UCurveFloat>() ;
	CurveFloat->FloatCurve.AddKey(0,0);
	CurveFloat->FloatCurve.AddKey(1,1);
 
	OnTimelineFloat.BindUFunction(this,TEXT("OnFuseUpdate")); 
	OnTimeLineFinish.BindUFunction(this,TEXT("OnFuseFinished"));
	
	
	AShooterWeapon_PlasmaGrenade* OwnerWeapon = Cast<AShooterWeapon_PlasmaGrenade>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}

	TimelineComponent->AddInterpFloat(CurveFloat ,OnTimelineFloat);
	TimelineComponent->SetTimelineFinishedFunc(OnTimeLineFinish);
	if(WeaponConfig.ProjectileLife>0)
	TimelineComponent->SetPlayRate(1/WeaponConfig.ProjectileLife);
	MyController = GetInstigatorController();
	SetOwner(GetInstigator());
	SetLifeSpan(0);

	
	
}

void APlasmaGrenade::BeginPlay()
{
	Super::BeginPlay();
	WidgetComponent->SetVisibility(false);
 	if(HasAuthority())	
		CreateMessageInteractionWidget();

	
}

 

void APlasmaGrenade::CreateMessageInteractionWidget_Implementation()
{ 
	if(PickUpMessageClass==nullptr)return;

	WidgetComponent->SetWidgetClass(PickUpMessageClass);
 
}
 

void APlasmaGrenade::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		
		/*On success stick the bomb stays with the target, instead bounces around*/
		 TryToStickToTarget(Hit);
	   
	}
}

void APlasmaGrenade::OnPlayerDetectorOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	 if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		if(OtherActor!=GetInstigator())return;
	 	if(!PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Pickable))return;
	 	bPlayerIsInsideArea=true;

	 	//*Single player must show*//
	 	OnRep_InsideArea();
	}
  
}

void APlasmaGrenade::OnPlayerDetectorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex )
{
	 
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		if(OtherActor!=GetInstigator())return;
		bPlayerIsInsideArea=false;
		//*Single player must hide*//
		OnRep_InsideArea();
	}
}


void APlasmaGrenade::OnImpact(const FHitResult& HitResult)
{
	if (HasAuthority() && !bExploded)
	{

		PlasmaGrenadeState=FPlasmaGrenadeGTStates.Pickable;

		SetActorRotation(FRotator::ZeroRotator);
		BeginDetonationCountDown();
	}

}

void APlasmaGrenade::BeginDetonationCountDown()
{
	if (GetLocalRole() == ROLE_Authority && !bExploded && GetWorld())
	{
		 
		TimelineComponent->PlayFromStart();
		OnBeginDetonationCountDown();
		if(ActivationSoundCue)
		UGameplayStatics::SpawnSoundAtLocation(this,ActivationSoundCue,GetActorLocation());
	
		if(PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Stuck))
		PlasmaGrenadeState=FPlasmaGrenadeGTStates.StuckIgnited;

		if(PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Pickable))
			PlasmaGrenadeState=FPlasmaGrenadeGTStates.PickableIgnited;
		
	}
}

void APlasmaGrenade::PlasmaDetonation()
{
	if(PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Recovered))return; 
	PlasmaGrenadeState=FPlasmaGrenadeGTStates.Explode;
	
	
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	
	WidgetComponent->SetVisibility(false);
	const FVector ExplosionOrigin = GetActorLocation();

	
	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, ExplosionOrigin, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}
	//if Singleplayer this should show the explosion
	OnRep_GameplayTagChanged();
	SetLifeSpan(2);
}

void APlasmaGrenade::ShootWithVelocity_Implementation(FVector ShootVelocity)
{
	MovementComp->Velocity=ShootVelocity;
}

 

void APlasmaGrenade::ShootTo(const FVector ShootDirection)
{
	MovementComp->Velocity=ShootDirection*MovementComp->InitialSpeed;
	
}


bool APlasmaGrenade::TryToPickUp(AShooterCharacter* PickInstigator)
{
	if(!PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Pickable))return false;
	
	if( GetInstigator()!=PickInstigator)return false;
	if(GetWorld())
 		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	PickInstigator->ResolveInteraction(FGameplayTag::RequestGameplayTag(FName("Weapons.PlasmaBomb")));
	PlasmaGrenadeState=FPlasmaGrenadeGTStates.Recovered;

	 
	
	Destroy();
	return true;
}

void APlasmaGrenade::OnFuseUpdate(float T)
{
	FuseProgress=T;
	//SP
	OnRep_FuseProgress();
}

void APlasmaGrenade::OnFuseFinished()  
{
	PlasmaDetonation();
}


void APlasmaGrenade::TryToStickToTarget_Implementation(const FHitResult& HitResult)
{
	if(PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Stuck))return;
 
	if(HitResult.GetActor()==nullptr)return  ;
	AShooterCharacter* ShooterCharacter=Cast<AShooterCharacter>(HitResult.GetActor());
	if(ShooterCharacter==nullptr)return  ;
	if(ShooterCharacter==GetInstigator())return  ;
 
	PlasmaGrenadeState=	FPlasmaGrenadeGTStates.Stuck;
	 
	CollisionComp->MoveIgnoreActors.Add(ShooterCharacter);
	ShooterCharacter->GetCapsuleComponent()->MoveIgnoreActors.Add(this);
	
	

	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComp->bShouldBounce=false;
	MovementComp->ProjectileGravityScale=0;
	MovementComp->StopMovementImmediately(); 
	MovementComp->SetComponentTickEnabled(false);
	FVector  AttachLocation;
	ShooterCharacter->GetPawnMesh()->FindClosestBone_K2(GetActorLocation(),AttachLocation);
	AttachLocation=AttachLocation.SizeSquared()<=0.1f?GetActorLocation():AttachLocation;
	
	AttachToComponent(ShooterCharacter->GetCapsuleComponent(),FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorLocation(AttachLocation);
	 BeginDetonationCountDown();
}

 
void APlasmaGrenade::OnRep_GameplayTagChanged()
{
	if(PlasmaGrenadeState.MatchesTag(FPlasmaGrenadeGTStates.Explode))
	{
		if(NiagaraComponent!=nullptr)
		{ 
			NiagaraComponent->DeactivateImmediate();
			NiagaraComponent->SetVisibility(false);
		}
		
		if (ExplosionTemplate)
		{ 
			FTransform const SpawnTransform(  GetActorLocation());
			AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);
			if (EffectActor)
			{ 
				UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
			}
		} 
	}

	
	
} 

void APlasmaGrenade::OnRep_InsideArea()
{
	WidgetComponent->SetVisibility(bPlayerIsInsideArea);
	
}

void APlasmaGrenade::OnRep_FuseProgress()
{
	if(NiagaraComponent)
	{
		NiagaraComponent->SetFloatParameter(TEXT("FuseProgress"),FuseProgress);
		
	}
}
void APlasmaGrenade::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME_CONDITION( APlasmaGrenade, bPlayerIsInsideArea, COND_OwnerOnly );
	DOREPLIFETIME( APlasmaGrenade, PlasmaGrenadeState   );
	DOREPLIFETIME( APlasmaGrenade, FuseProgress   );
}

 
 