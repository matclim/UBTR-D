#pragma once
#include <vector>
#include "G4ThreeVector.hh"
#include "CLHEP/Units/SystemOfUnits.h"

// ============================================================================
//  UBTEventStore
//  Owns the per-event hit vectors. Shared between UBTSD and
//  UBTEventAction. Pattern copied from the calorimeter EventStore.
//
//  Tube hit vectors  (one element = one track crossing one named gas volume):
//    tube_trackID, tube_edep [MeV],
//    tube_{x,y,z} [mm]        — midpoint of last step
//    tube_{x,y,z}_entry [mm]  — first step pre-point
//    tube_{x,y,z}_exit  [mm]  — last  step post-point
//    tube_region              — integer region code (see below)
//
//  Tile hit vectors  (one element = one track crossing one tile):
//    tile_trackID, tile_edep [MeV],
//    tile_{x,y,z} [mm], tile_tileX, tile_tileY, tile_sector
//
//  Region codes for tube_region:
//    0 = CentralTubes
//    1 = Top_Left     2 = Top_Right
//    3 = Bottom_Left  4 = Bottom_Right
// ============================================================================
struct UBTEventStore {

    // ---- tube hits ----------------------------------------------------------
    std::vector<int>    tube_trackID;
    std::vector<double> tube_edep;
    std::vector<double> tube_x,       tube_y,       tube_z;
    std::vector<double> tube_x_entry, tube_y_entry, tube_z_entry;
    std::vector<double> tube_x_exit,  tube_y_exit,  tube_z_exit;
    std::vector<int>    tube_region;

    // ---- tile hits ----------------------------------------------------------
    std::vector<int>    tile_trackID;
    std::vector<double> tile_edep;
    std::vector<double> tile_x,    tile_y,    tile_z;
    std::vector<int>    tile_tileX, tile_tileY;
    std::vector<int>    tile_sector;

    // -------------------------------------------------------------------------
    void clear() {
        tube_trackID.clear();
        tube_edep.clear();
        tube_x.clear();       tube_y.clear();       tube_z.clear();
        tube_x_entry.clear(); tube_y_entry.clear(); tube_z_entry.clear();
        tube_x_exit.clear();  tube_y_exit.clear();  tube_z_exit.clear();
        tube_region.clear();

        tile_trackID.clear();
        tile_edep.clear();
        tile_x.clear();    tile_y.clear();    tile_z.clear();
        tile_tileX.clear(); tile_tileY.clear();
        tile_sector.clear();
    }

    void addTubeHit(int trackID, double edep_G4,
                    const G4ThreeVector& pos,
                    const G4ThreeVector& entry,
                    const G4ThreeVector& exit_pt,
                    int region)
    {
        tube_trackID .push_back(trackID);
        tube_edep    .push_back(edep_G4 / CLHEP::MeV);
        tube_x       .push_back(pos.x()     / CLHEP::mm);
        tube_y       .push_back(pos.y()     / CLHEP::mm);
        tube_z       .push_back(pos.z()     / CLHEP::mm);
        tube_x_entry .push_back(entry.x()   / CLHEP::mm);
        tube_y_entry .push_back(entry.y()   / CLHEP::mm);
        tube_z_entry .push_back(entry.z()   / CLHEP::mm);
        tube_x_exit  .push_back(exit_pt.x() / CLHEP::mm);
        tube_y_exit  .push_back(exit_pt.y() / CLHEP::mm);
        tube_z_exit  .push_back(exit_pt.z() / CLHEP::mm);
        tube_region  .push_back(region);
    }

    void addTileHit(int trackID, double edep_G4,
                    const G4ThreeVector& pos,
                    int tileX, int tileY, int sector)
    {
        tile_trackID .push_back(trackID);
        tile_edep    .push_back(edep_G4 / CLHEP::MeV);
        tile_x       .push_back(pos.x() / CLHEP::mm);
        tile_y       .push_back(pos.y() / CLHEP::mm);
        tile_z       .push_back(pos.z() / CLHEP::mm);
        tile_tileX   .push_back(tileX);
        tile_tileY   .push_back(tileY);
        tile_sector  .push_back(sector);
    }
};
