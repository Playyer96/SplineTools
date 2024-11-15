#include "SplineFollowerBase.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

ASplineFollowerBase::ASplineFollowerBase()
{
    //RootComponent->RegisterComponent();
    bReplicates = true;
    bUseSkeletalMesh = true;
    bIsFollowing = true;
    //SkeletalMeshAsset = nullptr;
    //StaticMeshAsset = nullptr;
    //if (StaticMeshAsset)
    //{
    //    StaticMeshComponent->SetStaticMesh(StaticMeshAsset);
    //}

    //if (SkeletalMeshAsset)
    //{
    //    SkeletalMeshComponent->SetSkeletalMesh(SkeletalMeshAsset);
    //}

    //StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    //SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));

    MovementSpeed = 100.0f;
}

// Called when constructed in the editor or at runtime
void ASplineFollowerBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ToggleMesh(bUseSkeletalMesh);
}

// BeginPlay
void ASplineFollowerBase::BeginPlay()
{
    Super::BeginPlay();

    ToggleMesh(bUseSkeletalMesh);


    // Start following spline if enabled
    if (bStartFollowOnBeginPlay)
    {
        StartFollowingSpline();
    }
}

void ASplineFollowerBase::SetSplineComponent(USplineComponent* NewSplineComponent)
{
    if (NewSplineComponent)
    {
        SplineComponent = NewSplineComponent;
    }
}

void ASplineFollowerBase::SetupCollisionComponent()
{
    // Ensure the RootComponent is valid
    if (!RootComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("RootComponent is not valid. Cannot initialize CollisionComponent."));
        return;
    }

    // Check if a collision component already exists, if not, create one
    if (!CollisionComponent)
    {
        // Create collision component based on the selected collision type
        switch (CollisionType)
        {
        case ECollisionType::Box:
            CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
            break;
        case ECollisionType::Capsule:
            CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
            break;
        case ECollisionType::Sphere:
            CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
            break;
        default:
            CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
            break;
        }
    }

    // Attach the collision component to the currently active mesh
    if (CollisionComponent)
    {
        //UMeshComponent* ActiveMesh = bUseSkeletalMesh ? Cast<UMeshComponent>(SkeletalMeshComponent) : Cast<UMeshComponent>(StaticMeshComponent);

        if (ActiveMesh)
        {
            CollisionComponent->SetupAttachment(ActiveMesh); // Attach collision to the active mesh
            CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
            CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        }

        // Update the size of the collision component based on the mesh size
        UpdateCollisionSize();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionComponent failed to initialize properly."));
    }
}

// Toggle active mesh
void ASplineFollowerBase::ToggleMesh(bool bUseSkeletal)
{
    //bUseSkeletalMesh = bUseSkeletal;

    //if (bUseSkeletalMesh)
    //{
    //    if (!SkeletalMeshComponent)
    //    {
    //        SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass());
    //        SkeletalMeshComponent->RegisterComponent();
    //    }

    //    SkeletalMeshComponent->SetSkeletalMesh(SkeletalMeshAsset);
    //    SkeletalMeshComponent->SetVisibility(true);
    //    SkeletalMeshComponent->SetComponentTickEnabled(true);
    //    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    //    // Attach to root component
    //    SkeletalMeshComponent->SetupAttachment(RootComponent);

    //    // Destroy StaticMesh if SkeletalMesh is being used
    //    if (StaticMeshComponent)
    //    {
    //        StaticMeshComponent->DestroyComponent();
    //        StaticMeshComponent = nullptr;
    //    }
    //}
    //else
    //{
    //    if (StaticMeshAsset)
    //    {
    //        if (!StaticMeshComponent)
    //        {
    //            StaticMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
    //            StaticMeshComponent->RegisterComponent();
    //        }

    //        StaticMeshComponent->SetStaticMesh(StaticMeshAsset);
    //        StaticMeshComponent->SetVisibility(true);
    //        StaticMeshComponent->SetComponentTickEnabled(true);
    //        StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    //        // Attach to root component
    //        StaticMeshComponent->SetupAttachment(RootComponent);

    //        // Destroy SkeletalMesh if StaticMesh is being used
    //        if (SkeletalMeshComponent)
    //        {
    //            SkeletalMeshComponent->DestroyComponent();
    //            SkeletalMeshComponent = nullptr;
    //        }
    //    }
    //}
}

// Tick
void ASplineFollowerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFollowing)
    {
        if (HasAuthority())
        {
            UpdateSplinePosition(DeltaTime);
        }
        else
        {
            PredictClientMovement(DeltaTime);
        }
    }
}

// Start following the spline
void ASplineFollowerBase::StartFollowingSpline()
{
    bIsFollowing = true;
    CurrentSplinePosition = 0.0f;
}

// Stop following the spline
void ASplineFollowerBase::StopFollowingSpline()
{
    bIsFollowing = false;
}

