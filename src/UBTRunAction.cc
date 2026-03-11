#include "UBTRunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"

UBTRunAction::UBTRunAction(const std::string& outFile)
    : fOutFileName(outFile)
{}

void UBTRunAction::BeginOfRunAction(const G4Run*)
{
    auto* man = G4AnalysisManager::Instance();
    man->SetDefaultFileType("root");
    // Strip ".root" suffix if present — G4AnalysisManager adds it itself
    std::string base = fOutFileName;
    if (base.size() > 5 && base.substr(base.size()-5) == ".root")
        base = base.substr(0, base.size()-5);
    man->OpenFile(base);
}

void UBTRunAction::EndOfRunAction(const G4Run*)
{
    auto* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}
