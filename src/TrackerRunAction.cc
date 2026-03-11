#include "TrackerRunAction.hh"

#include "TFile.h"
#include "TTree.h"
#include "G4Run.hh"

TrackerRunAction::TrackerRunAction(const std::string& outFile)
    : fOutFileName(outFile)
{}

TrackerRunAction::~TrackerRunAction() = default;

void TrackerRunAction::BeginOfRunAction(const G4Run*)
{
    fRootFile = TFile::Open(fOutFileName.c_str(), "RECREATE");

    // ------------------------------------------------------------------
    //  Tube tree  (one row per track × gas-volume crossing × event)
    //  All positions in mm, energy in MeV.
    // ------------------------------------------------------------------
    fTubeTree = new TTree("TubeHits", "Drift tube gas hits");
    fTubeTree->Branch("eventID",  &t_eventID,  "eventID/I");
    fTubeTree->Branch("trackID",  &t_trackID,  "trackID/I");
    fTubeTree->Branch("edep",     &t_edep,     "edep/D");
    fTubeTree->Branch("x",        &t_x,        "x/D");
    fTubeTree->Branch("y",        &t_y,        "y/D");
    fTubeTree->Branch("z",        &t_z,        "z/D");
    fTubeTree->Branch("x_entry",  &t_x_entry,  "x_entry/D");
    fTubeTree->Branch("y_entry",  &t_y_entry,  "y_entry/D");
    fTubeTree->Branch("z_entry",  &t_z_entry,  "z_entry/D");
    fTubeTree->Branch("x_exit",   &t_x_exit,   "x_exit/D");
    fTubeTree->Branch("y_exit",   &t_y_exit,   "y_exit/D");
    fTubeTree->Branch("z_exit",   &t_z_exit,   "z_exit/D");

    // ------------------------------------------------------------------
    //  Tile tree  (one row per track × tile × event)
    //  sector: -1 = left block (x < 0),  +1 = right block (x > 0)
    // ------------------------------------------------------------------
    fTileTree = new TTree("TileHits", "Scintillator tile hits");
    fTileTree->Branch("eventID",  &p_eventID,  "eventID/I");
    fTileTree->Branch("trackID",  &p_trackID,  "trackID/I");
    fTileTree->Branch("edep",     &p_edep,     "edep/D");
    fTileTree->Branch("tileX",    &p_tileX,    "tileX/I");
    fTileTree->Branch("tileY",    &p_tileY,    "tileY/I");
    fTileTree->Branch("sector",   &p_sector,   "sector/I");
    fTileTree->Branch("x",        &p_x,        "x/D");
    fTileTree->Branch("y",        &p_y,        "y/D");
    fTileTree->Branch("z",        &p_z,        "z/D");
}

void TrackerRunAction::EndOfRunAction(const G4Run*)
{
    if (fRootFile) {
        fRootFile->Write();
        fRootFile->Close();
        delete fRootFile;
        fRootFile = nullptr;
        fTubeTree = nullptr;   // owned by the file; already deleted
        fTileTree = nullptr;
    }
}
