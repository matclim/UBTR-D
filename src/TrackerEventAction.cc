#include "TrackerEventAction.hh"
#include "TrackerRunAction.hh"
#include "TrackerHit.hh"
#include "TrackerSD.hh"

#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include "TTree.h"

TrackerEventAction::TrackerEventAction(TrackerRunAction* ra)
    : fRunAction(ra)
{}

void TrackerEventAction::BeginOfEventAction(const G4Event*)
{
    fRunAction->ClearVectors();
}

void TrackerEventAction::EndOfEventAction(const G4Event* event)
{
    // Resolve collection IDs once (cached after first event)
    auto* sdm = G4SDManager::GetSDMpointer();
    if (fTubeHCID < 0)
        fTubeHCID = sdm->GetCollectionID(
            std::string("TrackerSD/") + TrackerSD::TubeHCname());
    if (fTileHCID < 0)
        fTileHCID = sdm->GetCollectionID(
            std::string("TrackerSD/") + TrackerSD::TileHCname());

    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce) return;

    // ------------------------------------------------------------------
    //  Push tube hits into vectors
    // ------------------------------------------------------------------
    if (fTubeHCID >= 0) {
        auto* tubeHC = dynamic_cast<TubeHitsCollection*>(hce->GetHC(fTubeHCID));
        if (tubeHC) {
            const int n = static_cast<int>(tubeHC->GetSize());
            for (int i = 0; i < n; ++i) {
                TubeHit* h = static_cast<TubeHit*>(tubeHC->GetHit(i));

                fRunAction->tube_trackID .push_back(h->GetTrackID());
                fRunAction->tube_edep    .push_back(h->GetEdep()         / MeV);
                fRunAction->tube_x       .push_back(h->GetPosition().x() / mm);
                fRunAction->tube_y       .push_back(h->GetPosition().y() / mm);
                fRunAction->tube_z       .push_back(h->GetPosition().z() / mm);
                fRunAction->tube_x_entry .push_back(h->GetEntry().x()    / mm);
                fRunAction->tube_y_entry .push_back(h->GetEntry().y()    / mm);
                fRunAction->tube_z_entry .push_back(h->GetEntry().z()    / mm);
                fRunAction->tube_x_exit  .push_back(h->GetExit().x()     / mm);
                fRunAction->tube_y_exit  .push_back(h->GetExit().y()     / mm);
                fRunAction->tube_z_exit  .push_back(h->GetExit().z()     / mm);
            }
        }
    }

    // ------------------------------------------------------------------
    //  Push tile hits into vectors
    // ------------------------------------------------------------------
    if (fTileHCID >= 0) {
        auto* tileHC = dynamic_cast<TileHitsCollection*>(hce->GetHC(fTileHCID));
        if (tileHC) {
            const int n = static_cast<int>(tileHC->GetSize());
            for (int i = 0; i < n; ++i) {
                TileHit* h = static_cast<TileHit*>(tileHC->GetHit(i));

                fRunAction->tile_trackID .push_back(h->GetTrackID());
                fRunAction->tile_edep    .push_back(h->GetEdep()         / MeV);
                fRunAction->tile_tileX   .push_back(h->GetTileX());
                fRunAction->tile_tileY   .push_back(h->GetTileY());
                fRunAction->tile_sector  .push_back(h->GetSector());
                fRunAction->tile_x       .push_back(h->GetPosition().x() / mm);
                fRunAction->tile_y       .push_back(h->GetPosition().y() / mm);
                fRunAction->tile_z       .push_back(h->GetPosition().z() / mm);
            }
        }
    }

    // One row per event — fill after all hits are pushed
    if (fRunAction->GetTree())
        fRunAction->GetTree()->Fill();
}
