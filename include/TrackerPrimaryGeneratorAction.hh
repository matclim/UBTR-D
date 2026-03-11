#pragma once
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include <string>

class G4ParticleGun;
class G4Event;

struct TrackerGunConfig {
    std::string particle    = "mu-";
    double      energy_MeV  = 1000.0;    // 1 GeV muon by default
    G4ThreeVector position  = {0, 0, -15.0};  // mm, just upstream
    G4ThreeVector direction = {0, 0,   1.0};  // along +Z
    double sigma_xy_mm      = 0.0;            // beam spread (0 = pencil beam)
    long   seed             = 0;
};

class TrackerPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    explicit TrackerPrimaryGeneratorAction(const TrackerGunConfig& cfg);
    ~TrackerPrimaryGeneratorAction() override;
    void GeneratePrimaries(G4Event*) override;

private:
    G4ParticleGun*   fGun;
    TrackerGunConfig fCfg;
};
