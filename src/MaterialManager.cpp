#include "MaterialManager.h"

#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/Units.h"

using namespace GeoModelKernelUnits;

GeoMaterial* MaterialManager::getCached(const std::string& key) const {
  auto it = m_mats.find(key);
  return (it == m_mats.end()) ? nullptr : it->second;
}

void MaterialManager::cache(const std::string& key, GeoMaterial* mat) {
  m_mats[key] = mat;
}

GeoMaterial* MaterialManager::air() {
  if (auto* m = getCached("Air")) return m;

  auto* N = new GeoElement("AirElement", "Air", 7.0, 14.01 * g/mole);
  auto* m = new GeoMaterial("Air", 1.2041 * mg/cm3);
  m->add(N, 1.0);
  m->lock();
  cache("Air", m);
  return m;
}

GeoMaterial* MaterialManager::lead() {
  if (auto* m = getCached("Lead")) return m;

  auto* Pb = new GeoElement("LeadElement", "Pb", 82.0, 207.2 * g/mole);
  auto* m  = new GeoMaterial("Lead", 11.34 * g/cm3);
  m->add(Pb, 1.0);
  m->lock();
  cache("Lead", m);
  return m;
}

GeoMaterial* MaterialManager::pvt() {
  if (auto* m = getCached("PVT")) return m;

  auto* C = new GeoElement("Carbon",   "C", 6.0, 12.011 * g/mole);
  auto* H = new GeoElement("Hydrogen", "H", 1.0,  1.008 * g/mole);
  auto* m = new GeoMaterial("PVT", 1.032 * g/cm3);
  m->add(C,  9.0);
  m->add(H, 10.0);
  m->lock();
  cache("PVT", m);
  return m;
}

GeoMaterial* MaterialManager::polystyrene() {
  if (auto* m = getCached("Polystyrene")) return m;

  auto* C = new GeoElement("Carbon",   "C", 6.0, 12.011 * g/mole);
  auto* H = new GeoElement("Hydrogen", "H", 1.0,  1.008 * g/mole);
  auto* m = new GeoMaterial("Polystyrene", 1.05 * g/cm3);
  m->add(C, 8.0);
  m->add(H, 8.0);
  m->lock();
  cache("Polystyrene", m);
  return m;
}

GeoMaterial* MaterialManager::aluminum() {
  if (auto* m = getCached("Aluminium")) return m;

  auto* Al = new GeoElement("Aluminium", "Al", 13.0, 26.9815 * g/mole);
  auto* m  = new GeoMaterial("Aluminium", 2.70 * g/cm3);
  m->add(Al, 1.0);
  m->lock();
  cache("Aluminium", m);
  return m;
}

GeoMaterial* MaterialManager::iron() {
  if (auto* m = getCached("Iron")) return m;

  auto* Fe = new GeoElement("Iron", "Fe", 26.0, 55.845 * g/mole);
  auto* m  = new GeoMaterial("Iron", 7.874 * g/cm3);
  m->add(Fe, 1.0);
  m->lock();
  cache("Iron", m);
  return m;
}

// ----------------------------------------------------------------------------
//  Mylar  (biaxially-oriented PET: C10H8O4, density 1.39 g/cm³)
//  Molecular mass: 10*12.011 + 8*1.008 + 4*15.999 = 192.17 g/mol
//  Mass fractions:  C = 0.6250,  H = 0.0420,  O = 0.3330
// ----------------------------------------------------------------------------
GeoMaterial* MaterialManager::mylar() {
  if (auto* m = getCached("Mylar")) return m;

  auto* C = new GeoElement("Carbon",   "C", 6.0,  12.011 * g/mole);
  auto* H = new GeoElement("Hydrogen", "H", 1.0,   1.008 * g/mole);
  auto* O = new GeoElement("Oxygen",   "O", 8.0,  15.999 * g/mole);
  auto* m = new GeoMaterial("Mylar", 1.39 * g/cm3);
  m->add(C, 0.6250);
  m->add(H, 0.0420);
  m->add(O, 0.3330);
  m->lock();
  cache("Mylar", m);
  return m;
}

// ----------------------------------------------------------------------------
//  ArCO2 70/30 counting gas (by volume at STP)
//  Component densities at STP:  Ar = 1.784e-3 g/cm³,  CO2 = 1.977e-3 g/cm³
//  Mixture density: 0.70*1.784e-3 + 0.30*1.977e-3 = 1.842e-3 g/cm³
//
//  Mass fractions of the mixture:
//    Ar  : 0.70*39.948 / (0.70*39.948 + 0.30*44.009) = 27.964/41.166 = 0.6793
//    C   : 0.30*12.011 / 41.166                                        = 0.0875
//    O   : 0.30*31.998 / 41.166                                        = 0.2332
// ----------------------------------------------------------------------------
GeoMaterial* MaterialManager::arco2() {
  if (auto* m = getCached("ArCO2_70_30")) return m;

  auto* Ar = new GeoElement("Argon",   "Ar", 18.0, 39.948 * g/mole);
  auto* C  = new GeoElement("Carbon",  "C",   6.0, 12.011 * g/mole);
  auto* O  = new GeoElement("Oxygen",  "O",   8.0, 15.999 * g/mole);
  auto* m  = new GeoMaterial("ArCO2_70_30", 1.842e-3 * g/cm3);
  m->add(Ar, 0.6793);
  m->add(C,  0.0875);
  m->add(O,  0.2332);
  m->lock();
  cache("ArCO2_70_30", m);
  return m;
}

MaterialManager::RGBA MaterialManager::rgbaFor(const std::string& name) const {
  if (name == "Lead")        return {0.80, 0.80, 0.80, 1.0};  // dark grey
  if (name == "Iron")        return {0.60, 0.60, 0.60, 1.0};  // light grey
  if (name == "PVT")         return {0.00, 0.65, 0.65, 1.0};  // teal
  if (name == "Polystyrene") return {0.20, 0.85, 0.20, 1.0};  // green
  if (name == "Aluminium")   return {0.85, 0.85, 0.85, 1.0};  // silver
  if (name == "Mylar")       return {0.85, 0.85, 0.85, 0.6};  // silver, semi-transparent
  if (name == "ArCO2_70_30") return {0.30, 0.70, 1.00, 0.5};  // light blue, semi-transparent
  if (name == "Air")         return {1.00, 1.00, 1.00, 0.02}; // nearly invisible
  return {1.0, 1.0, 1.0, 1.0};
}
