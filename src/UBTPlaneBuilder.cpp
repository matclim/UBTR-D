// ============================================================================
//  UBTPlaneBuilder.cpp
//
//  2×3 m² tracker plane. All coordinates in mm; origin at plane centre.
//
//  Layout (view along +Z / beam):
//
//    Y
//    ^
//    +1500 ┌──────────────────────────────────────────┐
//          │    LEFT half-plane  +  RIGHT half-plane   │  y=[+200,+1500]
//    +200  ├────────┬──────────────────┬──────────────┤
//          │ TILES  │  CENTRAL TUBES   │  TILES        │  y=[-200,+200]
//          │(left)  │  x=[-600,+600]   │  (right)      │
//    -200  ├────────┴──────────────────┴──────────────┤
//          │    LEFT half-plane  +  RIGHT half-plane   │  y=[-1500,-200]
//    -1500 └──────────────────────────────────────────┘
//          -1000 -200               +200          +1000  --> X
//
//  Outer bands (top + bottom):
//    Two independent half-planes of tubes, each a double-staggered layer.
//    Left  half-plane: x = [-1000, +200], tubes 1.2 m long centred at x = -400
//    Right half-plane: x = [  -200, +1000], tubes 1.2 m long centred at x = +400
//    Overlap region:   x = [-200, +200] — both half-planes present
//
//    Z positions (4 sub-layers total, each separated by one tube diameter = 5 mm):
//      Left  sub-layer 0:  z = -7.5 mm
//      Left  sub-layer 1:  z = -2.5 mm   (staggered by r=2.5 mm in Y)
//      Right sub-layer 0:  z = +2.5 mm
//      Right sub-layer 1:  z = +7.5 mm   (staggered by r=2.5 mm in Y)
//
//  Central strip y=[-200,+200]:
//    x=[-600,+600]: double-staggered tube layer (same z as central detector)
//    x=[-1000,-600]: 40×40 polystyrene tiles (1×1×1 cm³)
//    x=[+600,+1000]: 40×40 polystyrene tiles (1×1×1 cm³)
//
//  Tube spec: 5 mm outer diameter, 15 µm mylar wall, ArCO2 70/30 fill, 1.2 m long.
//  Tubes run along X. GeoTube default axis is Z → rotated 90° around Y.
// ============================================================================

#include "UBTPlaneBuilder.h"
#include "MaterialManager.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelKernel/GeoFullPhysVol.h"

#include <string>
#include <cmath>
#include <algorithm>
#include <stdexcept>

using namespace GeoModelKernelUnits;

