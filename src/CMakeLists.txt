project(UniDeskCppExt)

set (CMAKE_CXX_STANDARD 17)

# add_compile_options(/Zc:__cplusplus /permissive- /W4)
# find Qt
find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 DBus Core5Compat Gui Qml REQUIRED)

# create the library
add_library(unideskcppext STATIC singleton.h stdafx.h UDFrameless.h UDFrameless.cpp UDTools.h UDTools.cpp )
target_link_libraries(unideskcppext PUBLIC Qt6::Core Qt6::Widgets Qt6::Quick Qt6::QuickControls2 Qt6::DBus Qt6::Core5Compat Qt6::Gui Qt6::Qml)
target_include_directories(unideskcppext PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

