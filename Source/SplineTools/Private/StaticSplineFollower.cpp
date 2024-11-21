#include "StaticSplineFollower.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

AStaticSplineFollower::AStaticSplineFollower()
{
    // Set replication flag for networked game
    bReplicates = true;

    // Create the Static Mesh component
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetComponentTickEnabled(true);
    StaticMeshComponent->SetupAttachment(RootComponent);

    // Configure Static Mesh collision
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // Example: ignore visibility channel
    StaticMeshComponent->SetGenerateOverlapEvents(false); // Disable overlap events if unnecessary

    // Optional: Set physics properties if needed
    StaticMeshComponent->SetSimulatePhysics(false); // Disable physics simulation by default
}

// Called when constructed in the editor or at runtime
void AStaticSplineFollower::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    ActiveMesh = Cast<UMeshComponent>(StaticMeshComponent);
}

// BeginPlay
void AStaticSplineFollower::BeginPlay()
{
    Super::BeginPlay();

    ActiveMesh = Cast<UMeshComponent>(StaticMeshComponent);
}