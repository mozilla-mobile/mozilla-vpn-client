add_definitions(-DAPP_VERSION=\"${CMAKE_PROJECT_VERSION}\")
add_definitions(-DBUILD_ID=\"1234\") ## TODO: FIX ME!
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../translations/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../glean)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lottie/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../nebula)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Test)
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)
find_package(Qt6 REQUIRED COMPONENTS NetworkAuth)
find_package(Qt6 REQUIRED COMPONENTS WebSockets)

add_executable(mozillavpn)

target_link_libraries(mozillavpn PRIVATE
    Qt6::NetworkAuth
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

include(cmake/sources.cmake)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DMVPN_LINUX)
    include(cmake/linux.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DMVPN_WINDOWS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_definitions(-DMVPN_MACOS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_definitions(-DMVPN_ANDROID)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    add_definitions(-DMVPN_IOS)
endif()

set_target_properties(mozillavpn PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
