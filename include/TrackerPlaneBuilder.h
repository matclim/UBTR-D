#pragma once
#include <string>

class GeoVPhysVol;
class MaterialManager;

// ============================================================================
//  TrackerPlaneBuilder
//
//  Builds the 2×3 m² tracker plane into `mother` at z = zOffset_mm.
//
//  Regions (origin at plane centre):
//    Outer top:     x=[-1000,+1000] mm,  y=[+200,+1500] mm  — drift tubes
//    Outer bottom:  x=[-1000,+1000] mm,  y=[-1500,-200] mm  — drift tubes
//    Central tubes: x=[-600,+600]   mm,  y=[-200,+200]  mm  — drift tubes
//    Tile left:     x=[-1000,-600]  mm,  y=[-200,+200]  mm  — PS tiles
//    Tile right:    x=[+600,+1000]  mm,  y=[-200,+200]  mm  — PS tiles
//
//  Tubes: 5 mm diam, 15 µm mylar wall, ArCO2 70/30 fill, 1.2 m long, along X.
//  Tiles: 1×1×1 cm³ polystyrene, 1600 per block (40×40 grid).
//  All regions coplanar at z = zOffset_mm.
// ============================================================================
class TrackerPlaneBuilder {
public:
    static void build(GeoVPhysVol*     mother,
                      MaterialManager& MM,
                      double           zOffset_mm = 0.0,
                      const std::string& tag      = "TRK");
};
