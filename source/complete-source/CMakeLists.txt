cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project(gotobed VERSION 2.0.7.0)

# CommonLibSSE-NG 6.7.0, built from source.
# The bundled vcpkg overlay port pinned the old Ryan-rsm-McKenzie CommonLibSSE, which only
# understands Address Library formats 1 and 2 - hence the original error:
#   REL/Relocation.h(636): Unsupported address library format: 5
set(ENABLE_SKYRIM_VR OFF)
set(BUILD_TESTS OFF)
add_subdirectory(external/CommonLibNG CommonLibNG EXCLUDE_FROM_ALL)
find_package(Detours REQUIRED)
find_package(nlohmann_json REQUIRED)
find_path(JCONTAINERS_INCLUDE_DIRS jcontainers/jc_interface.h)

configure_file(cmake/version.h.in version.h)
configure_file(cmake/version.rc.in version.rc)

add_library("${PROJECT_NAME}" SHARED)

target_sources("${PROJECT_NAME}"
	PRIVATE
	src/Actor.cpp
	src/Actor.h
	src/AIProcess.cpp
	src/AIProcess.h
	src/DiagUtil.cpp
	src/DiagUtil.h
	src/EquipConditions.cpp
	src/EquipConditions.h
	src/EquipMask.cpp
	src/EquipMask.h
	src/EquipSequence.cpp
	src/EquipSequence.h
	src/Fixes.cpp
	src/Fixes.h
	src/init.cpp
	src/init.h
	src/JCApi.cpp
	src/JCApi.h
	src/JCTypes.h
	src/main.cpp
	src/MenuOpenHandler.cpp
	src/MenuOpenHandler.h
	src/Offsets.h
	src/Outfit.cpp
	src/Outfit.h
	src/pch.h
	src/Settings.cpp
	src/Settings.h
	src/SettingsPapyrus.cpp
	src/SettingsPapyrus.h
	src/UIFuncs.cpp
	src/UIFuncs.h
	src/UIUtil.cpp
	src/UIUtil.h
	"${PROJECT_BINARY_DIR}/version.h"
	"${PROJECT_BINARY_DIR}/version.rc"
)

target_precompile_headers("${PROJECT_NAME}"
	PRIVATE
	src/pch.h
)

target_compile_features("${PROJECT_NAME}"
	PRIVATE
	cxx_std_23
)

target_include_directories("${PROJECT_NAME}"
	PRIVATE
	"${PROJECT_BINARY_DIR}"
	"${JCONTAINERS_INCLUDE_DIRS}"
)

target_link_libraries("${PROJECT_NAME}"
	PRIVATE
	CommonLibSSE::CommonLibSSE
	Detours::Detours
	nlohmann_json::nlohmann_json
)

install(TARGETS "${PROJECT_NAME}" RUNTIME DESTINATION "${SKYRIM_ROOT}/data/skse/plugins")