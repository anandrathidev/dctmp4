
#
cmake_minimum_required(VERSION 3.22)
project(OPCTest C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -Wall -Wextra -Werror -Wno-write-strings -Wno-unused-parameter -fdump-rtl-all -g -Wl,-v ") 

set(Boost_LIB_PREFIX lib)
set(Boost_USE_STATIC_LIBS ON)

set( Boost_USE_MULTITHREADED ON )


message("FFMPEG_LIB_PATHS=${FFMPEG_LIB_PATHS} ")


set(FFMPEG_INC_PATH  "$ENV{HOME}//ffmpeg/"  )
# set(FFMPEG_INC_PATH  "${PROJECT_SOURCE_DIR}/../ffmpeg/"  )

find_package(Boost REQUIRED COMPONENTS thread ) 
find_package(ZLIB REQUIRED)
find_package(LibLZMA REQUIRED)


# set(FFMPEG_LIB_PATHS "${PROJECT_SOURCE_DIR}/../ffmpeg/lib" )
set(FFMPEG_LIB_PATHS  "$ENV{HOME}//ffmpeg/lib"  )

#/usr/lib/x86_64-linux-gnu/libdrm.so  
find_library(
     LIBVA_DRM_LIB
     NAMES  drm
     PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib /opt/local/lib
     )

find_library(
    LIBVA_VA_LIB
    NAMES  va
    PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib /opt/local/lib
    )
   
find_library(LIB_FFMPEG_AVCODEC 
NAMES avcodec
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)

find_library(LIB_FFMPEG_AVUTIL 
NAMES avutil 
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)

find_library(LIB_FFMPEG_AVFORMAT 
NAMES avformat 
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)


find_library(LIB_FFMPEG_SWRESAMPLE 
NAMES swresample 
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)

find_library(LIB_FFMPEG_AVDEVICE 
NAMES avdevice 
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)


find_library(LIB_FFMPEG_SWSCALE 
NAMES swscale 
PATHS 
${FFMPEG_LIB_PATHS}
NO_CACHE
NO_DEFAULT_PATH
)

#message(" LIB_FFMPEG_AVCODEC_STATIC=${LIB_FFMPEG_AVCODEC_STATIC} ")
#message(" LIB_FFMPEG_AVUTIL_STATIC=${LIB_FFMPEG_AVUTIL_STATIC} ")
#message(" LIB_FFMPEG_AVFORMAT_STATIC=${LIB_FFMPEG_AVFORMAT_STATIC} ")
message(" LIBVA_DRM_LIB=${LIBVA_DRM_LIB} ")


#add_library(LIB_FFMPEG_AVCODEC_STATIC_A   STATIC ${LIB_FFMPEG_AVCODEC_STATIC})
#add_library(LIB_FFMPEG_AVUTIL_STATIC_A    STATIC ${LIB_FFMPEG_AVCODEC_STATIC})
#add_library(LIB_FFMPEG_AVFORMAT_STATIC_A  STATIC ${LIB_FFMPEG_AVCODEC_STATIC})

    set(FFMPEGTest_SOURCES
    decode.cpp
    dct.cpp
    Matrix.cpp 

    Matrix.hpp
    # [...]
    )
    #set_source_files_properties(${FFMPEGTest_SOURCES} PROPERTIES LANGUAGE CXX)

    add_executable(DCTEncoder 
    decode.cpp
    dct.cpp
    Matrix.cpp 
   )

    set_target_properties(DCTEncoder PROPERTIES LINKER_LANGUAGE CXX)
    
target_include_directories(DCTEncoder SYSTEM PRIVATE ${FFMPEG_INC_PATH})

#target_link_libraries(DCTEncoder "$<LINK_LIBRARY:WHOLE_ARCHIVE,LIB_FFMPEG_AVCODEC_STATIC>"  ) 
#target_link_libraries(DCTEncoder "$<LINK_LIBRARY:WHOLE_ARCHIVE,LIB_FFMPEG_AVUTIL_STATIC>"   ) 
#target_link_libraries(DCTEncoder "$<LINK_LIBRARY:WHOLE_ARCHIVE,LIB_FFMPEG_AVFORMAT_STATIC>" ) 

# target_link_libraries(DCTEncoder -Wl, --whole-archive ${LIB_FFMPEG_AVCODEC_STATIC}  -Wl,--no-whole-archive) 
# target_link_libraries(DCTEncoder  -Wl,--whole-archive ${LIB_FFMPEG_AVUTIL_STATIC}  -Wl,--no-whole-archive) 
# target_link_libraries(DCTEncoder  -Wl,--whole-archive ${LIB_FFMPEG_AVFORMAT_STATIC}  -Wl,--no-whole-archive) 
# target_link_libraries(DCTEncoder  -Wl,--whole-archive ${LIB_FFMPEG_SWRESAMPLE_STATIC}  -Wl,--no-whole-archive) 

target_link_directories(DCTEncoder PRIVATE BEFORE ${FFMPEG_LIB_PATHS} )

target_link_libraries(DCTEncoder  ${LIB_FFMPEG_AVUTIL}     ) 
target_link_libraries(DCTEncoder  ${LIB_FFMPEG_AVFORMAT}   ) 
target_link_libraries(DCTEncoder  ${LIB_FFMPEG_SWRESAMPLE} ) 
target_link_libraries(DCTEncoder  ${LIB_FFMPEG_AVCODEC}    ) 
target_link_libraries(DCTEncoder  ${LIB_FFMPEG_SWSCALE}   )

#target_link_libraries(DCTEncoder  -Wl,--whole-archive ${LIB_FFMPEG_AVDEVICE_STATIC}  -Wl,--no-whole-archive) 
target_link_libraries(DCTEncoder  ${LIBVA_DRM_LIB})
target_link_libraries(DCTEncoder  ${LIBVA_VA_LIB})
target_link_libraries(DCTEncoder ZLIB::ZLIB)
target_link_libraries(DCTEncoder LibLZMA::LibLZMA)
