#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "UBTDetectorConstruction.hh"
#include "UBTPrimaryGeneratorAction.hh"
#include "UBTRunAction.hh"
#include "UBTEventAction.hh"

#include <string>
#include <iostream>
#include <cstdlib>

// ============================================================================
//  main
//
//  Usage:
//    ./run_ubt [options]
//
//  Geometry / output:
//    --output <file>         ROOT output filename          (default: ubt_hits.root)
//    --write-gdml            Export geometry to GDML
//    --gdml-out <file>       GDML output filename          (default: ubt_geometry.gdml)
//
//  Run control:
//    --n-events <N>          Number of events to simulate  (default: 0 = vis only)
//    --seed <N>              Random seed
//
//  Primary particle:
//    --particle <name>       Geant4 particle name          (default: mu-)
//    --energy-MeV <E>        Kinetic energy [MeV]          (default: 1000)
//    --pos-mm <x> <y> <z>    Gun position [mm]             (default: 0 0 -15)
//    --dir <x> <y> <z>       Direction (unit vector)       (default: 0 0 1)
//    --sigma-xy-mm <s>       Gaussian beam spread [mm]     (default: 0)
//
//  Visualisation:
//    --visualize             Open interactive viewer
//    --vis-macro <file>      Vis macro                     (default: ubt_vis.mac)
//    --vis-mode <0|1|2>      0=envelopes 1=by-region 2=full detail
// ============================================================================

int main(int argc, char** argv)
{
    // ---- Defaults -----------------------------------------------------------
    std::string outFile    = "ubt_hits.root";
    bool        writeGdml  = false;
    std::string gdmlOut    = "ubt_geometry.gdml";
    int         nEvents    = 0;
    long        seed       = 0;
    bool        doVis      = false;
    std::string visMacro   = "ubt_vis.mac";
    int         visMode    = 1;

    UBTGunConfig gun;   // particle, energy, pos, dir, sigma

    // ---- Argument parsing ---------------------------------------------------
    auto require = [&](int& i, const char* opt) -> const char* {
        if (i + 1 >= argc) {
            std::cerr << "Missing value after " << opt << "\n"; std::exit(2);
        }
        return argv[++i];
    };
    auto require3 = [&](int& i, const char* opt, double& a, double& b, double& c) {
        if (i + 3 >= argc) {
            std::cerr << "Missing 3 values after " << opt << "\n"; std::exit(2);
        }
        a = std::stod(argv[++i]);
        b = std::stod(argv[++i]);
        c = std::stod(argv[++i]);
    };

    for (int i = 1; i < argc; ++i) {
        const std::string opt = argv[i];
        if      (opt == "--output")       outFile          = require(i, "--output");
        else if (opt == "--write-gdml")   writeGdml        = true;
        else if (opt == "--gdml-out")     gdmlOut          = require(i, "--gdml-out");
        else if (opt == "--n-events")     nEvents          = std::stoi(require(i, "--n-events"));
        else if (opt == "--seed")         seed             = std::stol(require(i, "--seed"));
        else if (opt == "--particle")     gun.particle     = require(i, "--particle");
        else if (opt == "--energy-MeV")   gun.energy_MeV   = std::stod(require(i, "--energy-MeV"));
        else if (opt == "--sigma-xy-mm")  gun.sigma_xy_mm  = std::stod(require(i, "--sigma-xy-mm"));
        else if (opt == "--visualize")    doVis            = true;
        else if (opt == "--vis-macro")    visMacro         = require(i, "--vis-macro");
        else if (opt == "--vis-mode")     visMode          = std::stoi(require(i, "--vis-mode"));
        else if (opt == "--pos-mm") {
            double x, y, z;
            require3(i, "--pos-mm", x, y, z);
            gun.position = {x, y, z};
        }
        else if (opt == "--dir") {
            double x, y, z;
            require3(i, "--dir", x, y, z);
            gun.direction = {x, y, z};
        }
        else {
            std::cerr << "Unknown option: " << opt << "\n"
                      << "Run ./run_ubt --help for usage.\n";
            std::exit(2);
        }
    }

    if (seed != 0) CLHEP::HepRandom::setTheSeed(seed);
    if (writeGdml) setenv("G4_GDML_OUT", gdmlOut.c_str(), 1);

    // ---- Geant4 setup -------------------------------------------------------
    auto* runManager = new G4RunManager;

    // Detector
    auto* detCon = new UBTDetectorConstruction(writeGdml);
    detCon->SetVisMode(visMode);
    detCon->SetRegisterSD(nEvents > 0);   // only register SD when actually simulating
    runManager->SetUserInitialization(detCon);

    // Physics
    G4PhysListFactory factory;
    runManager->SetUserInitialization(factory.GetReferencePhysList("FTFP_BERT"));

    // User actions
    // EventStore is owned by detCon; wire it into EventAction before Initialize()
    auto* runAction   = new UBTRunAction(outFile);
    auto* eventAction = new UBTEventAction(detCon->GetStore());
    runManager->SetUserAction(new UBTPrimaryGeneratorAction(gun));
    runManager->SetUserAction(runAction);
    runManager->SetUserAction(eventAction);

    runManager->Initialize();

    // ---- Visualisation ------------------------------------------------------
    G4VisExecutive* vis = nullptr;
    if (doVis) {
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

    // ---- Run simulation -----------------------------------------------------
    if (nEvents > 0) {
        auto* UI = G4UImanager::GetUIpointer();
        UI->ApplyCommand("/run/beamOn " + std::to_string(nEvents));
    }

    delete runManager;
    return 0;
}
