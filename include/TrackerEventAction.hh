#pragma once
#include "G4UserEventAction.hh"
#include "TrackerEventStore.hh"

// ============================================================================
//  TrackerEventAction
//  Creates the G4AnalysisManager ntuple (once, at construction).
//  BeginOfEventAction clears the store; EndOfEventAction calls AddNtupleRow.
// ============================================================================
class TrackerEventAction : public G4UserEventAction {
public:
    explicit TrackerEventAction(TrackerEventStore* store);
    ~TrackerEventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction  (const G4Event*) override;

private:
    TrackerEventStore* fStore = nullptr;
};
