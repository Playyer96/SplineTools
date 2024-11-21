#include "SkeletalSplineFollower.h"
#include "Components/SplineComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ASkeletalSplineFollower::ASkeletalSplineFollower()
{
    bReplicates = true;

    // Create the Skeletal Mesh component
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SkeletalMeshComponent->SetupAttachment(RootComponent); // Attach to RootComponent after registration

    // Create the CapsuleComponent as a child of SkeletalMeshComponent
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    CapsuleComponent->SetSimulatePhysics(false);
    CapsuleComponent->SetCanEverAffectNavigation(false);
    CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);  // Radius = 42.0f, HalfHeight = 96.0f (similar to default character capsule)

    // Set CapsuleComponent location relative to SkeletalMeshComponent
    CapsuleComponent->SetRelativeLocation(FVector(0.f, 0.f, 84.f));

    // Attach the CapsuleComponent after registration
    CapsuleComponent->SetupAttachment(SkeletalMeshComponent);  // Attach to SkeletalMeshComponent after it's registered
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