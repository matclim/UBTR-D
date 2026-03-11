#include "UBTPrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "Randomize.hh"

UBTPrimaryGeneratorAction::UBTPrimaryGeneratorAction(
        const UBTGunConfig& cfg)
    : fCfg(cfg)
{
    fGun = new G4ParticleGun(1);

    auto* pt  = G4ParticleTable::GetParticleTable();
    auto* def = pt->FindParticle(cfg.particle);
    if (!def) {
        G4cerr << "[UBTPrimaryGeneratorAction] Unknown particle '"
               << cfg.particle << "', falling back to mu-\n";
        def = pt->FindParticle("mu-");
    }

    fGun->SetParticleDefinition(def);
    fGun->SetParticleEnergy(cfg.energy_MeV * MeV);
    fGun->SetParticleMomentumDirection(cfg.direction);
    fGun->SetParticlePosition(cfg.position * mm);
}

UBTPrimaryGeneratorAction::~UBTPrimaryGeneratorAction()
{
    delete fGun;
}

void UBTPrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    G4ThreeVector pos = fCfg.position * mm;

    if (fCfg.sigma_xy_mm > 0.0) {
        const double sig = fCfg.sigma_xy_mm * mm;
        pos.setX(pos.x() + G4RandGauss::shoot(0.0, sig));
        pos.setY(pos.y() + G4RandGauss::shoot(0.0, sig));
    }

    fGun->SetParticlePosition(pos);
    fGun->GeneratePrimaryVertex(event);
}