// Update position along the spline
void ASplineFollowerBase::UpdateCollisionSize()
{
    // Get the currently active mesh
    //UMeshComponent* ActiveMesh = bUseSkeletalMesh ? Cast<UMeshComponent>(SkeletalMeshComponent) : Cast<UMeshComponent>(StaticMeshComponent);

    if (!ActiveMesh || !CollisionComponent) return;

    // Get mesh bounds and adjust the collision component size accordingly
    FVector MeshSize = ActiveMesh->Bounds.BoxExtent * 2.0f;

    if (UBoxComponent* BoxComp = Cast<UBoxComponent>(CollisionComponent))
    {
        BoxComp->SetBoxExtent(MeshSize * 0.5f); // Half of the size for correct fitting
    }
    else if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(CollisionComponent))
    {
        CapsuleComp->SetCapsuleSize(MeshSize.X * 0.5f, MeshSize.Z * 0.5f); // Adjust for width and height
    }
    else if (USphereComponent* SphereComp = Cast<USphereComponent>(CollisionComponent))
    {
        SphereComp->SetSphereRadius(MeshSize.Size() * 0.5f); // Average size for a sphere
    }
}

// Update position along the spline
void ASplineFollowerBase::UpdateSplinePosition(float DeltaTime)
{
    if (!SplineComponent) return;

    // Get the active mesh
    //UMeshComponent* ActiveMesh = bUseSkeletalMesh ? Cast<UMeshComponent>(SkeletalMeshComponent) : Cast<UMeshComponent>(StaticMeshComponent);

    if (!ActiveMesh) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;

    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewLocation.Z = ActiveMesh->GetComponentLocation().Z;

    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewRotation.Yaw -= 90.0f;

    if (FVector::Dist(NewLocation, ActiveMesh->GetComponentLocation()) > Tolerance)
    {
        ActiveMesh->SetWorldLocation(NewLocation);
        ActiveMesh->SetWorldRotation(NewRotation);
    }
}

// Predict client movement
void ASplineFollowerBase::PredictClientMovement(float DeltaTime)
{
    if (!SplineComponent) return;

    //UMeshComponent* ActiveMesh = bUseSkeletalMesh ? Cast<UMeshComponent>(SkeletalMeshComponent) : Cast<UMeshComponent>(StaticMeshComponent);
    if (!ActiveMesh) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;

    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector PredictedLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedLocation.Z = ActiveMesh->GetComponentLocation().Z;

    FRotator PredictedRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedRotation.Yaw -= 90.0f;

    InterpolateMovement(PredictedLocation, PredictedRotation, DeltaTime);
}

// Interpolate movement
void ASplineFollowerBase::InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime)
{
    //UMeshComponent* ActiveMesh = bUseSkeletalMesh ? Cast<UMeshComponent>(SkeletalMeshComponent) : Cast<UMeshComponent>(StaticMeshComponent);
    if (!ActiveMesh) return;

    FVector CurrentLocation = ActiveMesh->GetComponentLocation();
    FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpolationSpeed);
    SmoothedLocation.Z = CurrentLocation.Z;

    FQuat CurrentRotationQuat = FQuat(ActiveMesh->GetComponentRotation());
    FQuat TargetRotationQuat = FQuat(TargetRotation);
    FQuat SmoothedRotationQuat = FQuat::Slerp(CurrentRotationQuat, TargetRotationQuat, DeltaTime * InterpolationSpeed);

    ActiveMesh->SetWorldLocation(SmoothedLocation);
    ActiveMesh->SetWorldRotation(SmoothedRotationQuat.Rotator());
}

// Replication
void ASplineFollowerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASplineFollowerBase, CurrentSplinePosition);
    DOREPLIFETIME(ASplineFollowerBase, bIsFollowing);
    DOREPLIFETIME(ASplineFollowerBase, bUseSkeletalMesh);
}

void ASplineFollowerBase::OnRep_CurrentSplinePosition()
{
    if (!SplineComponent) return;

    // Get the new location along the spline
    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);

    // Preserve the current Z location to avoid abrupt height changes
    FVector CurrentMeshLocation = ActiveMesh->GetComponentLocation();
    NewLocation.Z = CurrentMeshLocation.Z;

    // Get the new rotation along the spline
    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);

    // Apply an adjustment to the yaw to align the mesh correctly
    NewRotation.Yaw -= 90.0f;

    // Update the position and rotation of the selected mesh
    ActiveMesh->SetWorldLocation(NewLocation);
    ActiveMesh->SetWorldRotation(NewRotation);
    //if (bUseSkeletalMesh && SkeletalMeshComponent)
    //{
    //}
    //else if (StaticMeshComponent)
    //{
    //    StaticMeshComponent->SetWorldLocation(NewLocation);
    //    StaticMeshComponent->SetWorldRotation(NewRotation);
    //}
}