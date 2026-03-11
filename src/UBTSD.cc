#include "UBTSD.hh"
#include "UBTEventStore.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4TouchableHandle.hh"
#include "G4SystemOfUnits.hh"

#include <atomic>
#include <mutex>
#include <set>
#include <regex>
#include <sstream>

static std::atomic<bool> s_firstTube{true};
static std::atomic<bool> s_firstTile{true};

// ----------------------------------------------------------------------------
//  Make a unique string key for a (trackID, volume instance) pair.
//
//  The problem: all tubes in a region share one GeoLogVol, so Geo2G4 gives
//  every tube PV the SAME name (e.g. "TRK_CentralTubes_Wall_PV").
//  The PV *pointer* is stable within one step but changes between steps of
//  the same track in the same tube (Geant4 re-uses wrapper objects).
//
//  Solution: use the full touchable history path as the key.
//  touch->GetHistoryDepth() levels of (volumeName, copyNo) pairs give a
//  globally unique address for every placed volume in the hierarchy.
// ----------------------------------------------------------------------------
static std::string touchableKey(int trackID,
                                const G4TouchableHandle& touch)
{
    std::ostringstream oss;
    oss << trackID << '|';
    const int depth = touch->GetHistoryDepth();
    for (int i = 0; i <= depth; ++i) {
        oss << touch->GetVolume(i)->GetName()
            << ':' << touch->GetCopyNumber(i) << '/';
    }
    return oss.str();
}

UBTSD::UBTSD(const G4String& name, UBTEventStore* store)
    : G4VSensitiveDetector(name), fStore(store)
{}

void UBTSD::Initialize(G4HCofThisEvent*)
{
    fMap.clear();
    s_firstTube = true;
    s_firstTile = true;
}

G4bool UBTSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    const G4StepPoint*   pre    = step->GetPreStepPoint();
    const G4StepPoint*   post   = step->GetPostStepPoint();
    G4VPhysicalVolume*   pv     = pre->GetPhysicalVolume();
    if (!pv) return false;

    const double         edep   = step->GetTotalEnergyDeposit();
    const std::string    lvName = pv->GetLogicalVolume()->GetName();
    const G4TouchableHandle& touch = pre->GetTouchableHandle();

    // ----------------------------------------------------------------
    //  Tube gas hit
    // ----------------------------------------------------------------
    if (lvName.find("TubeGas_LV") != std::string::npos) {

        const std::string key = touchableKey(step->GetTrack()->GetTrackID(), touch);

        if (s_firstTube.exchange(false)) {
            G4cout << "[UBTSD] First tube step:"
                   << " LV="   << lvName
                   << " PV="   << pv->GetName()
                   << " key="  << key
                   << " edep=" << edep/MeV << " MeV"
                   << " depth=" << touch->GetHistoryDepth() << G4endl;
        }

        auto& agg = fMap[key];
        if (!agg.hasEntry) {
            agg.isTube   = true;
            agg.trackID  = step->GetTrack()->GetTrackID();
            agg.entry    = pre->GetPosition();
            agg.hasEntry = true;
            agg.region   = regionCode(lvName);
        }
        agg.exit_pt = post->GetPosition();
        if (edep > 0.0) {
            const G4ThreeVector mid = 0.5*(pre->GetPosition()+post->GetPosition());
            agg.edep    += edep;
            agg.sumEpos += edep * mid;
        }
        return true;
    }

    // ----------------------------------------------------------------
    //  Tile hit
    // ----------------------------------------------------------------
    if (lvName.find("Tile_LV") != std::string::npos) {

        const std::string key = touchableKey(step->GetTrack()->GetTrackID(), touch);

        if (s_firstTile.exchange(false)) {
            G4cout << "[UBTSD] First tile step:"
                   << " LV="   << lvName
                   << " PV="   << pv->GetName()
                   << " key="  << key
                   << " edep=" << edep/MeV << " MeV" << G4endl;
        }

        auto& agg = fMap[key];
        if (!agg.hasEntry) {
            agg.isTile   = true;
            agg.trackID  = step->GetTrack()->GetTrackID();
            agg.firstPos = pre->GetPosition();
            agg.hasEntry = true;
            parseTilePVName(pv->GetName(), agg.tileX, agg.tileY, agg.sector);
        }
        if (edep > 0.0) agg.edep += edep;
        return true;
    }

    return false;
}

void UBTSD::EndOfEvent(G4HCofThisEvent*)
{
    int nTube = 0, nTile = 0;

    for (const auto& [key, agg] : fMap) {
        if (!agg.hasEntry) continue;

        if (agg.isTube && agg.edep > 0.0) {
            const G4ThreeVector pos = agg.sumEpos / agg.edep;
            fStore->addTubeHit(agg.trackID, agg.edep,
                               pos, agg.entry, agg.exit_pt, agg.region);
            ++nTube;
        }
        else if (agg.isTile && agg.edep > 0.0) {
            fStore->addTileHit(agg.trackID, agg.edep,
                               agg.firstPos,
                               agg.tileX, agg.tileY, agg.sector);
            ++nTile;
        }
    }

    G4cout << "[UBTSD] EndOfEvent: tube hits=" << nTube
           << "  tile hits=" << nTile
           << "  map entries=" << fMap.size() << G4endl;
}

int UBTSD::regionCode(const std::string& lvName)
{
    if (lvName.find("CentralTubes") != std::string::npos) return 0;
    if (lvName.find("Top_Left")     != std::string::npos) return 1;
    if (lvName.find("Top_Right")    != std::string::npos) return 2;
    if (lvName.find("Bottom_Left")  != std::string::npos) return 3;
    if (lvName.find("Bottom_Right") != std::string::npos) return 4;
    return -1;
}

bool UBTSD::parseTilePVName(const std::string& name,
                                int& ix, int& iy, int& sector)
{
    if      (name.find("TileLeft")  != std::string::npos) sector = -1;
    else if (name.find("TileRight") != std::string::npos) sector = +1;
    else { sector = 0; ix = iy = -1; return false; }

    static const std::regex re("_T(\\d+)_(\\d+)");
    std::smatch m;
    if (std::regex_search(name, m, re)) {
        ix = std::stoi(m[1].str());
        iy = std::stoi(m[2].str());
        return true;
    }
    ix = iy = -1;
    return false;
}
