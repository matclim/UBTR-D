// ============================================================================
//  MaterialManager_TrackerAdditions.cpp
//
//  Paste these two method bodies into your MaterialManager class.
//  Corresponding declarations to add to MaterialManager.h:
//
//      GeoMaterial* mylar();     // Mylar (PET) C10H8O4, rho=1.39 g/cm3
//      GeoMaterial* arco2();     // ArCO2 70/30 by mass, rho=0.00183 g/cm3
//
// ============================================================================

// ---- Declaration additions for MaterialManager.h ----
//
//  GeoMaterial* mylar();
//  GeoMaterial* arco2();
//
// Place these alongside your existing declarations such as air(), lead(), etc.


// ---- Implementation to add to MaterialManager.cpp ----

#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/Units.h"

using namespace GeoModelKernelUnits;

// ---------------------------------------------------------------------------
//  Mylar  (biaxially-oriented PET: C10H8O4)
//  Density: 1.39 g/cm³
// ---------------------------------------------------------------------------
GeoMaterial* MaterialManager::mylar()
{
    static GeoMaterial* mat = nullptr;
    if (mat) return mat;

    // Elements: C, H, O  (standard atomic weights)
    auto* elC = new GeoElement("Carbon",   "C",  6,  12.011  * g/mole);
    auto* elH = new GeoElement("Hydrogen", "H",  1,   1.008  * g/mole);
    auto* elO = new GeoElement("Oxygen",   "O",  8,  15.999  * g/mole);

    // Molecular formula C10H8O4  →  molar mass = 192.17 g/mol
    // Mass fractions:
    //   C: 10*12.011 / 192.17 = 0.6250
    //   H:  8* 1.008 / 192.17 = 0.0420
    //   O:  4*15.999 / 192.17 = 0.3330
    mat = new GeoMaterial("Mylar", 1.39 * g/cm3);
    mat->add(elC, 0.6250);
    mat->add(elH, 0.0420);
    mat->add(elO, 0.3330);
    mat->lock();

    return mat;
}

// ---------------------------------------------------------------------------
//  ArCO2 70/30 counting gas
//  Component densities at STP:
//    Ar  : 1.784e-3 g/cm³
//    CO2 : 1.977e-3 g/cm³
//  Mixture density (70% Ar + 30% CO2 by volume ≈ by mass for gases):
//    rho = 0.70 * 1.784e-3 + 0.30 * 1.977e-3 = 1.842e-3 g/cm³
// ---------------------------------------------------------------------------
GeoMaterial* MaterialManager::arco2()
{
    static GeoMaterial* mat = nullptr;
    if (mat) return mat;

    auto* elAr = new GeoElement("Argon",   "Ar", 18, 39.948 * g/mole);
    auto* elC  = new GeoElement("Carbon",  "C",   6, 12.011 * g/mole);
    auto* elO  = new GeoElement("Oxygen",  "O",   8, 15.999 * g/mole);

    // Mass fractions of the 70/30 mixture:
    //   Ar  : 0.70 * 39.948 / (0.70*39.948 + 0.30*(12.011+2*15.999))
    //       = 27.964 / (27.964 + 13.202) = 0.6794
    //   CO2 : 0.30 * 44.009 / 41.166     = 0.3206  →  split into C + O2
    //     C fraction in mixture : 0.3206 * 12.011/44.009 = 0.08744
    //     O fraction in mixture : 0.3206 * 31.998/44.009 = 0.23316
    const double rho = (0.70 * 1.784e-3 + 0.30 * 1.977e-3) * g/cm3;

    mat = new GeoMaterial("ArCO2_70_30", rho);
    mat->add(elAr, 0.6794);
    mat->add(elC,  0.0874);
    mat->add(elO,  0.2332);
    mat->lock();

    return mat;
}
