// © 2014 - 2017 Soverance Studios
// http://www.soverance.com

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "EndlessReachHD.h"
#include "Pickups/Orb.h"
#include "Pickups/FuelCell.h"
#include "Pickups/Laser.h"
#include "Environment/Asteroid.h"

// Sets default values
AAsteroid::AAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Creates a scene component and sets it as the root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Asteroid Materials	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> GreyInst(TEXT("/Game/Environment/Materials/Rock/M_Asteroid_Inst.M_Asteroid_Inst"));
	RockColor = GreyInst.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> RedInst(TEXT("/Game/Environment/Materials/Rock/M_AsteroidHit_Inst.M_AsteroidHit_Inst"));
	RedColor = RedInst.Object;

	// Asteroid
	Asteroid = CreateDefaultSubobject<UDestructibleComponent>(TEXT("AsteroidBody"));
	Asteroid->SetupAttachment(RootComponent);
	Asteroid->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Asteroid->SetSimulatePhysics(true);
	Asteroid->BodyInstance.bLockZTranslation = true;
	//static ConstructorHelpers::FObjectFinder<UDestructibleMesh> AsteroidMeshObject(TEXT("/Game/Environment/Meshes/Asteroids/SM_Asteroid_DM.SM_Asteroid_DM"));
	//DM_Asteroid = AsteroidMeshObject.Object;
	////Asteroid->SetMaterial(0, RockColor);  // outside color
	////Asteroid->SetMaterial(1, RedColor);  // inside color

	// Explosion Visual Effect
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionParticleObject(TEXT("/Game/Particles/Emitter/P_Explosion.P_Explosion"));
	P_ExplosionFX = ExplosionParticleObject.Object;
	ExplosionFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionFX"));
	ExplosionFX->SetupAttachment(RootComponent);
	ExplosionFX->Template = P_ExplosionFX;
	ExplosionFX->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
	ExplosionFX->bAutoActivate = false;

	static ConstructorHelpers::FObjectFinder<USoundCue> AsteroidExplosionAudio(TEXT("/Game/Audio/Environment/Asteroid_Explosion_Cue.Asteroid_Explosion_Cue"));
	S_AsteroidExplosion = AsteroidExplosionAudio.Object;
	AsteroidExplosionSound = CreateDefaultSubobject<UAudioComponent>(TEXT("AsteroidExplosionSound"));
	AsteroidExplosionSound->SetupAttachment(RootComponent);
	AsteroidExplosionSound->Sound = S_AsteroidExplosion;
	AsteroidExplosionSound->bAutoActivate = false;

	// Asteroid Settings
	HitCount = 0;
	bWasDestroyed = false;
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();
	OnHitAsteroid.AddDynamic(this, &AAsteroid::HitAsteroid);  // bind asteroid hit function
	OnDestroyAsteroid.AddDynamic(this, &AAsteroid::DestroyAsteroid);  // bind asteroid hit function

	// add the destructible rock after a short delay... thanks 4.18 for moving apex destructibles into a plugin so they load super slow now
	FTimerHandle DestructibleAddTimer;
	GetWorldTimerManager().SetTimer(DestructibleAddTimer, this, &AAsteroid::AddDestructible, 5.0f, false);
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the asteroid is hit with player bullets
void AAsteroid::AddDestructible()
{
	//DM_Asteroid = LoadObject<UDestructibleMesh>(GetOuter(), TEXT("/Game/Environment/Meshes/Asteroids/SM_Asteroid_DM.SM_Asteroid_DM"), NULL, LOAD_None, NULL);
	DM_Asteroid = Cast<UDestructibleMesh>(StaticLoadObject(UDestructibleMesh::StaticClass(), NULL, TEXT("/Game/Environment/Meshes/Asteroids/SM_Asteroid_DM")));
	Asteroid->SetDestructibleMesh(DM_Asteroid);
}

// Called when the asteroid is hit with player bullets
void AAsteroid::HitAsteroid()
{
	Asteroid->SetMaterial(0, RedColor);  // outside color
	Asteroid->SetMaterial(1, RedColor);  // inside color

	if (HitCount < 5)  // if less than 5 hits
	{
		HitCount++;  // increment hit count

		if (HitCount >= 3)  // check if hit count equals 5
		{
			if (!bWasDestroyed)
			{
				DestroyAsteroid();  // if so, destroy this asteroid
			}
			
		}
	}
	else
	{
		DestroyAsteroid();  // additional error catch, probably unnecessary
	}
}

// Called when the asteroid is destroyed
void AAsteroid::DestroyAsteroid()
{
	if (!bWasDestroyed)
	{
		bWasDestroyed = true;
		Asteroid->SetMaterial(0, RockColor);  // outside color
		Asteroid->SetMaterial(1, RockColor);  // inside color		
		ExplosionFX->Activate();  // explosion fx
		AsteroidExplosionSound->Play();  // explosion sound fx
		Asteroid->ApplyRadiusDamage(1000, this->GetActorLocation(), 1000, 1000, true);  // break apart destructible mesh
		FActorSpawnParameters Params;
		Params.OverrideLevel = GetLevel();  // make pickups spawn within the streaming level so they can be properly unloaded

		//int32 DroppedOrbCount = FMath::RandRange(0, 9);  // drop a random amount of orbs
		int32 DroppedOrbCount = 250;  // drop a static amount of orbs
		const FTransform Settings = FTransform(GetActorRotation(), GetActorLocation(), FVector(1,1,1));  // cache transform settings

		for (int32 i = 0; i < DroppedOrbCount; i++)
		{
			AOrb* Orb = GetWorld()->SpawnActor<AOrb>(AOrb::StaticClass(), Settings, Params);
		}

		int32 FuelDropChance = FMath::RandRange(0, 9);  // get a random number to determine whether or not this rock will drop a fuel cell
		const FTransform FuelSettings = FTransform(GetActorRotation(), GetActorLocation(), FVector(0.25f, 0.25f, 0.25f));  // cache transform settings
		
		// drop fuel @ 50%
		if (FuelDropChance > 4)
		{
			AFuelCell* Fuel = GetWorld()->SpawnActor<AFuelCell>(AFuelCell::StaticClass(), FuelSettings, Params);  // spawn fuel
		}

		int32 LaserDropChance = FMath::RandRange(0, 9);  // get a random number to determine whether or not this rock will drop a fuel cell
		const FTransform LaserSettings = FTransform(GetActorRotation(), GetActorLocation(), FVector(0.25f, 0.25f, 0.25f));  // cache transform settings

		// drop fuel @ 20%
		if (FuelDropChance > 7)
		{
			ALaser* Laser = GetWorld()->SpawnActor<ALaser>(ALaser::StaticClass(), LaserSettings, Params);  // spawn fuel
		}
	}
	
}