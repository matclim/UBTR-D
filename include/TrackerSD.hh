#pragma once

#include "G4VSensitiveDetector.hh"
#include "TrackerHit.hh"
#include <unordered_map>
#include <string>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

// ============================================================================
//  TrackerSD
//
//  Single sensitive detector class registered on both gas volumes (ArCO2)
//  and tile volumes (polystyrene). Distinguishes them by logical volume name:
//    - names containing "TubeGas_LV"  → TubeHit
//    - names containing "Tile_LV"     → TileHit
//
//  Hit accumulation strategy:
//    Tube hits: one TubeHit per (trackID, gas volume copy) per event.
//               Entry point = first step pre-point in that volume.
//               Exit  point = last step post-point.
//               Position    = energy-weighted centroid of all steps.
//    Tile hits: one TileHit per (trackID, tile copy) per event.
//               Tile ix/iy and sector decoded from volume name.
// ============================================================================
class TrackerSD : public G4VSensitiveDetector {
public:
    explicit TrackerSD(const std::string& name);
    ~TrackerSD() override = default;

    void   Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    void   EndOfEvent(G4HCofThisEvent* hce) override;

    static const char* TubeHCname() { return "TubeHitsCollection"; }
    static const char* TileHCname() { return "TileHitsCollection"; }

private:
    TubeHitsCollection* fTubeHC = nullptr;
    TileHitsCollection* fTileHC = nullptr;
    int fTubeHCID = -1;
    int fTileHCID = -1;

    // Map (trackID << 32 | copyNo) → index in hits collection
    std::unordered_map<uint64_t, int> fTubeHitMap;
    std::unordered_map<uint64_t, int> fTileHitMap;

    // Parse tile ix, iy, sector from volume name string
    // Volume names follow the pattern: TRK_TileLeft_T<ix>_<iy>_... or TRK_TileRight_...
    static bool parseTileVolumeName(const std::string& volName,
                                    int& ix, int& iy, int& sector);
};