// ============================================================================
//  Internal helpers
// ============================================================================
namespace {

// ----------------------------------------------------------------------------
//  makeEnvelope
//  Air box placed in `mother` at world position (xc, yc, zc).
//  All arguments already in GeoModel length units (mm applied).
// ----------------------------------------------------------------------------
GeoPhysVol* makeEnvelope(GeoVPhysVol*       mother,
                         GeoMaterial*       airMat,
                         const std::string& name,
                         double halfX, double halfY, double halfZ,
                         double xc,    double yc,    double zc)
{
    auto* lv = new GeoLogVol((name + "_LOG").c_str(),
                              new GeoBox(halfX, halfY, halfZ), airMat);
    auto* pv = new GeoPhysVol(lv);
    mother->add(new GeoNameTag(name.c_str()));
    mother->add(new GeoTransform(GeoTrf::Translate3D(xc, yc, zc)));
    mother->add(pv);
    return pv;
}

// ----------------------------------------------------------------------------
//  placeSingleTubeSubLayer
//
//  Places ONE sub-layer of horizontal tubes inside `envelope`.
//
//  Tubes run along X.  GeoTube's natural axis is Z, so we rotate the
//  tube by -90° around Y to align it with X.  The rotation is applied
//  ONLY to the tube placement transform; the gas volume inside the wall
//  inherits the same orientation automatically because it is a daughter
//  with no additional transform.
//
//  Parameters (all GeoModel units unless noted):
//    halfTubeLen  : physical half-length of tube along X (= 600 mm)
//    halfExtentY  : packing half-extent; tubes centred outside this are dropped
//    zLocal       : z position of tube centres inside the envelope
//    yShift       : offset of first tube centre from the symmetric bundle start
//                   (0 for sub-layer 0, r for sub-layer 1)
//    rOuter_mm    : outer radius [mm]
//    wallThick_mm : wall thickness [mm]
//    wallLog/gasLog: shared logical volumes (created once by caller)
//    tag          : name prefix
//    subIdx       : sub-layer index for naming (0 or 1)
// ----------------------------------------------------------------------------
void placeSingleTubeSubLayer(GeoVPhysVol*  envelope,
                             GeoLogVol*    wallLog,
                             GeoLogVol*    gasLog,
                             double        halfTubeLen,
                             double        halfExtentY,
                             double        zLocal,
                             double        yShift,
                             double        rOuter_mm,
                             const std::string& tag,
                             int           subIdx)
{
    const double rOuter = rOuter_mm * mm;
    const double pitch  = 2.0 * rOuter;

    // Number of tubes that fit with the given shift applied
    const double usable = 2.0 * halfExtentY - std::abs(yShift);
    const int nTubes = std::max(1, static_cast<int>(std::floor(usable / pitch)) + 1);

    // Centre the bundle symmetrically, then apply yShift
    const double yFirst = -0.5 * (nTubes - 1) * pitch + yShift;

    // Rotation: align tube axis (Z) with X by rotating -90° around Y
    const GeoTrf::Transform3D rotToX = GeoTrf::RotateY3D(-90.0 * deg);

    for (int i = 0; i < nTubes; ++i) {
        const double yPos = yFirst + i * pitch;

        // Drop tubes whose centre is outside the envelope Y range
        if (std::abs(yPos) > halfExtentY + 1e-6) continue;

        const std::string name =
            tag + "_S" + std::to_string(subIdx) + "_T" + std::to_string(i);

        const GeoTrf::Transform3D tubeTrf =
            GeoTrf::Translate3D(0.0, yPos, zLocal) * rotToX;

        // Place wall and gas as SIBLINGS in the envelope (not nested).
        // Nesting gas inside wall causes Geo2G4 to assign garbage copy numbers
        // and Geant4's navigator cannot find the gas volume reliably.
        // Both shapes are co-centred; the hollow wall [rInner,rOuter] and the
        // solid gas [0,rInner] together fill the full tube cross-section.

        // Wall
        auto* wallPV = new GeoPhysVol(wallLog);
        envelope->add(new GeoNameTag((name + "_Wall").c_str()));
        envelope->add(new GeoTransform(tubeTrf));
        envelope->add(wallPV);

        // Gas — same transform, placed after wall so navigator prefers it
        auto* gasPV = new GeoPhysVol(gasLog);
        envelope->add(new GeoNameTag((name + "_Gas").c_str()));
        envelope->add(new GeoTransform(tubeTrf));
        envelope->add(gasPV);
    }
}

// ----------------------------------------------------------------------------
//  placeDoubleStaggeredLayer
//
//  Places two sub-layers (double-staggered) inside `envelope`.
//  Sub-layer 0: zLocal = zCenter - r,  no Y shift
//  Sub-layer 1: zLocal = zCenter + r,  Y shift = +r (half pitch)
//
//  Creates one pair of shared LVs (wall + gas) for this region.
// ----------------------------------------------------------------------------
void placeDoubleStaggeredLayer(GeoVPhysVol*       envelope,
                               GeoMaterial*       mylarMat,
                               GeoMaterial*       gasMat,
                               double             halfTubeLen,   // GeoModel units
                               double             halfExtentY,   // GeoModel units
                               double             zCenter,       // GeoModel units, local to envelope
                               double             rOuter_mm,
                               double             wallThick_mm,
                               const std::string& tag)
{
    const double rOuter = rOuter_mm             * mm;
    const double rInner = (rOuter_mm - wallThick_mm) * mm;

    if (rInner <= 0.0 || rInner >= rOuter)
        throw std::runtime_error("placeDoubleStaggeredLayer: invalid wall for " + tag);

    // Shared LVs — one wall shape and one gas shape for the whole layer
    auto* wallLog = new GeoLogVol(
        (tag + "_TubeWall_LV").c_str(),
        new GeoTube(rInner, rOuter, halfTubeLen),
        mylarMat);
    // Gas radius is shrunk by 1 µm relative to wall inner radius to give
    // Geant4's navigator a clean gap — avoids shared-surface ambiguity.
    const double rGas = rInner - 0.001 * mm;
    auto* gasLog = new GeoLogVol(
        (tag + "_TubeGas_LV").c_str(),
        new GeoTube(0.0, rGas, halfTubeLen),
        gasMat);

    // Sub-layer 0: z = zCenter - r,  yShift = 0
    placeSingleTubeSubLayer(envelope, wallLog, gasLog,
                            halfTubeLen, halfExtentY,
                            zCenter - rOuter, /*yShift=*/0.0,
                            rOuter_mm, tag, 0);

    // Sub-layer 1: z = zCenter + r,  yShift = r (half pitch)
    placeSingleTubeSubLayer(envelope, wallLog, gasLog,
                            halfTubeLen, halfExtentY,
                            zCenter + rOuter, /*yShift=*/rOuter,
                            rOuter_mm, tag, 1);
}

} // anonymous namespace


