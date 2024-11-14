#include "CharacterSplineFollower.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

// ACharacterSplineFollower class
ACharacterSplineFollower::ACharacterSplineFollower()
{
    // Initialize RootComponent
    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    }

    // Initialize Mesh Component
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    CharacterMesh->SetupAttachment(RootComponent);  // Attach mesh to root component

    // Set default mesh or set it from the editor
    if (CharacterMesh)
    {
        // Optionally, assign a mesh here
        // CharacterMesh->SetSkeletalMesh(YourMeshAsset);
    }

    // Set up the default collision component
    SetupCollisionComponent();
}

void ACharacterSplineFollower::SetupCollisionComponent()
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

    // Attach the collision component to the mesh
    if (CollisionComponent)
    {
        CollisionComponent->SetupAttachment(CharacterMesh); // Attach collision to mesh
        CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        //CollisionComponent->SetSimulatePhysics(true); // Enable physics simulation for collision

        // Update the size of the collision component based on the mesh size
        UpdateCollisionSize();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CollisionComponent failed to initialize properly."));
    }
}

void ACharacterSplineFollower::UpdateCollisionSize()
{
    if (!CharacterMesh || !CollisionComponent) return;

    // Get mesh bounds and adjust the collision component size accordingly
    FVector MeshSize = CharacterMesh->GetOwner()->GetComponentsBoundingBox().GetSize();

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

void ACharacterSplineFollower::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Re-create the collision component when the type is changed in editor
    //SetupCollisionComponent();
}

void ACharacterSplineFollower::BeginPlay()
{
    Super::BeginPlay();

    // Ensure character mesh is valid
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterMesh is null in BeginPlay."));
        return;
    }

    // Start spline following if enabled
    if (bStartFollowOnBeginPlay)
    {
        StartFollowingSpline();
    }
}

void ACharacterSplineFollower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFollowing && CharacterMesh)
    {
        if (HasAuthority())
        {
            // Update the position on the server
            UpdateSplinePosition(DeltaTime);
        }
        else
        {
            // Predict movement on the client
            PredictClientMovement(DeltaTime);
        }
    }
}

void ACharacterSplineFollower::StartFollowingSpline()
{
    bIsFollowing = true;
    CurrentSplinePosition = 0.0f;
}

void ACharacterSplineFollower::StopFollowingSpline()
{
    bIsFollowing = false;
}

void ACharacterSplineFollower::SetSplineComponent(USplineComponent* Spline)
{
    SplineComponent = Spline;
}

void ACharacterSplineFollower::UpdateSplinePosition(float DeltaTime)
{
    if (!SplineComponent || !CharacterMesh) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;

    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewLocation.Z = CharacterMesh->GetComponentLocation().Z;

    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewRotation.Yaw -= 90.0f;

    if (FVector::Dist(NewLocation, CharacterMesh->GetComponentLocation()) > Tolerance)
    {
        CharacterMesh->SetWorldLocation(NewLocation);
        CharacterMesh->SetWorldRotation(NewRotation);
    }
}


void ACharacterSplineFollower::PredictClientMovement(float DeltaTime)
{
    if (!SplineComponent || !CharacterMesh) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;

    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector PredictedLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedLocation.Z = CharacterMesh->GetComponentLocation().Z;

    FRotator PredictedRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedRotation.Yaw -= 90.0f;

    InterpolateMovement(PredictedLocation, PredictedRotation, DeltaTime);
}

void ACharacterSplineFollower::InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime)
{
    if (!CharacterMesh) return;

    FVector CurrentLocation = CharacterMesh->GetComponentLocation();
    FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpolationSpeed);
    SmoothedLocation.Z = CurrentLocation.Z;

    FQuat CurrentRotationQuat = FQuat(CharacterMesh->GetComponentRotation());
    FQuat TargetRotationQuat = FQuat(TargetRotation);
    FQuat SmoothedRotationQuat = FQuat::Slerp(CurrentRotationQuat, TargetRotationQuat, DeltaTime * InterpolationSpeed);

    CharacterMesh->SetWorldLocation(SmoothedLocation);
    CharacterMesh->SetWorldRotation(SmoothedRotationQuat.Rotator());
}

void ACharacterSplineFollower::OnRep_CurrentSplinePosition()
{
    if (!SplineComponent || !CharacterMesh) return;

    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewLocation.Z = CharacterMesh->GetComponentLocation().Z;
    CharacterMesh->SetWorldLocation(NewLocation);

    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewRotation.Yaw -= 90.0f;
    CharacterMesh->SetWorldRotation(NewRotation);
}

void ACharacterSplineFollower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACharacterSplineFollower, CurrentSplinePosition);
    DOREPLIFETIME(ACharacterSplineFollower, bIsFollowing);
}