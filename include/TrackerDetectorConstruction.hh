#pragma once

#include "G4VUserDetectorConstruction.hh"
#include <string>

class GeoPhysVol;

class TrackerDetectorConstruction : public G4VUserDetectorConstruction {
public:
    explicit TrackerDetectorConstruction(bool writeGdml = false);
    ~TrackerDetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    void SetVisMode(int mode)   { m_visMode    = mode; }
    void SetRegisterSD(bool on) { m_registerSD = on;   }

private:
    bool m_writeGdml  = false;
    int  m_visMode    = 1;      // 0=envelopes only  1=by-region  2=full detail
    bool m_registerSD = false;  // false for vis-only, true for simulation

    GeoPhysVol* buildGeoModelWorld();
};
