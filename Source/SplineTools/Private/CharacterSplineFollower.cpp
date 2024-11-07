#include "CharacterSplineFollower.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"

ACharacterSplineFollower::ACharacterSplineFollower()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentSplinePosition = 0.0f;
    bIsFollowing = false;
    bReplicates = true;

    // Access the CharacterMovementComponent to set smoothing properties
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Linear;
        GetCharacterMovement()->bNetworkAlwaysReplicateTransformUpdateTimestamp = true;
        GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.5f;
        GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.5f;
    }

    // Initialize character mesh
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    CharacterMesh->SetupAttachment(RootComponent);
}

void ACharacterSplineFollower::BeginPlay()
{
    Super::BeginPlay();

    if (bStartFollowOnBeginPlay) {
        StartFollowingSpline();
    }
}

void ACharacterSplineFollower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFollowing && SplineComponent)
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
    CurrentSplinePosition += MovementSpeed * DeltaTime;

    // Wrap around if we exceed the spline length
    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    // Move the character along the spline
    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewLocation.Z = GetActorLocation().Z; // Keep the original Z position

    // Apply the corrected rotation with -90 degrees
    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    NewRotation.Yaw -= 90.0f;

    // Correct position with tolerance (if needed)
    if (FVector::Dist(NewLocation, GetActorLocation()) > Tolerance)
    {
        SetActorLocation(NewLocation);
        SetActorRotation(NewRotation);
    }
}

void ACharacterSplineFollower::PredictClientMovement(float DeltaTime)
{
    // Calculate the new predicted position
    CurrentSplinePosition += MovementSpeed * DeltaTime;

    // Wrap around if we exceed the spline length
    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    // Get predicted location and rotation along the spline
    FVector PredictedLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedLocation.Z = GetActorLocation().Z; // Maintain the original Z height

    FRotator PredictedRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    PredictedRotation.Yaw -= 90.0f;

    // Interpolate movement to the predicted location and rotation
    InterpolateMovement(PredictedLocation, PredictedRotation, DeltaTime);
}

void ACharacterSplineFollower::InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();
    // Increase interpolation speed for smoother movement
    FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpolationSpeed); // Adjusted interpolation speed

    // Keep the original Z height
    SmoothedLocation.Z = CurrentLocation.Z;

    // Smoothly interpolate the rotation using SLERP for better results
    FQuat CurrentRotationQuat = FQuat(GetActorRotation());
    FQuat TargetRotationQuat = FQuat(TargetRotation);
    FQuat SmoothedRotationQuat = FQuat::Slerp(CurrentRotationQuat, TargetRotationQuat, DeltaTime * InterpolationSpeed); // Adjusted interpolation speed

    SetActorLocation(SmoothedLocation);
    SetActorRotation(SmoothedRotationQuat.Rotator());
}

void ACharacterSplineFollower::OnRep_CurrentSplinePosition()
{
    // Update location when the spline position is replicated
    if (SplineComponent)
    {
        FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
        NewLocation.Z = GetActorLocation().Z; // Maintain the original Z height
        SetActorLocation(NewLocation);

        // Apply the corrected rotation with -90 degrees on replication
        FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
        NewRotation.Yaw -= 90.0f;

        SetActorRotation(NewRotation);
    }
}

void ACharacterSplineFollower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACharacterSplineFollower, CurrentSplinePosition);
    DOREPLIFETIME(ACharacterSplineFollower, bIsFollowing);
}