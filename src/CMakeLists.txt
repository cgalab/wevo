if(BUILD_VIEW)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif(BUILD_VIEW)

find_package(PkgConfig REQUIRED)
find_package(Boost 1.58 REQUIRED COMPONENTS thread program_options)
find_package(CGAL REQUIRED COMPONENTS Core)

if(BUILD_LOG)
    find_package(Boost 1.58 REQUIRED COMPONENTS system log log_setup)
endif(BUILD_LOG)

if(BUILD_VIEW)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL Svg)
    find_package(CGAL REQUIRED COMPONENTS Qt5)
endif(BUILD_VIEW)

set(SOURCES bisector.cpp file_io.cpp isect.cpp log.cpp main.cpp mwvd.cpp offcirc.cpp 
    overlay.cpp sites.cpp timepnt.cpp traj.cpp util.cpp view.cpp)
set (UIS mwvd.ui)
set (RESOURCES mwvd.qrc)

if(CGAL_FOUND AND (CGAL_Qt5_FOUND OR NOT BUILD_VIEW))
    add_definitions(-DBOOST_ALL_DYN_LINK -DBOOST_LOG_DYN_LINK)

    add_executable(mwvd ${SOURCES} ${UIS} ${RESOURCES})

    set_target_properties(mwvd PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

    target_link_libraries(mwvd Boost::thread Boost::program_options)
    if(BUILD_LOG)
        target_link_libraries(mwvd Boost::system Boost::log Boost::log_setup)
    endif(BUILD_LOG)

    target_link_libraries(mwvd CGAL::CGAL CGAL::CGAL_Core)
    if(BUILD_VIEW)
        target_link_libraries(mwvd CGAL::CGAL_Qt5)
        target_link_libraries (mwvd Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Svg)
    endif(BUILD_VIEW)
else()
  message("ERROR: this program requires CGAL and CGAL_Qt5 and will not be compiled.")
endif()