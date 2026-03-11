#include "TrackerEventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"

TrackerEventAction::TrackerEventAction(TrackerEventStore* store)
    : fStore(store)
{
    auto* man = G4AnalysisManager::Instance();

    man->CreateNtuple("Events", "Tracker hits — one row per event");

    // ---- tube branches ------------------------------------------------------
    man->CreateNtupleIColumn("tube_trackID",  fStore->tube_trackID);
    man->CreateNtupleDColumn("tube_edep",     fStore->tube_edep);
    man->CreateNtupleDColumn("tube_x",        fStore->tube_x);
    man->CreateNtupleDColumn("tube_y",        fStore->tube_y);
    man->CreateNtupleDColumn("tube_z",        fStore->tube_z);
    man->CreateNtupleDColumn("tube_x_entry",  fStore->tube_x_entry);
    man->CreateNtupleDColumn("tube_y_entry",  fStore->tube_y_entry);
    man->CreateNtupleDColumn("tube_z_entry",  fStore->tube_z_entry);
    man->CreateNtupleDColumn("tube_x_exit",   fStore->tube_x_exit);
    man->CreateNtupleDColumn("tube_y_exit",   fStore->tube_y_exit);
    man->CreateNtupleDColumn("tube_z_exit",   fStore->tube_z_exit);
    man->CreateNtupleIColumn("tube_region",   fStore->tube_region);

    // ---- tile branches ------------------------------------------------------
    man->CreateNtupleIColumn("tile_trackID",  fStore->tile_trackID);
    man->CreateNtupleDColumn("tile_edep",     fStore->tile_edep);
    man->CreateNtupleDColumn("tile_x",        fStore->tile_x);
    man->CreateNtupleDColumn("tile_y",        fStore->tile_y);
    man->CreateNtupleDColumn("tile_z",        fStore->tile_z);
    man->CreateNtupleIColumn("tile_tileX",    fStore->tile_tileX);
    man->CreateNtupleIColumn("tile_tileY",    fStore->tile_tileY);
    man->CreateNtupleIColumn("tile_sector",   fStore->tile_sector);

    man->FinishNtuple();
}

void TrackerEventAction::BeginOfEventAction(const G4Event* event)
{
    fStore->clear();
    G4cout << "\n=== Tracker Event " << event->GetEventID() << " ===" << G4endl;
}

void TrackerEventAction::EndOfEventAction(const G4Event*)
{
    G4AnalysisManager::Instance()->AddNtupleRow();
}
