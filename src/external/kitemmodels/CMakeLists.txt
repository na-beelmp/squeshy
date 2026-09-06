cmake_minimum_required(VERSION 3.16)
project(KItemModels)

set(CMAKE_AUTOMOC ON)

add_library(KF6ItemModels STATIC
    src/core/klinkitemselectionmodel.cpp
    src/core/kmodelindexproxymapper.cpp
    src/core/kbreadcrumbselectionmodel.cpp
    src/core/kcheckableproxymodel.cpp
    src/core/kcolumnheadersmodel.cpp
    src/core/kdescendantsproxymodel.cpp
    src/core/kextracolumnsproxymodel.cpp
    src/core/knumbermodel.cpp
    src/core/krearrangecolumnsproxymodel.cpp
    src/core/kselectionproxymodel.cpp
    src/core/kitemmodels_debug.cpp
)

target_include_directories(KF6ItemModels PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/core>
)

target_link_libraries(KF6ItemModels PUBLIC
    Qt6::Core
    Qt6::Gui
)

add_library(KF6::ItemModels ALIAS KF6ItemModels)
