#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "SplineTrackerActor.h"
#include "SplineCharacterFollower.h"
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
    void ApplyGlobalSplineToAllSplines(ASplineCharacterFollower* SourceSplineCharacter);

private:
    // Static instance of the tool
    static USplineManagerTool* Instance;

    // Current selected global spline actor
    ASplineTrackerActor* CurrentGlobalSplineActor;
    ASplineCharacterFollower* CurrentGlobalSplineCharacter;

    // List of all tracked spline actors in the scene
    TArray<ASplineTrackerActor*> AllTrackedSplines;
    TArray<ASplineCharacterFollower*> AllTrackedCharacters;

    // Timer handle for periodic updates
    FTimerHandle UpdateTimerHandle;

    bool bHasModifications;
    template<typename T>
    void ApplyGlobalSpline(T* SourceSpline, TArray<T*>& TrackedSplines);
};
