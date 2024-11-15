#include "SkeletalSplineFollower.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

ASkeletalSplineFollower::ASkeletalSplineFollower()
{
    // Set replication flag for networked game
    bReplicates = true;
    bUseSkeletalMesh = true;

    // Create the Skeletal Mesh component
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SkeletalMeshComponent->RegisterComponent();
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMeshComponent->SetComponentTickEnabled(true);
    SkeletalMeshComponent->SetupAttachment(RootComponent);

    // Create the CapsuleComponent as a child of SkeletalMeshComponent
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetupAttachment(SkeletalMeshComponent);
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    CapsuleComponent->SetSimulatePhysics(false);
    CapsuleComponent->SetCanEverAffectNavigation(false);

    // Set the default size of the capsule (matching a typical character capsule)
    CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);  // Radius = 42.0f, HalfHeight = 96.0f (similar to default character capsule)

    // Move the Capsule Component up by 84 units in the Z-axis
    CapsuleComponent->SetRelativeLocation(FVector(0.f, 0.f, 84.f));
}

// Called when constructed in the editor or at runtime
void ASkeletalSplineFollower::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    ActiveMesh = Cast<UMeshComponent>(SkeletalMeshComponent);
}

// BeginPlay
void ASkeletalSplineFollower::BeginPlay()
{
    Super::BeginPlay();

    ActiveMesh = Cast<UMeshComponent>(SkeletalMeshComponent);
}

// Tick
void ASkeletalSplineFollower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}