#pragma once

#include "G4VUserDetectorConstruction.hh"
#include <string>

// Forward declarations
class GeoPhysVol;

// ============================================================================
//  TrackerDetectorConstruction
//
//  Minimal Geant4 detector construction for the 2×3 m² tracker plane.
//  Intended for standalone visualisation and GDML export only —
//  no sensitive detectors or hit collections are registered.
//
//  Usage (from main_g4.cpp when --tracker-only flag is set):
//      auto* det = new TrackerDetectorConstruction(write_gdml);
//      det->SetVisMode(visMode);
//      runManager->SetUserInitialization(det);
// ============================================================================

class TrackerDetectorConstruction : public G4VUserDetectorConstruction {
public:
    explicit TrackerDetectorConstruction(bool writeGdml = false);
    ~TrackerDetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    void SetVisMode(int mode) { m_visMode = mode; }

private:
    bool m_writeGdml;
    int  m_visMode = 1;   // default: colour by region

    GeoPhysVol* buildGeoModelWorld();
};
