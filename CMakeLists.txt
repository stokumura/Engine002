CMAKE_MINIMUM_REQUIRED(VERSION 3.25)
CMAKE_POLICY(SET CMP0072 NEW)

PROJECT(Engine002 VERSION 0.0.1)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS ON)

IF(UNIX AND NOT APPLE)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")
ENDIF()

IF(WIN32)
    SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
ENDIF()

FILE(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

IF(WIN32)
    FIND_PACKAGE(GLFW3 MODULE REQUIRED)
    FIND_PACKAGE(GLM MODULE REQUIRED)
    FIND_PACKAGE(ASSIMP MODULE REQUIRED)
ELSEIF(UNIX AND NOT APPLE)
    FIND_PACKAGE(OpenGL REQUIRED)
    FIND_PACKAGE(glfw3 CONFIG REQUIRED)
    FIND_PACKAGE(glm CONFIG REQUIRED)
    FIND_PACKAGE(assimp CONFIG REQUIRED)
ENDIF()

ADD_LIBRARY(GLAD src/gl.c)
TARGET_INCLUDE_DIRECTORIES(GLAD PRIVATE ${CMAKE_SOURCE_DIR}/includes)
ADD_LIBRARY(STB_IMAGE src/stb_image.cpp)
TARGET_INCLUDE_DIRECTORIES(STB_IMAGE PRIVATE ${CMAKE_SOURCE_DIR}/includes)

SET(FILE_READER
    "src/Utils/FileReader/FileReader.h"
    "src/Utils/FileReader/FileReader.cpp"
)

SET(DEBUG_SRC
    "src/Utils/Debug/Debug.h"
    "src/Utils/Debug/Debug.cpp"
)

SET(EXECUTABLE_PATH
    "src/Utils/ExecutablePath/ExecutablePath.h"
    "src/Utils/ExecutablePath/ExecutablePath.cpp"
)

SET(LOGGER
    "src/Utils/Logger/Logger.h"
    $<$<CONFIG:Debug>:src/Utils/Logger/Logger.cpp>
)

SET(UTILS 
    ${FILE_READER}
    ${DEBUG_SRC}
    ${EXECUTABLE_PATH}
    ${LOGGER}
    "src/Utils/Utils.h"
)

SET(SHADER
    "src/Engine/Shader/Shader.h"
    "src/Engine/Shader/Shader.cpp"
    "src/Engine/Shader/ShaderProgram.h"
    "src/Engine/Shader/ShaderProgram.cpp"
    "src/Engine/Shader/ShaderProgramUniforms.cpp"
    "src/Engine/Shader/ShaderUniforms.h"
    "src/Engine/Shader/ShaderUniforms.cpp"
    "src/Engine/Shader/UniformBlock.h"
    "src/Engine/Shader/UniformBlock.cpp"
)

SET(CAMERA
    "src/Engine/Camera/Camera.h"
    "src/Engine/Camera/Camera.cpp"
)

SET(MESH
    "src/Engine/Mesh/VertexAttribute.h"
    "src/Engine/Mesh/VertexAttribute.cpp"
    "src/Engine/Mesh/Mesh.h"
    "src/Engine/Mesh/Mesh.cpp"
)

SET(PRIMITIVE
    "src/Engine/Primitive/Primitive.h"
    "src/Engine/Primitive/Shape.h"
    "src/Engine/Primitive/Shape.cpp"
    "src/Engine/Primitive/Sphere.h"
    "src/Engine/Primitive/Sphere.cpp"
    "src/Engine/Primitive/Cylinder.h"
    "src/Engine/Primitive/Cylinder.cpp"
    "src/Engine/Primitive/Cube.h"
    "src/Engine/Primitive/Cube.cpp"
)

SET(TEXTURE
    "src/Engine/Texture/Sampler.h"
    "src/Engine/Texture/Sampler.cpp"
    "src/Engine/Texture/Texture.h"
    "src/Engine/Texture/Texture.cpp"
)

SET(SHADER_VARIANTS
    "src/Engine/ShaderVariants/ShaderDefines.h"
    "src/Engine/ShaderVariants/ShaderDefines.cpp"
    "src/Engine/ShaderVariants/ShaderVariants.h"
    "src/Engine/ShaderVariants/ShaderVariants.cpp"
)

SET(MODEL
    "src/Engine/Model/Model.h"
    "src/Engine/Model/Model.cpp"
)

SET(ENGINE
    ${SHADER}
    ${CAMERA}
    ${MESH}
    ${PRIMITIVE}
    ${TEXTURE}
    ${SHADER_VARIANTS}
    ${MODEL}
    "src/Engine/Engine.h"
)

SET(SOURCE
    ${UTILS}
    ${ENGINE}
    "src/main.cpp"
)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE})

TARGET_LINK_DIRECTORIES(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)

SET(LIBS GLAD STB_IMAGE)
IF(WIN32)
    SET(LIBS ${LIBS} ${ASSIMP_LIBRARIES} ${GLFW3_LIBRARY} opengl32)
ELSEIF(UNIX AND NOT APPLE)
    SET(LIBS ${LIBS} glm::glm assimp::assimp glfw OpenGL::GL)
ENDIF()
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ${LIBS})

IF(NOT MSVC)
    TARGET_LINK_OPTIONS(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
ENDIF()

IF(WIN32)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/includes ${GLM_INCLUDE_DIRS})
ELSEIF(UNIX AND NOT APPLE)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/includes)
ENDIF()

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:DEBUG>)
ELSEIF(UNIX AND NOT APPLE)
    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        MESSAGE(STATUS "Debug mode enabled")
        TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC DEBUG)
    ENDIF()

    IF(CMAKE_EXPORT_COMPILE_COMMANDS)
        FILE(CREATE_LINK
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/build/compile_commands.json"
            SYMBOLIC
        )
        MESSAGE(STATUS "Compile Commands symbolic link created")
    ENDIF()
ENDIF()
