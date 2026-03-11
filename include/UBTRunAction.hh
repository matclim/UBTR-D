#pragma once
#include "G4UserRunAction.hh"
#include "UBTEventStore.hh"
#include <string>

// ============================================================================
//  UBTRunAction
//  Opens/closes the output file via G4AnalysisManager (writes ROOT by default).
//  The ntuple is created once at construction of UBTEventAction.
// ============================================================================
class UBTRunAction : public G4UserRunAction {
public:
    explicit UBTRunAction(const std::string& outFile = "ubt_hits.root");
    ~UBTRunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*)   override;

private:
    std::string fOutFileName;
};
