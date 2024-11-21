#include "SplineTrackerActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "SplineManagerTool.h"
#endif

ASplineTrackerActor::ASplineTrackerActor()
{
    PrimaryActorTick.bCanEverTick = true; // Enable tick for rotation

    // Create and set the root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent->bEditableWhenInherited = true;
    SetRootComponent(RootComponent);
    RootComponent->SetMobility(EComponentMobility::Movable); // Set mobility if needed

    // Create the spline component and attach it to the root component
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent); // Attach to root component

    // Initialize the last label color
    LastLabelColor = LabelColor;
}

void ASplineTrackerActor::BeginPlay()
{
    Super::BeginPlay();
}

void ASplineTrackerActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (HasAnyFlags(RF_Transactional))
    {
        SplineComponent->SetClosedLoop(bCloseLoopOverride);

#if WITH_EDITOR
        InitializeSplineManager();
#endif

        // Check if labels should be shown
        if (bShowPointLabels)
        {
            UpdateLabelPositions();
        }
        else
        {
            // Destroy existing labels if flag is disabled
            for (UTextRenderComponent* Label : PointLabels)
            {
                if (Label)
                {
                    Label->DestroyComponent();
                }
            }
            PointLabels.Empty();
        }
    }
}

#if WITH_EDITOR
void ASplineTrackerActor::InitializeSplineManager()
{
    USplineManagerTool* SplineManager = USplineManagerTool::GetInstance();
    if (SplineManager)
    {
        SplineManager->Initialize();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SplineManagerTool instance is not valid."));
    }
}
#endif

void ASplineTrackerActor::UpdateLabelPositions()
{
    // Clear any existing labels safely
    for (UTextRenderComponent* Label : PointLabels)
    {
        if (Label && Label->IsValidLowLevel())
        {
            Label->UnregisterComponent();
            Label->DestroyComponent();
        }
    }
    PointLabels.Empty();

    // Create new labels for each spline point
    int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector PointLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

        UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this, UTextRenderComponent::StaticClass());
        if (Label)
        {
            Label->RegisterComponent();
            Label->SetText(FText::FromString(FString::Printf(TEXT("%d"), i)));
            Label->SetTextRenderColor(LabelColor);
            Label->SetWorldSize(fontSize);
            Label->SetWorldLocation(PointLocation + FVector(0, 0, zOffset));
            Label->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
            PointLabels.Add(Label);
        }
    }
}

// Call this method when spline points are edited
void ASplineTrackerActor::OnSplinePointEdited()
{
    if (bShowPointLabels)
    {
        UpdateLabelPositions();
    }
}