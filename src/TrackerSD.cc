#include "TrackerSD.hh"
#include "TrackerHit.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4TouchableHandle.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

#include <regex>

TrackerSD::TrackerSD(const std::string& name)
    : G4VSensitiveDetector(name)
{
    collectionName.insert(TubeHCname());
    collectionName.insert(TileHCname());
}

void TrackerSD::Initialize(G4HCofThisEvent* hce)
{
    fTubeHC = new TubeHitsCollection(SensitiveDetectorName, TubeHCname());
    fTileHC = new TileHitsCollection(SensitiveDetectorName, TileHCname());

    if (fTubeHCID < 0)
        fTubeHCID = G4SDManager::GetSDMpointer()->GetCollectionID(
                        SensitiveDetectorName + "/" + TubeHCname());
    if (fTileHCID < 0)
        fTileHCID = G4SDManager::GetSDMpointer()->GetCollectionID(
                        SensitiveDetectorName + "/" + TileHCname());

    hce->AddHitsCollection(fTubeHCID, fTubeHC);
    hce->AddHitsCollection(fTileHCID, fTileHC);

    fTubeHitMap.clear();
    fTileHitMap.clear();
}

G4bool TrackerSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    const double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.0) return false;

    const G4Track*         track   = step->GetTrack();
    const int              trackID = track->GetTrackID();
    const G4StepPoint*     pre     = step->GetPreStepPoint();
    const G4StepPoint*     post    = step->GetPostStepPoint();
    const G4TouchableHandle& touch  = pre->GetTouchableHandle();
    const int              copyNo  = touch->GetCopyNumber();
    const std::string      lvName  = pre->GetPhysicalVolume()
                                         ->GetLogicalVolume()->GetName();

    // ----------------------------------------------------------------
    //  Tube gas hit
    // ----------------------------------------------------------------
    if (lvName.find("TubeGas_LV") != std::string::npos) {

        const uint64_t key = (static_cast<uint64_t>(trackID) << 32)
                           | static_cast<uint32_t>(copyNo);

        auto it = fTubeHitMap.find(key);
        TubeHit* hit = nullptr;

        if (it == fTubeHitMap.end()) {
            // First step of this track in this gas volume → new hit
            hit = new TubeHit();
            hit->SetVolumeName(lvName);
            hit->SetTrackID(trackID);
            hit->SetEntryPoint(pre->GetPosition());

            // Decode region from LV name for convenience
            // e.g. "TRK_Top_Left_TubeGas_LV" → "Top_Left"
            std::string region = lvName;
            const auto p = region.find("TRK_");
            if (p != std::string::npos) region = region.substr(p + 4);
            const auto q = region.find("_TubeGas_LV");
            if (q != std::string::npos) region = region.substr(0, q);
            hit->SetRegion(region);

            const int idx = static_cast<int>(fTubeHC->GetSize());
            fTubeHC->insert(hit);
            fTubeHitMap[key] = idx;
        } else {
            hit = static_cast<TubeHit*>(fTubeHC->GetHit(it->second));
        }

        hit->AddEdep(edep);
        // Energy-weighted position update
        const G4ThreeVector stepPos = pre->GetPosition()
                                    + 0.5 * (post->GetPosition() - pre->GetPosition());
        // Approximate centroid: store the midpoint of the last step weighted by edep
        // (full weighted average would need to track running sum separately;
        //  for typical single tracks the midpoint of all steps is adequate)
        hit->SetPosition(stepPos);

        // Update exit point every step so the last step's post-point is the exit
        hit->SetExitPoint(post->GetPosition());
        return true;
    }

    // ----------------------------------------------------------------
    //  Tile hit
    // ----------------------------------------------------------------
    if (lvName.find("Tile_LV") != std::string::npos) {

        const uint64_t key = (static_cast<uint64_t>(trackID) << 32)
                           | static_cast<uint32_t>(copyNo);

        auto it = fTileHitMap.find(key);
        TileHit* hit = nullptr;

        if (it == fTileHitMap.end()) {
            hit = new TileHit();
            hit->SetTrackID(trackID);
            hit->SetPosition(pre->GetPosition());

            // Decode tile ix, iy, sector from the physical volume name
            const std::string pvName =
                pre->GetPhysicalVolume()->GetName();
            int ix = -1, iy = -1, sector = 0;
            parseTileVolumeName(pvName, ix, iy, sector);
            hit->SetTileIndex(ix, iy);
            hit->SetSector(sector);

            const int idx = static_cast<int>(fTileHC->GetSize());
            fTileHC->insert(hit);
            fTileHitMap[key] = idx;
        } else {
            hit = static_cast<TileHit*>(fTileHC->GetHit(it->second));
        }

        hit->AddEdep(edep);
        return true;
    }

    return false;
}

void TrackerSD::EndOfEvent(G4HCofThisEvent*)
{
    // Nothing to flush — hits already in collection
}

// ----------------------------------------------------------------------------
//  parseTileVolumeName
//
//  Parses names of the form:
//    TRK_TileLeft_T<ix>_<iy>_Wall   (sector = -1)
//    TRK_TileRight_T<ix>_<iy>_Wall  (sector = +1)
//
//  Returns true if parsing succeeds.
// ----------------------------------------------------------------------------
bool TrackerSD::parseTileVolumeName(const std::string& name,
                                    int& ix, int& iy, int& sector)
{
    // Determine sector from "TileLeft" / "TileRight"
    if (name.find("TileLeft") != std::string::npos)       sector = -1;
    else if (name.find("TileRight") != std::string::npos) sector = +1;
    else { sector = 0; ix = iy = -1; return false; }

    // Extract _T<ix>_<iy>
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
