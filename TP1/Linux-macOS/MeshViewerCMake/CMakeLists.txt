cmake_minimum_required(VERSION 3.10)
project(MeshViewer)

set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE LEGACY)

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})

# glm
find_package(glm REQUIRED)

# Hard-coded paths for Homebrew (M1 Mac)
set(GLEW_INCLUDE_DIR "/opt/homebrew/include")

include_directories(
    ${GLEW_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(MESH_SOURCE_FILES
    myHalfedge.cpp
    myVector3D.cpp
    myPoint3D.cpp
    myFace.cpp
    myMesh.cpp
    myVertex.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${MESH_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    "/opt/homebrew/lib/libGLEW.dylib"
    "-framework GLUT"
    glm::glm
)

include(CTest)
if(BUILD_TESTING)
    add_executable(CatmullClarkTest
        tests/catmull_clark_test.cpp
        ${MESH_SOURCE_FILES}
    )
    target_link_libraries(CatmullClarkTest ${OPENGL_LIBRARIES})
    add_test(
        NAME catmull_clark
        COMMAND CatmullClarkTest
            ${CMAKE_CURRENT_SOURCE_DIR}/cube.obj
            ${CMAKE_CURRENT_SOURCE_DIR}/surface_revolution.obj
    )
endif()
