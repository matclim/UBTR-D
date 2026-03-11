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

void TrackerEventAction::BeginOfEventAction(const G4Event*) {}

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

    const int evtID = event->GetEventID();

    // ------------------------------------------------------------------
    //  Tube hits
    // ------------------------------------------------------------------
    TTree* tubeTree = fRunAction->GetTubeTree();
    if (tubeTree && fTubeHCID >= 0) {
        auto* tubeHC = dynamic_cast<TubeHitsCollection*>(hce->GetHC(fTubeHCID));
        if (tubeHC) {
            const int n = static_cast<int>(tubeHC->GetSize());
            for (int i = 0; i < n; ++i) {
                TubeHit* h = static_cast<TubeHit*>(tubeHC->GetHit(i));

                fRunAction->t_eventID  = evtID;
                fRunAction->t_trackID  = h->GetTrackID();
                fRunAction->t_edep     = h->GetEdep()          / MeV;
                fRunAction->t_x        = h->GetPosition().x()  / mm;
                fRunAction->t_y        = h->GetPosition().y()  / mm;
                fRunAction->t_z        = h->GetPosition().z()  / mm;
                fRunAction->t_x_entry  = h->GetEntry().x()     / mm;
                fRunAction->t_y_entry  = h->GetEntry().y()     / mm;
                fRunAction->t_z_entry  = h->GetEntry().z()     / mm;
                fRunAction->t_x_exit   = h->GetExit().x()      / mm;
                fRunAction->t_y_exit   = h->GetExit().y()      / mm;
                fRunAction->t_z_exit   = h->GetExit().z()      / mm;

                tubeTree->Fill();
            }
        }
    }

    // ------------------------------------------------------------------
    //  Tile hits
    // ------------------------------------------------------------------
    TTree* tileTree = fRunAction->GetTileTree();
    if (tileTree && fTileHCID >= 0) {
        auto* tileHC = dynamic_cast<TileHitsCollection*>(hce->GetHC(fTileHCID));
        if (tileHC) {
            const int n = static_cast<int>(tileHC->GetSize());
            for (int i = 0; i < n; ++i) {
                TileHit* h = static_cast<TileHit*>(tileHC->GetHit(i));

                fRunAction->p_eventID  = evtID;
                fRunAction->p_trackID  = h->GetTrackID();
                fRunAction->p_edep     = h->GetEdep()          / MeV;
                fRunAction->p_tileX    = h->GetTileX();
                fRunAction->p_tileY    = h->GetTileY();
                fRunAction->p_sector   = h->GetSector();
                fRunAction->p_x        = h->GetPosition().x()  / mm;
                fRunAction->p_y        = h->GetPosition().y()  / mm;
                fRunAction->p_z        = h->GetPosition().z()  / mm;

                tileTree->Fill();
            }
        }
    }
}
