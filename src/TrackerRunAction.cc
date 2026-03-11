#include "TrackerRunAction.hh"

#include "TFile.h"
#include "TTree.h"
#include "G4Run.hh"

TrackerRunAction::TrackerRunAction(const std::string& outFile)
    : fOutFileName(outFile)
{}

TrackerRunAction::~TrackerRunAction() = default;

void TrackerRunAction::ClearVectors()
{
    tube_trackID.clear();
    tube_edep.clear();
    tube_x.clear();       tube_y.clear();       tube_z.clear();
    tube_x_entry.clear(); tube_y_entry.clear(); tube_z_entry.clear();
    tube_x_exit.clear();  tube_y_exit.clear();  tube_z_exit.clear();

    tile_trackID.clear();
    tile_edep.clear();
    tile_tileX.clear();   tile_tileY.clear();
    tile_sector.clear();
    tile_x.clear();       tile_y.clear();       tile_z.clear();
}

void TrackerRunAction::BeginOfRunAction(const G4Run*)
{
    fRootFile = TFile::Open(fOutFileName.c_str(), "RECREATE");

    fTree = new TTree("Events", "Tracker hits — one entry per event");

    // ---- Tube branches ------------------------------------------------------
    fTree->Branch("tube_trackID",  &tube_trackID);
    fTree->Branch("tube_edep",     &tube_edep);
    fTree->Branch("tube_x",        &tube_x);
    fTree->Branch("tube_y",        &tube_y);
    fTree->Branch("tube_z",        &tube_z);
    fTree->Branch("tube_x_entry",  &tube_x_entry);
    fTree->Branch("tube_y_entry",  &tube_y_entry);
    fTree->Branch("tube_z_entry",  &tube_z_entry);
    fTree->Branch("tube_x_exit",   &tube_x_exit);
    fTree->Branch("tube_y_exit",   &tube_y_exit);
    fTree->Branch("tube_z_exit",   &tube_z_exit);

    // ---- Tile branches ------------------------------------------------------
    fTree->Branch("tile_trackID",  &tile_trackID);
    fTree->Branch("tile_edep",     &tile_edep);
    fTree->Branch("tile_tileX",    &tile_tileX);
    fTree->Branch("tile_tileY",    &tile_tileY);
    fTree->Branch("tile_sector",   &tile_sector);
    fTree->Branch("tile_x",        &tile_x);
    fTree->Branch("tile_y",        &tile_y);
    fTree->Branch("tile_z",        &tile_z);
}

void TrackerRunAction::EndOfRunAction(const G4Run*)
{
    if (fRootFile) {
        fRootFile->Write();
        fRootFile->Close();
        delete fRootFile;
        fRootFile = nullptr;
        fTree     = nullptr;   // owned and deleted by TFile
    }
}
