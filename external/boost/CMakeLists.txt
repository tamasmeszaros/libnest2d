set(URL_BOOST "https://github.com/boostorg/geometry.git" CACHE STRING "Location of the boost sources")

include(ExternalProject)
ExternalProject_Add(   dep_boost
                       GIT_REPOSITORY     ${URL_BOOST}
                       INSTALL_DIR	  ${CMAKE_PREFIX_PATH}
                       CONFIGURE_COMMAND  ""
                       BUILD_COMMAND      ""
                       INSTALL_COMMAND    "${CMAKE_COMMAND} -E copy_directory include/boost ${CMAKE_PREFIX_PATH}/include/boost"
                       TEST_COMMAND       ""
)
