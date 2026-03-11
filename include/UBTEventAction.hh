#pragma once
#include "G4UserEventAction.hh"
#include "UBTEventStore.hh"

// ============================================================================
//  UBTEventAction
//  Creates the G4AnalysisManager ntuple (once, at construction).
//  BeginOfEventAction clears the store; EndOfEventAction calls AddNtupleRow.
// ============================================================================
class UBTEventAction : public G4UserEventAction {
public:
    explicit UBTEventAction(UBTEventStore* store);
    ~UBTEventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction  (const G4Event*) override;

private:
    UBTEventStore* fStore = nullptr;
};
