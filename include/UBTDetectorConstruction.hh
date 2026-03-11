#pragma once
#include "G4VUserDetectorConstruction.hh"
#include "UBTEventStore.hh"
#include <string>

class GeoPhysVol;

class UBTDetectorConstruction : public G4VUserDetectorConstruction {
public:
    explicit UBTDetectorConstruction(bool writeGdml = false);
    ~UBTDetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    void SetVisMode(int mode)   { m_visMode    = mode; }
    void SetRegisterSD(bool on) { m_registerSD = on;   }

    // Access the shared event store (wired into EventAction at startup)
    UBTEventStore* GetStore() { return &m_store; }

private:
    bool               m_writeGdml  = false;
    int                m_visMode    = 1;
    bool               m_registerSD = false;
    UBTEventStore  m_store;      // owned here, shared by ptr

    GeoPhysVol* buildGeoModelWorld();
};
