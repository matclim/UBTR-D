#pragma once
#include "G4UserRunAction.hh"
#include <string>
#include <vector>

class TFile;
class TTree;

// ============================================================================
//  TrackerRunAction
//
//  One TTree, one row per event.
//  Every branch is a std::vector; vectors are cleared at BeginOfEvent and
//  filled by TrackerEventAction before the single Fill() call at EndOfEvent.
//
//  Tube branches  (one entry per tube hit = one track crossing one gas volume):
//    tube_trackID              : Geant4 track ID
//    tube_edep                 : total energy deposit in that gas volume [MeV]
//    tube_x/y/z                : global position of last step midpoint [mm]
//    tube_x/y/z_entry          : track entry point into the gas volume [mm]
//    tube_x/y/z_exit           : track exit  point from the gas volume [mm]
//
//  Tile branches  (one entry per tile hit = one track crossing one tile):
//    tile_trackID              : Geant4 track ID
//    tile_edep                 : total energy deposit [MeV]
//    tile_tileX / tile_tileY   : tile grid indices (0-39)
//    tile_sector               : -1 = left block, +1 = right block
//    tile_x/y/z                : global hit position [mm]
// ============================================================================
class TrackerRunAction : public G4UserRunAction {
public:
    explicit TrackerRunAction(const std::string& outFile = "tracker_hits.root");
    ~TrackerRunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*)   override;

    TTree* GetTree() const { return fTree; }

    // ---- Tube hit vectors ---------------------------------------------------
    std::vector<int>    tube_trackID;
    std::vector<double> tube_edep;
    std::vector<double> tube_x,       tube_y,       tube_z;
    std::vector<double> tube_x_entry, tube_y_entry, tube_z_entry;
    std::vector<double> tube_x_exit,  tube_y_exit,  tube_z_exit;

    // ---- Tile hit vectors ---------------------------------------------------
    std::vector<int>    tile_trackID;
    std::vector<double> tile_edep;
    std::vector<int>    tile_tileX,   tile_tileY;
    std::vector<int>    tile_sector;
    std::vector<double> tile_x,       tile_y,       tile_z;

    // Clears all vectors — called by TrackerEventAction::BeginOfEventAction
    void ClearVectors();

private:
    std::string fOutFileName;
    TFile*      fRootFile = nullptr;
    TTree*      fTree     = nullptr;
};
