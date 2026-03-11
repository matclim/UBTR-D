#pragma once
#include "G4VSensitiveDetector.hh"
#include "G4ThreeVector.hh"
#include <unordered_map>
#include <string>

class UBTEventStore;
class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

// Per-volume aggregator — mirrors CaloSD's HitAgg
struct UBTHitAgg {
    int    trackID  = -1;
    double edep     = 0.0;
    bool   isTube   = false;
    bool   isTile   = false;

    // tube fields
    G4ThreeVector entry  = {0,0,0};
    G4ThreeVector exit_pt= {0,0,0};
    G4ThreeVector sumEpos= {0,0,0};   // energy-weighted position sum
    bool          hasEntry = false;
    int           region   = -1;

    // tile fields
    G4ThreeVector firstPos = {0,0,0};
    int tileX = -1, tileY = -1, sector = 0;
};

// ============================================================================
//  UBTSD
//  Aggregates steps by pvName (stable key even for shared LVs) exactly as
//  CaloSD does. Pushes completed hits into UBTEventStore in EndOfEvent.
// ============================================================================
class UBTSD : public G4VSensitiveDetector {
public:
    UBTSD(const G4String& name, UBTEventStore* store);
    ~UBTSD() override = default;

    void   Initialize(G4HCofThisEvent*) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
    void   EndOfEvent(G4HCofThisEvent*) override;

private:
    UBTEventStore* fStore = nullptr;
    std::unordered_map<std::string, UBTHitAgg> fMap;

    static int  regionCode(const std::string& lvName);
    static bool parseTilePVName(const std::string& pvName,
                                int& ix, int& iy, int& sector);
};
