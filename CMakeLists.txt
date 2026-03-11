cmake_minimum_required(VERSION 3.16)
project(TrackerGeoSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Dependencies ------------------------------------------------------------
find_package(GeoModelCore REQUIRED)
find_package(GeoModelG4   REQUIRED)
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# ---- Tracker geometry library -----------------------------------------------
add_library(TrackerGeo STATIC
  src/TrackerPlaneBuilder.cpp
  src/MaterialManager.cpp
)
target_include_directories(TrackerGeo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(TrackerGeo PUBLIC
  GeoModelCore::GeoModelKernel
)

# ---- Geant4 simulation executable -------------------------------------------
add_executable(run_tracker
  exec/main_g4.cpp
  src/TrackerDetectorConstruction.cc
)
target_include_directories(run_tracker PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(run_tracker PRIVATE
  TrackerGeo
  ${Geant4_LIBRARIES}
)

# GeoModel -> Geant4 bridge (target name varies across GeoModelG4 versions)
if (TARGET GeoModelG4::GeoModel2G4)
  target_link_libraries(run_tracker PRIVATE GeoModelG4::GeoModel2G4)
elseif (TARGET GeoModel2G4)
  target_link_libraries(run_tracker PRIVATE GeoModel2G4)
else()
  message(FATAL_ERROR "GeoModel2G4 target not found. Check GeoModelG4Config.cmake")
endif()

if (TARGET GeoModelG4::GeoMaterial2G4)
  target_link_libraries(run_tracker PRIVATE GeoModelG4::GeoMaterial2G4)
elseif (TARGET GeoMaterial2G4)
  target_link_libraries(run_tracker PRIVATE GeoMaterial2G4)
endif()

# ---- Copy vis macro to build directory so it is found at runtime ------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tracker_vis.mac
  ${CMAKE_CURRENT_BINARY_DIR}/tracker_vis.mac
  COPYONLY
)
