#pragma once
#include "G4UserRunAction.hh"
#include "TrackerEventStore.hh"
#include <string>

// ============================================================================
//  TrackerRunAction
//  Opens/closes the output file via G4AnalysisManager (writes ROOT by default).
//  The ntuple is created once at construction of TrackerEventAction.
// ============================================================================
class TrackerRunAction : public G4UserRunAction {
public:
    explicit TrackerRunAction(const std::string& outFile = "tracker_hits.root");
    ~TrackerRunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*)   override;

private:
    std::string fOutFileName;
};
