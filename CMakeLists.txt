cmake_minimum_required(VERSION 3.16)
project(UBTGeoSim LANGUAGES CXX)

# ---- C++ standard -----------------------------------------------------------
# Set globally BEFORE find_package calls so nothing can override it.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Dependencies -----------------------------------------------------------
find_package(GeoModelCore REQUIRED)
find_package(GeoModelG4   REQUIRED)
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
include(${ROOT_USE_FILE})

# Geant4_USE_FILE and ROOT_USE_FILE may inject their own -std flags.
# Re-assert C++20 here so it wins in all compile-flag lists.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
#  GeoModel2G4 / GeoMaterial2G4 link helper
#  Resolves the target name difference across GeoModelG4 package versions.
# ============================================================================
function(link_geomodel2g4 target)
    if (TARGET GeoModelG4::GeoModel2G4)
        target_link_libraries(${target} PRIVATE GeoModelG4::GeoModel2G4)
    elseif (TARGET GeoModel2G4)
        target_link_libraries(${target} PRIVATE GeoModel2G4)
    else()
        message(FATAL_ERROR "GeoModel2G4 target not found for ${target}.")
    endif()

    if (TARGET GeoModelG4::GeoMaterial2G4)
        target_link_libraries(${target} PRIVATE GeoModelG4::GeoMaterial2G4)
    elseif (TARGET GeoMaterial2G4)
        target_link_libraries(${target} PRIVATE GeoMaterial2G4)
    endif()
endfunction()

# ============================================================================
#  UBTGeo — geometry library shared by both executables
#  Contains: UBTPlaneBuilder, UBTAllTilePlaneBuilder, MaterialManager
# ============================================================================
add_library(UBTGeo STATIC
    src/UBTPlaneBuilder.cpp
    src/UBTAllTilePlaneBuilder.cpp
    src/MaterialManager.cpp
)
target_include_directories(UBTGeo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(UBTGeo PUBLIC cxx_std_20)
target_link_libraries(UBTGeo PUBLIC
    GeoModelCore::GeoModelKernel
)

# ============================================================================
#  run_ubt — hybrid detector (drift tubes + 1 cm PS tiles)
# ============================================================================
add_executable(run_ubt
    exec/main_g4.cpp
    src/UBTDetectorConstruction.cc
    src/UBTSD.cc
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
link_geomodel2g4(run_ubt)

# ============================================================================
#  run_ubt_alltile — all-polystyrene-tile detector (2×2 and 4×4 cm tiles)
# ============================================================================
add_executable(run_ubt_alltile
    exec/main_ubt_alltile.cpp
    src/UBTAllTileDetectorConstruction.cc
    src/UBTAllTileSD.cc
    src/UBTAllTileRunAction.cc
    src/UBTAllTileEventAction.cc
    src/UBTAllTilePrimaryGeneratorAction.cc
)
target_include_directories(run_ubt_alltile PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(run_ubt_alltile PRIVATE cxx_std_20)
target_link_libraries(run_ubt_alltile PRIVATE
    UBTGeo
    ${Geant4_LIBRARIES}
    ROOT::Core ROOT::RIO ROOT::Tree
)
link_geomodel2g4(run_ubt_alltile)

# ============================================================================
#  Copy macro files to build directory
# ============================================================================
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ubt_vis.mac
    ${CMAKE_CURRENT_BINARY_DIR}/ubt_vis.mac
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ubt_alltile_vis.mac
    ${CMAKE_CURRENT_BINARY_DIR}/ubt_alltile_vis.mac
    COPYONLY
)
