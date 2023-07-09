cmake_minimum_required(VERSION 3.5)

set(PROJECT_SOURCES
        include/debug.h                 src/debug.cpp
        include/ocharutil.h             src/ocharutil.cpp
        include/oconfig.h               src/oconfig.cpp
        include/oconsole.h              src/oconsole.cpp
        include/ocontainer.h            src/ocontainer.cpp
        include/ocontainer.tpp          
        include/oexception.h            src/oexception.cpp
        include/ofile.h                 src/ofile.cpp
        include/oformat.h               src/oformat.cpp
        include/ographics.h             src/ographics.cpp
        include/omemory.h               src/omemory.cpp
        include/omemory.tpp             
        include/onumber.h               src/onumber.cpp
        include/ostringutil.h           src/ostringutil.cpp
        include/ostringutil.tpp
        include/osystem.h               src/osystem.cpp
        include/otestutil.h             src/otestutil.cpp
        include/otextutil.h             src/otextutil.cpp
        include/otime.h                 src/otime.cpp
        include/ounicode.h              src/ounicode.cpp
)

project(ostd VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

add_library(ostd STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE /home/herbert/swprojekte/lib/ostd/include)

#target_link_libraries(${PROJECT_NAME} PRIVATE -L/home/herbert/ablage/programme/lib)
#target_link_libraries(${PROJECT_NAME} PRIVATE libostd.a)

install(TARGETS ostd DESTINATION /home/herbert/ablage/programme/lib)