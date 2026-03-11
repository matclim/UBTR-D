#pragma once
#include "G4VUserDetectorConstruction.hh"
#include "TrackerEventStore.hh"
#include <string>

class GeoPhysVol;

class TrackerDetectorConstruction : public G4VUserDetectorConstruction {
public:
    explicit TrackerDetectorConstruction(bool writeGdml = false);
    ~TrackerDetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    void SetVisMode(int mode)   { m_visMode    = mode; }
    void SetRegisterSD(bool on) { m_registerSD = on;   }

    // Access the shared event store (wired into EventAction at startup)
    TrackerEventStore* GetStore() { return &m_store; }

private:
    bool               m_writeGdml  = false;
    int                m_visMode    = 1;
    bool               m_registerSD = false;
    TrackerEventStore  m_store;      // owned here, shared by ptr

    GeoPhysVol* buildGeoModelWorld();
};
