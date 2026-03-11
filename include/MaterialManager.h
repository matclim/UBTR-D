#pragma once

#include <map>
#include <string>
#include <array>

class GeoMaterial;
class GeoElement;

class MaterialManager {
public:
  MaterialManager() = default;

  // Return existing or create+cache
  GeoMaterial* air();          // simple air proxy (N)
  GeoMaterial* lead();         // Pb
  GeoMaterial* iron();         // Fe
  GeoMaterial* pvt();          // C9H10, density ~1.032 g/cm3
  GeoMaterial* polystyrene();  // C8H8, density ~1.05 g/cm3
  GeoMaterial* aluminum();     // Al
  GeoMaterial* mylar();        // PET C10H8O4, density 1.39 g/cm3
  GeoMaterial* arco2();        // ArCO2 70/30 by volume, density ~1.84e-3 g/cm3

  // RGBA in [0,1]
  using RGBA = std::array<double,4>;
  RGBA rgbaFor(const std::string& materialName) const;

private:
  GeoMaterial* getCached(const std::string& key) const;
  void cache(const std::string& key, GeoMaterial* mat);

  mutable std::map<std::string, GeoMaterial*> m_mats;
};
