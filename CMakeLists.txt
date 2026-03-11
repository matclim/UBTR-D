cmake_minimum_required(VERSION 3.16)
project(UBTGeoSim LANGUAGES CXX)

# ---- C++ standard -----------------------------------------------------------
# Set globally BEFORE find_package calls so nothing can override it.
# Also set CXX_EXTENSIONS OFF to avoid -std=gnu++20 vs -std=c++20 mismatches.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Dependencies ------------------------------------------------------------
find_package(GeoModelCore REQUIRED)
find_package(GeoModelG4   REQUIRED)
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
include(${ROOT_USE_FILE})

# Geant4_USE_FILE and ROOT_USE_FILE can inject their own -std flags via
# add_compile_options / CMAKE_CXX_FLAGS.  Re-assert C++20 after both includes
# so it wins in all compile-flag lists.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- UBT geometry library -----------------------------------------------
add_library(UBTGeo STATIC
  src/UBTPlaneBuilder.cpp
  src/MaterialManager.cpp
)
target_include_directories(UBTGeo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(UBTGeo PUBLIC cxx_std_20)
target_link_libraries(UBTGeo PUBLIC
  GeoModelCore::GeoModelKernel
)

# ---- Geant4 simulation executable -------------------------------------------
add_executable(run_ubt
  exec/main_g4.cpp
  src/UBTDetectorConstruction.cc
  src/UBTSD.cc
  src/UBTHit.cc
  src/UBTRunAction.cc
  src/UBTEventAction.cc
  src/UBTPrimaryGeneratorAction.cc
)
target_include_directories(run_ubt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(run_ubt PRIVATE cxx_std_20)
target_link_libraries(run_ubt PRIVATE
  UBTGeo
  ${Geant4_LIBRARIES}
  ROOT::Core ROOT::RIO ROOT::Tree
)

# GeoModel -> Geant4 bridge
if (TARGET GeoModelG4::GeoModel2G4)
  target_link_libraries(run_ubt PRIVATE GeoModelG4::GeoModel2G4)
elseif (TARGET GeoModel2G4)
  target_link_libraries(run_ubt PRIVATE GeoModel2G4)
else()
  message(FATAL_ERROR "GeoModel2G4 target not found.")
endif()

if (TARGET GeoModelG4::GeoMaterial2G4)
  target_link_libraries(run_ubt PRIVATE GeoModelG4::GeoMaterial2G4)
elseif (TARGET GeoMaterial2G4)
  target_link_libraries(run_ubt PRIVATE GeoMaterial2G4)
endif()

# Copy vis macro to build directory
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ubt_vis.mac
  ${CMAKE_CURRENT_BINARY_DIR}/ubt_vis.mac
  COPYONLY
)
