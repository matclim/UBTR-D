#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4SystemOfUnits.hh"

#include "TrackerDetectorConstruction.hh"

// Minimal primary generator (required by Geant4 even when firing zero events)
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"

#include <string>
#include <iostream>
#include <cstdlib>

// ============================================================================
//  NullPrimaryGenerator
//  Fires a single geantino that misses the detector; satisfies the Geant4
//  requirement for a registered primary generator without affecting geometry.
// ============================================================================
class NullPrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
    NullPrimaryGenerator() {
        m_gun = new G4ParticleGun(1);
        auto* pt = G4ParticleTable::GetParticleTable();
        m_gun->SetParticleDefinition(pt->FindParticle("geantino"));
        m_gun->SetParticleEnergy(1.0 * GeV);
        m_gun->SetParticleMomentumDirection({0, 0, 1});
        m_gun->SetParticlePosition({0, 0, -10000.0 * mm});
    }
    ~NullPrimaryGenerator() override { delete m_gun; }
    void GeneratePrimaries(G4Event* ev) override { m_gun->GeneratePrimaryVertex(ev); }
private:
    G4ParticleGun* m_gun;
};

// ============================================================================
//  main
//
//  Usage:
//    ./run_tracker [options]
//
//  Options:
//    --write-gdml            Export geometry to GDML (default filename: tracker_geometry.gdml)
//    --gdml-out <path>       Override GDML output filename
//    --visualize             Open interactive viewer
//    --vis-macro <path>      Vis macro to execute (default: tracker_vis.mac)
//    --vis-mode <N>          0=envelopes only  1=colour by region (default)  2=full detail
// ============================================================================
int main(int argc, char** argv)
{
    bool        writeGdml   = false;
    bool        doVisualize = false;
    int         visMode     = 1;
    std::string visMacro    = "tracker_vis.mac";
    std::string gdmlOut     = "tracker_geometry.gdml";

    auto require = [&](int& i, const char* opt) -> const char* {
        if (i + 1 >= argc) {
            std::cerr << "Missing value after " << opt << "\n";
            std::exit(2);
        }
        return argv[++i];
    };

    for (int i = 1; i < argc; ++i) {
        const std::string opt = argv[i];
        if      (opt == "--write-gdml") { writeGdml   = true; }
        else if (opt == "--visualize")  { doVisualize = true; }
        else if (opt == "--vis-mode")   { visMode  = std::stoi(require(i, "--vis-mode")); }
        else if (opt == "--vis-macro")  { visMacro = require(i, "--vis-macro"); }
        else if (opt == "--gdml-out")   { gdmlOut  = require(i, "--gdml-out"); }
        else {
            std::cerr << "Unknown option: " << opt << "\n"
                      << "Usage: run_tracker [--write-gdml] [--gdml-out path]\n"
                      << "                   [--visualize] [--vis-macro path]\n"
                      << "                   [--vis-mode 0|1|2]\n";
            std::exit(2);
        }
    }

    // Propagate GDML path via environment (picked up in TrackerDetectorConstruction)
    if (writeGdml) {
        setenv("G4_GDML_OUT", gdmlOut.c_str(), /*overwrite=*/1);
    }

    // --- Geant4 setup --------------------------------------------------------
    auto* runManager = new G4RunManager;

    auto* detCon = new TrackerDetectorConstruction(writeGdml);
    detCon->SetVisMode(visMode);
    runManager->SetUserInitialization(detCon);

    G4PhysListFactory factory;
    runManager->SetUserInitialization(factory.GetReferencePhysList("FTFP_BERT"));
    runManager->SetUserAction(new NullPrimaryGenerator);

    runManager->Initialize();

    // --- Visualisation -------------------------------------------------------
    G4VisExecutive* vis = nullptr;
    if (doVisualize) {
        vis = new G4VisExecutive;
        vis->Initialize();

        int   ui_argc   = 1;
        char* ui_argv[] = { argv[0] };
        auto* app = new G4UIExecutive(ui_argc, ui_argv);

        auto* UI = G4UImanager::GetUIpointer();
        UI->ApplyCommand("/control/execute " + G4String(visMacro));
        app->SessionStart();

        delete app;
        delete vis;
    }

    delete runManager;
    return 0;
}
