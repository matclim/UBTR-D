#pragma once
#include <vector>
#include "G4ThreeVector.hh"
#include "CLHEP/Units/SystemOfUnits.h"

// ============================================================================
//  UBTEventStore
//  Owns the per-event hit vectors.  Shared between the SD and EventAction.
//
//  Tube hits  (UBT hybrid detector only):
//    tube_trackID, tube_edep [MeV]
//    tube_{x,y,z}            [mm]  energy-weighted centroid
//    tube_{x,y,z}_entry/exit [mm]
//    tube_region              0=Central  1=Top_Left  2=Top_Right
//                             3=Bottom_Left  4=Bottom_Right
//
//  Tile hits  (both detectors):
//    tile_trackID, tile_edep [MeV]
//    tile_{x,y,z}            [mm]  first-step position
//    tile_tileX, tile_tileY   grid indices within the envelope
//    tile_region              0=FineLeft  1=FineRight  2=CoarseCentral
//                             3=CoarseTop  4=CoarseBottom
//                             (hybrid uses -1=TileLeft  +1=TileRight for sector)
// ============================================================================
struct UBTEventStore {

    // ---- tube hits (hybrid detector) ----------------------------------------
    std::vector<int>    tube_trackID;
    std::vector<double> tube_edep;
    std::vector<double> tube_x,       tube_y,       tube_z;
    std::vector<double> tube_x_entry, tube_y_entry, tube_z_entry;
    std::vector<double> tube_x_exit,  tube_y_exit,  tube_z_exit;
    std::vector<int>    tube_region;

    // ---- tile hits ----------------------------------------------------------
    std::vector<int>    tile_trackID;
    std::vector<double> tile_edep;
    std::vector<double> tile_x, tile_y, tile_z;
    std::vector<int>    tile_tileX, tile_tileY;
    std::vector<int>    tile_region;   // region code (see above)
    std::vector<int>    tile_sector;   

    // -------------------------------------------------------------------------
    void clear() {
        tube_trackID.clear();
        tube_edep.clear();
        tube_x.clear(); tube_y.clear(); tube_z.clear();
        tube_x_entry.clear(); tube_y_entry.clear(); tube_z_entry.clear();
        tube_x_exit.clear();  tube_y_exit.clear();  tube_z_exit.clear();
        tube_region.clear();

        tile_trackID.clear();
        tile_edep.clear();
        tile_x.clear(); tile_y.clear(); tile_z.clear();
        tile_tileX.clear(); tile_tileY.clear();
        tile_region.clear();
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
                    int tileX, int tileY, int region)
    {
        tile_trackID .push_back(trackID);
        tile_edep    .push_back(edep_G4 / CLHEP::MeV);
        tile_x       .push_back(pos.x() / CLHEP::mm);
        tile_y       .push_back(pos.y() / CLHEP::mm);
        tile_z       .push_back(pos.z() / CLHEP::mm);
        tile_tileX   .push_back(tileX);
        tile_tileY   .push_back(tileY);
        tile_region  .push_back(region);
    }
};
