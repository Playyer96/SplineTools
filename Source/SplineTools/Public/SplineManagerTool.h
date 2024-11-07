#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "SplineTrackerActor.h"
#include "SplineManagerTool.generated.h"

class ASplineTrackerActor;

UCLASS()
class SPLINETOOLS_API USplineManagerTool : public UObject
{
    GENERATED_BODY()

public:
    USplineManagerTool();
    ~USplineManagerTool();

    static USplineManagerTool* GetInstance();

    void Initialize();
    void OnLevelChanged();
    void OnLevelChanged(bool bIsSimulating);
    void OnLevelChanged(const FString& MapName, bool bAsTemplate);
    void Shutdown();

    // Handle selection changes in the editor
    void OnSelectionChanged(UObject* NewSelection);
    void CheckForSplineUpdates();
    void ApplyGlobalSplineToAllSplines(ASplineTrackerActor* SourceSplineActor);

private:
    // Static instance of the tool
    static USplineManagerTool* Instance;

    // Current selected global spline actor
    ASplineTrackerActor* CurrentGlobalSplineActor;

    // List of all tracked spline actors in the scene
    TArray<ASplineTrackerActor*> AllTrackedSplines;

    // Timer handle for periodic updates
    FTimerHandle UpdateTimerHandle;

    bool bHasModifications;
};
