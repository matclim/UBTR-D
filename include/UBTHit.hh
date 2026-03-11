#pragma once

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include <string>

// ============================================================================
//  TubeHit
//  One hit = one charged track passing through one ArCO2 gas volume.
//  Accumulates total energy deposit; records entry and exit points.
// ============================================================================
class TubeHit : public G4VHit {
public:
    TubeHit() = default;
    ~TubeHit() override = default;

    // Accumulated over all steps in this gas volume for this track
    void AddEdep(double e)              { fEdep += e; }
    void SetEntryPoint(G4ThreeVector v) { fEntry = v; fHasEntry = true; }
    void SetExitPoint(G4ThreeVector v)  { fExit  = v; }
    void SetPosition(G4ThreeVector v)   { fPos   = v; }   // global step position
    void SetVolumeName(const std::string& n) { fVolName = n; }
    void SetTrackID(int id)             { fTrackID = id; }
    void SetRegion(const std::string& r){ fRegion = r; }  // e.g. "Top_Left"

    double           GetEdep()       const { return fEdep; }
    G4ThreeVector    GetPosition()   const { return fPos; }
    G4ThreeVector    GetEntry()      const { return fEntry; }
    G4ThreeVector    GetExit()       const { return fExit; }
    const std::string& GetVolName()  const { return fVolName; }
    int              GetTrackID()    const { return fTrackID; }
    const std::string& GetRegion()   const { return fRegion; }
    bool             HasEntry()      const { return fHasEntry; }

private:
    double        fEdep    = 0.0;
    G4ThreeVector fPos     = {0,0,0};
    G4ThreeVector fEntry   = {0,0,0};
    G4ThreeVector fExit    = {0,0,0};
    std::string   fVolName = "";
    std::string   fRegion  = "";
    int           fTrackID = -1;
    bool          fHasEntry = false;
};

using TubeHitsCollection = G4THitsCollection<TubeHit>;

// ============================================================================
//  TileHit
//  One hit = one charged track depositing energy in one polystyrene tile.
//  Records tile index (ix, iy), sector (left=-1, right=+1), and total edep.
// ============================================================================
class TileHit : public G4VHit {
public:
    TileHit() = default;
    ~TileHit() override = default;

    void AddEdep(double e)              { fEdep += e; }
    void SetTileIndex(int ix, int iy)   { fTileX = ix; fTileY = iy; }
    void SetSector(int s)               { fSector = s; }   // -1=left, +1=right
    void SetPosition(G4ThreeVector v)   { fPos = v; }
    void SetTrackID(int id)             { fTrackID = id; }

    double        GetEdep()    const { return fEdep; }
    int           GetTileX()   const { return fTileX; }
    int           GetTileY()   const { return fTileY; }
    int           GetSector()  const { return fSector; }
    G4ThreeVector GetPosition()const { return fPos; }
    int           GetTrackID() const { return fTrackID; }

private:
    double        fEdep    = 0.0;
    G4ThreeVector fPos     = {0,0,0};
    int           fTileX   = -1;
    int           fTileY   = -1;
    int           fSector  = 0;
    int           fTrackID = -1;
};

using TileHitsCollection = G4THitsCollection<TileHit>;
