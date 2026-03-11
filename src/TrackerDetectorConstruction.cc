#include "TrackerDetectorConstruction.hh"
#include "TrackerPlaneBuilder.h"
#include "MaterialManager.h"

// GeoModel kernel
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"

// GeoModel → Geant4
#include "GeoModel2G4/ExtParameterisedVolumeBuilder.h"

// Geant4
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"

// GDML export
#include "G4GDMLParser.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"

using namespace GeoModelKernelUnits;

// ============================================================================

TrackerDetectorConstruction::TrackerDetectorConstruction(bool writeGdml)
    : m_writeGdml(writeGdml)
{}

// ----------------------------------------------------------------------------
//  buildGeoModelWorld
//  Constructs a minimal GeoModel tree:
//    World (air box, 4000 × 3000 × 500 mm)
//      └─ TrackerPlane (built by TrackerPlaneBuilder)
// ----------------------------------------------------------------------------
GeoPhysVol* TrackerDetectorConstruction::buildGeoModelWorld()
{
    MaterialManager MM;

    // World box — generous margins around the 3000×2000 mm plane
    const double worldHalfX = 1200.0 * GeoModelKernelUnits::mm;   // detector ±1000 mm in X + margin
    const double worldHalfY = 1700.0 * GeoModelKernelUnits::mm;   // detector ±1500 mm in Y + margin
    const double worldHalfZ =   20.0 * GeoModelKernelUnits::mm;   // stack is ±8 mm; 20 mm is generous

    auto* worldShape = new GeoBox(worldHalfX, worldHalfY, worldHalfZ);
    auto* worldLog   = new GeoLogVol("TrackerWorldLog", worldShape, MM.air());
    auto* worldPhys  = new GeoPhysVol(worldLog);

    // Place the tracker plane at z = 0 (centre of world)
    TrackerPlaneBuilder::build(worldPhys, MM, /*zOffset_mm=*/0.0, /*tag=*/"TRK");

    G4cout << "[TrackerDetectorConstruction] GeoModel world children = "
           << worldPhys->getNChildVols() << G4endl;

    return worldPhys;
}

// ----------------------------------------------------------------------------
//  Construct  (called by Geant4 run manager)
// ----------------------------------------------------------------------------
G4VPhysicalVolume* TrackerDetectorConstruction::Construct()
{
    // 1. Build GeoModel tree
    GeoPhysVol* geoWorld = buildGeoModelWorld();

    // 2. Convert to Geant4 via GeoModel2G4
    ExtParameterisedVolumeBuilder vb("TrackerWorld");
    G4LogicalVolume* g4WorldLV = vb.Build(static_cast<PVConstLink>(geoWorld));

    if (!g4WorldLV) {
        G4Exception("TrackerDetectorConstruction::Construct",
                    "Geo2G4BuildFailed", FatalException,
                    "ExtParameterisedVolumeBuilder::Build returned null LV.");
    }

    auto* pvWorld = new G4PVPlacement(nullptr, {}, g4WorldLV,
                                      "TrackerWorldPV", nullptr, false, 0);

    G4cout << "[TrackerDetectorConstruction] G4 world daughters = "
           << g4WorldLV->GetNoDaughters()
           << "  LVStore size = "
           << G4LogicalVolumeStore::GetInstance()->size() << G4endl;

    // 3. Visualisation attributes
    auto* store = G4LogicalVolumeStore::GetInstance();

    auto makeVis = [](double r, double g, double b, double a,
                      bool solid = true) -> G4VisAttributes* {
        auto* v = new G4VisAttributes(G4Colour(r, g, b, a));
        v->SetVisibility(true);
        solid ? v->SetForceSolid(true) : v->SetForceWireframe(true);
        return v;
    };

    auto* visHide   = new G4VisAttributes(false);
    auto* visWorld  = makeVis(1, 1, 1, 0.03, /*solid=*/false);
    auto* visEnvOuter = makeVis(0.2, 0.6, 1.0, 0.08, false);  // blue wireframe
    auto* visEnvCtr   = makeVis(0.0, 0.9, 0.5, 0.08, false);  // green wireframe
    auto* visEnvTile  = makeVis(1.0, 0.6, 0.0, 0.08, false);  // orange wireframe

    // Tube volumes
    auto* visWall  = makeVis(0.85, 0.85, 0.85, 0.6);   // light grey — mylar
    auto* visGas   = makeVis(0.3,  0.7,  1.0,  0.5);   // blue       — ArCO2
    // Tile volumes
    auto* visTile  = makeVis(0.2,  0.85, 0.2,  0.8);   // green      — polystyrene

    for (auto* lv : *store) {
        const std::string& n = lv->GetName();

        if (n.find("TrackerWorldLog") != std::string::npos || n == "World") {
            lv->SetVisAttributes(visWorld);
        }
        // Region envelopes  (_LOG suffix added by makeEnvelope)
        else if (n.find("TRK_OuterLeft_LOG")    != std::string::npos ||
                 n.find("TRK_OuterRight_LOG")   != std::string::npos) {
            if (m_visMode == 0) lv->SetVisAttributes(visHide);
            else                lv->SetVisAttributes(visEnvOuter);
        }
        else if (n.find("TRK_CentralTubes_LOG") != std::string::npos) {
            if (m_visMode == 0) lv->SetVisAttributes(visHide);
            else                lv->SetVisAttributes(visEnvCtr);
        }
        else if (n.find("TRK_TileLeft_LOG")   != std::string::npos ||
                 n.find("TRK_TileRight_LOG")  != std::string::npos) {
            if (m_visMode == 0) lv->SetVisAttributes(visHide);
            else                lv->SetVisAttributes(visEnvTile);
        }
        // Active volumes
        else if (n.find("TubeWall_LV") != std::string::npos) {
            lv->SetVisAttributes(m_visMode >= 1 ? visWall : visHide);
        }
        else if (n.find("TubeGas_LV")  != std::string::npos) {
            lv->SetVisAttributes(m_visMode >= 1 ? visGas  : visHide);
        }
        else if (n.find("Tile_LV")     != std::string::npos) {
            lv->SetVisAttributes(m_visMode >= 1 ? visTile : visHide);
        }
        else {
            lv->SetVisAttributes(visHide);
        }
    }

    // 4. Optional GDML export (master thread only)
    if (m_writeGdml) {
        static G4Mutex   gdmlMutex   = G4MUTEX_INITIALIZER;
        static bool      gdmlWritten = false;
        G4AutoLock lock(&gdmlMutex);
        if (!gdmlWritten && G4Threading::IsMasterThread()) {
            const char* out     = std::getenv("G4_GDML_OUT");
            std::string outName = out ? out : "tracker_geometry.gdml";
            G4GDMLParser parser;
            parser.SetStripFlag(false);
            parser.Write(outName, pvWorld, /*storeReferences=*/true);
            G4cout << "[TrackerDetectorConstruction] GDML written to: "
                   << outName << G4endl;
            gdmlWritten = true;
        }
    }

    return pvWorld;
}
