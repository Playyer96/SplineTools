#include "SplineFollowerBase.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

ASplineFollowerBase::ASplineFollowerBase()
{
    bReplicates = true;
    bIsFollowing = true;

    MovementSpeed = 100.0f;
}

// Called when constructed in the editor or at runtime
void ASplineFollowerBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
}

// BeginPlay
void ASplineFollowerBase::BeginPlay()
{
    Super::BeginPlay();

    if (SplineComponent)
    {
        // Generate a random starting position along the spline
        const float SplineLength = SplineComponent->GetSplineLength();

        if (bFollowSplineAtRandomPosition) {
            CurrentSplinePosition = FMath::FRandRange(0.0f, SplineLength);
        }
        else {
            // Clamp the StartFollowingSplineAt variable to be between 0 and 1
            StartFollowingSplineAt = FMath::Clamp(StartFollowingSplineAt, 0.0f, 1.0f);

            // Calculate the position based on the clamped StartFollowingSplineAt
            CurrentSplinePosition = StartFollowingSplineAt * SplineLength;

        }
    }

    // Start following spline if enabled
    if (HasAuthority() && bStartFollowOnBeginPlay)
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
}

// Stop following the spline
void ASplineFollowerBase::StopFollowingSpline()
{
    bIsFollowing = false;
}

// Update position along the spline
void ASplineFollowerBase::UpdateSplinePosition(float DeltaTime)
{
    if (!SplineComponent) return;

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
}