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
#include "LevelNode.h"


// Sets default values
ALevelNode::ALevelNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates a scene component and sets it as the root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// configure Node Entry Radius
	NodeEntryRadius = CreateDefaultSubobject<USphereComponent>(TEXT("NodeEntryRadius"));
	NodeEntryRadius->SetupAttachment(RootComponent);
	NodeEntryRadius->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	NodeEntryRadius->SetSphereRadius(250);

	// Portal Visual Effect
	static ConstructorHelpers::FObjectFinder<UParticleSystem> NodePortalParticleObject(TEXT("/Game/VectorFields/Particles/P_Gateway.P_Gateway"));
	P_NodePortalFX = NodePortalParticleObject.Object;
	NodePortalFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("NodePortalFX"));
	NodePortalFX->SetupAttachment(RootComponent);
	NodePortalFX->Template = P_NodePortalFX;
	NodePortalFX->SetRelativeRotation(FRotator(0, 0, 0));
	NodePortalFX->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
	NodePortalFX->bAutoActivate = false;

	
}

// Called when the game starts or when spawned
void ALevelNode::BeginPlay()
{
	Super::BeginPlay();
	
	// if the level is unlocked, then activate the portal to show this node on the map
	if (!LevelConfig.Locked)
	{
		NodePortalFX->Activate();
	}
}

// Called every frame
void ALevelNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