// ============================================================================
//  UBTPlaneBuilder::build
// ============================================================================
void UBTPlaneBuilder::build(GeoVPhysVol*      mother,
                                MaterialManager&  MM,
                                double            zOffset_mm,
                                const std::string& tag)
{
    // ------------------------------------------------------------------
    //  Geometry constants (all in mm)
    // ------------------------------------------------------------------
    const double r_mm       = 2.5;      // tube outer radius
    const double wall_mm    = 0.015;    // mylar wall thickness
    const double halfLen_mm = 600.0;    // tube physical half-length (1.2 m)
    const double r          = r_mm * mm;

    // Outer Y bands: y=[+200,+1500] top, y=[-1500,-200] bottom
    const double outerHalfY_mm = 650.0;   // (1500-200)/2
    const double outerCtrY_mm  = 850.0;   // (200+1500)/2

    // Left half-plane:  x=[-1000,+200]  → halfX=600, centreX=-400
    const double leftHalfX_mm  = 600.0;
    const double leftCtrX_mm   = -400.0;

    // Right half-plane: x=[-200,+1000]  → halfX=600, centreX=+400
    const double rightHalfX_mm = 600.0;
    const double rightCtrX_mm  = +400.0;

    // Z centres of the two half-planes (one tube-diameter = 5 mm apart)
    // Left:  z = -5 mm  → sub-layers at -7.5 and -2.5 mm
    // Right: z = +5 mm  → sub-layers at +2.5 and +7.5 mm
    const double zLeft_mm  = -2.0 * r_mm;   // = -5 mm
    const double zRight_mm = +2.0 * r_mm;   // = +5 mm

    // Envelope Z half-extent: must contain the outermost sub-layer centre ± r
    // Outermost is at |zLeft_mm| + r = 5+2.5 = 7.5 mm → halfZ = 8 mm (small clearance)
    const double outerEnvHalfZ_mm = 3.0 * r_mm + 0.5;  // = 8 mm

    // Central strip: x=[-600,+600], y=[-200,+200]
    const double ctrTubeHalfX_mm = 600.0;
    const double ctrTubeHalfY_mm = 200.0;

    // Central strip tube z: use z=0 (single double-staggered layer)
    const double zCtr_mm = 0.0;
    const double ctrEnvHalfZ_mm = r_mm + 0.5;  // = 3 mm

    // Tile blocks: x=[-1000,-600] and x=[+600,+1000], y=[-200,+200]
    const double tileBlockHalfX_mm = 200.0;
    const double tileBlockCtrX_mm  = 800.0;
    const double tileBlockHalfY_mm = 200.0;
    const double tileSide_mm       =  10.0;
    const double tileHalfZ_mm      =   5.0;  // half of 10 mm

    // ------------------------------------------------------------------
    //  Materials
    // ------------------------------------------------------------------
    auto* airMat   = MM.air();
    auto* mylarMat = MM.mylar();
    auto* gasMat   = MM.arco2();
    auto* psMat    = MM.polystyrene();

    const double zOff          = zOffset_mm         * mm;
    const double outerHalfY    = outerHalfY_mm      * mm;
    const double outerEnvHalfZ = outerEnvHalfZ_mm   * mm;

    // ------------------------------------------------------------------
    //  Helper: build one Y-band (top or bottom) with left+right half-planes
    // ------------------------------------------------------------------
    auto buildOuterBand = [&](const std::string& bandTag, double yCtr_mm)
    {
        const double yCtr = yCtr_mm * mm;

        // ---- Left half-plane envelope ----
        {
            const double envHalfX = leftHalfX_mm * mm;
            const double envCtrX  = leftCtrX_mm  * mm;
            // Envelope Z is centred on zLeft; must contain sub-layers at zLeft ± r
            const double envZCtr  = zOff + zLeft_mm * mm;

            auto* env = makeEnvelope(mother, airMat,
                                     bandTag + "_Left",
                                     envHalfX, outerHalfY, outerEnvHalfZ,
                                     envCtrX, yCtr, envZCtr);

            // zCenter local to envelope = 0 (envelope centred on zLeft already)
            placeDoubleStaggeredLayer(env, mylarMat, gasMat,
                                      halfLen_mm * mm,
                                      outerHalfY,
                                      /*zCenter local=*/0.0,
                                      r_mm, wall_mm,
                                      bandTag + "_Left");
        }

        // ---- Right half-plane envelope ----
        {
            const double envHalfX = rightHalfX_mm * mm;
            const double envCtrX  = rightCtrX_mm  * mm;
            const double envZCtr  = zOff + zRight_mm * mm;

            auto* env = makeEnvelope(mother, airMat,
                                     bandTag + "_Right",
                                     envHalfX, outerHalfY, outerEnvHalfZ,
                                     envCtrX, yCtr, envZCtr);

            placeDoubleStaggeredLayer(env, mylarMat, gasMat,
                                      halfLen_mm * mm,
                                      outerHalfY,
                                      /*zCenter local=*/0.0,
                                      r_mm, wall_mm,
                                      bandTag + "_Right");
        }
    };

    // ------------------------------------------------------------------
    //  (A)  OUTER TUBE BANDS
    // ------------------------------------------------------------------
    buildOuterBand(tag + "_Top",    +outerCtrY_mm);   // y=[+200,+1500]
    buildOuterBand(tag + "_Bottom", -outerCtrY_mm);   // y=[-1500,-200]

    // ------------------------------------------------------------------
    //  (B)  CENTRAL TUBE REGION  x=[-600,+600], y=[-200,+200]
    // ------------------------------------------------------------------
    {
        const double ctrHalfX    = ctrTubeHalfX_mm * mm;
        const double ctrHalfY    = ctrTubeHalfY_mm * mm;
        const double ctrEnvHalfZ = ctrEnvHalfZ_mm  * mm;
        const double envZCtr     = zOff + zCtr_mm   * mm;

        auto* env = makeEnvelope(mother, airMat,
                                 tag + "_CentralTubes",
                                 ctrHalfX, ctrHalfY, ctrEnvHalfZ,
                                 0.0, 0.0, envZCtr);

        placeDoubleStaggeredLayer(env, mylarMat, gasMat,
                                  halfLen_mm * mm,
                                  ctrHalfY,
                                  /*zCenter local=*/0.0,
                                  r_mm, wall_mm,
                                  tag + "_CentralTubes");
    }

    // ------------------------------------------------------------------
    //  (C)  TILE BLOCKS  x=[-1000,-600] and x=[+600,+1000], y=[-200,+200]
    //       40×40 = 1600 tiles per block, coplanar at z = zOff
    // ------------------------------------------------------------------
    const double tileBlockHalfX = tileBlockHalfX_mm * mm;
    const double tileBlockHalfY = tileBlockHalfY_mm * mm;
    const double tileHalfZ      = tileHalfZ_mm      * mm;

    const int nTileX = static_cast<int>(
        std::round(2.0 * tileBlockHalfX_mm / tileSide_mm));  // 40
    const int nTileY = static_cast<int>(
        std::round(2.0 * tileBlockHalfY_mm / tileSide_mm));  // 40

    // Each tile gets its own uniquely-named LV to prevent Geo2G4 from
    // collapsing all instances into copy number 16969 (shared-LV bug).
    // GeoFullPhysVol is used so the SD can register it as sensitive.
    // Shrink tile by 5 µm per side to leave a gap between adjacent tiles.
    // Zero-gap touching faces confuse Geant4's navigator (GeomNav1002).
    const double tileGap = 0.005 * mm;
    auto* tileShape = new GeoBox(0.5 * tileSide_mm * mm - tileGap,
                                 0.5 * tileSide_mm * mm - tileGap,
                                 tileHalfZ - tileGap);

    auto placeTileBlock = [&](GeoVPhysVol* env, const std::string& btag) {
        const double x0 = -0.5 * (nTileX - 1) * tileSide_mm * mm;
        const double y0 = -0.5 * (nTileY - 1) * tileSide_mm * mm;
        for (int ix = 0; ix < nTileX; ++ix) {
            for (int iy = 0; iy < nTileY; ++iy) {
                const std::string tname =
                    btag + "_T" + std::to_string(ix) + "_" + std::to_string(iy);
                // Unique LV name per tile → unique copy number from Geo2G4
                auto* lv = new GeoLogVol((tname + "_LV").c_str(), tileShape, psMat);
                env->add(new GeoNameTag(tname.c_str()));
                env->add(new GeoTransform(GeoTrf::Translate3D(
                    x0 + ix * tileSide_mm * mm,
                    y0 + iy * tileSide_mm * mm,
                    0.0)));
                env->add(new GeoFullPhysVol(lv));
            }
        }
    };

    // Envelope gets 0.1 mm margin in Z so tiles don't touch the envelope face
    // (touching faces cause GeomNav1002 stuck-track warnings).
    const double tileEnvHalfZ = tileHalfZ + 0.1 * mm;

    // ---- Left tile block  x=[-1000,-600] ----
    {
        auto* env = makeEnvelope(mother, airMat,
                                 tag + "_TileLeft",
                                 tileBlockHalfX, tileBlockHalfY, tileEnvHalfZ,
                                 -tileBlockCtrX_mm * mm, 0.0, zOff);
        placeTileBlock(env, tag + "_TileLeft");
    }

    // ---- Right tile block  x=[+600,+1000] ----
    {
        auto* env = makeEnvelope(mother, airMat,
                                 tag + "_TileRight",
                                 tileBlockHalfX, tileBlockHalfY, tileEnvHalfZ,
                                 +tileBlockCtrX_mm * mm, 0.0, zOff);
        placeTileBlock(env, tag + "_TileRight");
    }
}
