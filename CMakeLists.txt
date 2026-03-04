cmake_minimum_required(VERSION 3.5)
project(cvedix_backend CXX)

# ===================== C++ STANDARD =====================
include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)

if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    # do nothing
elseif(HAS_ANY AND HAS_STRING_VIEW AND HAS_COROUTINE)
    set(CMAKE_CXX_STANDARD 20)
elseif(HAS_ANY AND HAS_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===================== CVEDIX SDK =====================
set(CVEDIX_ROOT /opt/edgeos-sdk)
set(CVEDIX_INCLUDE_DIR ${CVEDIX_ROOT}/include)
set(CVEDIX_LIB_DIR ${CVEDIX_ROOT}/lib/cvedix)

message(STATUS "CVEDIX ROOT = ${CVEDIX_ROOT}")
message(STATUS "CVEDIX INCLUDE = ${CVEDIX_INCLUDE_DIR}")
message(STATUS "CVEDIX LIB = ${CVEDIX_LIB_DIR}")

# ===================== EXECUTABLE =====================
add_executable(${PROJECT_NAME} main.cc)

# ===================== DROGON / JSON / JWT =====================
find_package(Drogon CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

find_package(jwt-cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE jwt-cpp::jwt-cpp)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# ===================== OPENCV =====================
find_package(OpenCV REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

# ===================== EIGEN =====================
find_package(Eigen3 REQUIRED)

# ===================== PKG CONFIG =====================
find_package(PkgConfig REQUIRED)

# ===================== GStreamer =====================
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
)

target_include_directories(${PROJECT_NAME} PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GST_LIBRARIES})

# ===================== CVEDIX =====================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CVEDIX_INCLUDE_DIR}
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${CVEDIX_LIB_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Eigen3::Eigen
        cvedix_core
        trt_yolov11
        trt_yolov11_face
        tinyexpr
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        CVEDIX_WITH_GSTREAMER
)

# ===================== OPTIONAL KAFKA =====================
option(ENABLE_CVEDIX_KAFKA "Enable Kafka broker support (requires librdkafka)" OFF)

if(ENABLE_CVEDIX_KAFKA)
    message(STATUS "Kafka support enabled")
    pkg_check_modules(RDKAFKA REQUIRED rdkafka)
    target_include_directories(${PROJECT_NAME} PRIVATE ${RDKAFKA_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${RDKAFKA_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE CVEDIX_WITH_KAFKA)
endif()

# ===================== SOURCE FILES =====================
aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)
aux_source_directory(controllers/MakeNode MAKE_NODE_SRC)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/controllers/MakeNode
        ${CMAKE_CURRENT_SOURCE_DIR}/models
)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${CTL_SRC}
        ${MAKE_NODE_SRC}
        ${FILTER_SRC}
        ${PLUGIN_SRC}
        ${MODEL_SRC}
)

# ===================== DROGON VIEWS =====================
drogon_create_views(
    ${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_BINARY_DIR}
)

# ===================== BUILD TYPE =====================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build type: DEBUG")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -O0 -Wall -Wextra)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
else()
    message(STATUS "Build type: RELEASE")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

# ===================== TEST =====================
add_subdirectory(test)
