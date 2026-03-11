// ============================================================================
//  Integration notes – how to add the tracker plane to DetectorConstruction
// ============================================================================
//
//  1.  Add the two new source files to your build system:
//          TrackerPlaneBuilder.cpp
//          (paste MaterialManager_TrackerAdditions.cpp into MaterialManager.cpp)
//
//  2.  Add declarations to MaterialManager.h:
//          GeoMaterial* mylar();
//          GeoMaterial* arco2();
//
//  3.  In DetectorConstruction.cc, include the new builder:
//          #include "TrackerPlaneBuilder.h"
//
//  4.  In DetectorConstruction::buildGeoModelWorld(), after creating worldPhys,
//      add a single call:
//
//          TrackerPlaneBuilder::build(worldPhys, MM,
//                                     /*zOffset_mm=*/ 0.0,
//                                     /*tag=*/        "TRK");
//
//      The tracker plane will be placed centred at (0, 0, 0) in the world.
//      Adjust zOffset_mm to position it along the beam axis as needed.
//
//  5.  Sensitive detector wiring (optional).
//      In DetectorConstruction::Construct(), extend the LV-scan loop to mark
//      the ArCO2 gas volumes as sensitive:
//
//          } else if (ln.find("TubeGas_LV") != std::string::npos) {
//              lv->SetSensitiveDetector(caloSD); ++nSensitive;
//          }
//
//      Or create a dedicated TrackerSD if you want separate hit collections.
//
//  6.  World size.
//      The plane is 3000 mm × 2000 mm × ~20 mm.
//      Make sure the worldXY in buildGeoModelWorld() is at least 3000 mm.
//      The existing worldZ = 120827 mm is already sufficient.
//
// ============================================================================
//
//  DESIGN NOTES
//  ------------
//
//  A. Double-staggered tube layers
//     The term "double-staggered" here means two sub-layers:
//
//       sub-layer 0  (z = -r):    tube centres at y = y0, y0+p, y0+2p, ...
//       sub-layer 1  (z = +r):    tube centres at y = y0+p/2, y0+3p/2, ...
//
//     where p = 2r = 5 mm (tight packing).  The z-separation of 2r = 5 mm
//     means the two sub-layers are just touching.  This is the standard
//     "double layer" used in straw-tube trackers.  For a true staggered
//     honeycomb you would use z-separation = r*sqrt(3); adjust dzSub[] in
//     placeDriftTubeLayer() if needed.
//
//  B. Overlap region
//     The left half spans  X ∈ [-1500, +100] (centre -700, half-width 800 mm).
//     The right half spans X ∈ [-100, +1500] (centre +700, half-width 800 mm).
//     The 200 mm central strip [-100, +100] is thus covered by BOTH halves,
//     providing the required overlap.
//     The central tube region (B) additionally covers [-600, +600] × [-200, +200]
//     with its own independent double-staggered structure.
//
//  C. Tile geometry
//     Each tile block is 400 mm (X) × 400 mm (Y) × 10 mm (Z),
//     subdivided into 40 × 40 = 1600 individual 10×10×10 mm³ tiles
//     (GeoBox half-sizes 5 × 5 × 5 mm).  All tiles share a single GeoLogVol
//     (reused physical volumes), consistent with the BarLayer pattern in the
//     reference code.
//     The tile blocks are placed immediately behind (+Z) the drift tube stack.
//
//  D. Material note
//     The mylar wall is modelled as a hollow GeoTube (rInner to rOuter).
//     The gas lumen is a solid GeoTube (0 to rInner) placed as a daughter of
//     the wall PV, following the same cladding/core nesting used in
//     Fibre_HPLayer for scintillating fibres.
//
// ============================================================================
