cmake_minimum_required(VERSION 3.16)

project(Jacquard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

file(GLOB SRC_FILES "src/*.cpp")
file(GLOB SCENE_SRC_FILES "src/scene/*.cpp")
file(GLOB_RECURSE HDR_FILES "src/*.h" "src/scene/*.h")
file(GLOB UI_FILES "forms/*.ui")
file(GLOB RC_FILES "resources/*.qrc")

add_executable(Jacquard MACOSX_BUNDLE ${SRC_FILES} ${SCENE_SRC_FILES} ${HDR_FILES} ${UI_FILES} ${RC_FILES})
set_target_properties(Jacquard PROPERTIES
    AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms
)

include_directories(libs)

target_include_directories(Jacquard PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene
	${CMAKE_BINARY_DIR}
)

target_link_libraries(Jacquard PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

if(APPLE)
    set_target_properties(Jacquard PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist" 
    )

 	set(RESOURCE_DIRS
        environment_maps
        glsl
		resources
		drafts
    )
add_custom_command(TARGET Jacquard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:Jacquard>/Contents/Resources"
        VERBATIM
    )

    foreach(DIR_NAME ${RESOURCE_DIRS})
        set(RESOURCE_DIR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}")
        set(RESOURCE_DIR_DESTINATION "$<TARGET_BUNDLE_DIR:Jacquard>/Contents/Resources/${DIR_NAME}") 

        if(EXISTS "${RESOURCE_DIR_SOURCE}")
            add_custom_command(TARGET Jacquard POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCE_DIR_SOURCE}" "${RESOURCE_DIR_DESTINATION}"
                COMMENT "Copying ${DIR_NAME} to bundle Resources directory"
                VERBATIM
            )
            message(STATUS "Added post-build command to copy directory: ${DIR_NAME} to Contents/Resources/${DIR_NAME}")
        else()
             message(WARNING "Resource directory not found, skipping copy: ${RESOURCE_DIR_SOURCE}")
        endif()
    endforeach()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(STATUS "Enabling additional warnings for Clang/GCC")
    target_compile_options(Jacquard PRIVATE -Wall -Wextra -pedantic -Winit-self -Wno-strict-aliasing -fno-omit-frame-pointer)
    target_compile_options(Jacquard PRIVATE -fstack-protector-all)
endif()

#target_compile_definitions(Jacquard PRIVATE PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}")