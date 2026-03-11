#include "TrackerRunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"

TrackerRunAction::TrackerRunAction(const std::string& outFile)
    : fOutFileName(outFile)
{}

void TrackerRunAction::BeginOfRunAction(const G4Run*)
{
    auto* man = G4AnalysisManager::Instance();
    man->SetDefaultFileType("root");
    // Strip ".root" suffix if present — G4AnalysisManager adds it itself
    std::string base = fOutFileName;
    if (base.size() > 5 && base.substr(base.size()-5) == ".root")
        base = base.substr(0, base.size()-5);
    man->OpenFile(base);
}

void TrackerRunAction::EndOfRunAction(const G4Run*)
{
    auto* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}
