#pragma once
#include "G4UserRunAction.hh"
#include <string>

class TFile;
class TTree;

// ============================================================================
//  TrackerRunAction
//  Opens the ROOT file, owns the two TTrees and their branch variables.
//  TrackerEventAction fills the variables and calls Fill() each event.
// ============================================================================
class TrackerRunAction : public G4UserRunAction {
public:
    explicit TrackerRunAction(const std::string& outFile = "tracker_hits.root");
    ~TrackerRunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*)   override;

    TTree* GetTubeTree() const { return fTubeTree; }
    TTree* GetTileTree() const { return fTileTree; }

    // ---- Tube branch variables (filled by TrackerEventAction) ----
    int    t_eventID  = 0;
    int    t_trackID  = 0;
    double t_edep     = 0.;
    double t_x        = 0., t_y     = 0., t_z     = 0.;
    double t_x_entry  = 0., t_y_entry = 0., t_z_entry = 0.;
    double t_x_exit   = 0., t_y_exit  = 0., t_z_exit  = 0.;

    // ---- Tile branch variables (filled by TrackerEventAction) ----
    int    p_eventID  = 0;
    int    p_trackID  = 0;
    double p_edep     = 0.;
    int    p_tileX    = 0,  p_tileY  = 0;
    int    p_sector   = 0;
    double p_x        = 0., p_y      = 0., p_z = 0.;

private:
    std::string fOutFileName;
    TFile*      fRootFile  = nullptr;
    TTree*      fTubeTree  = nullptr;
    TTree*      fTileTree  = nullptr;
};
