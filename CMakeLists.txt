cmake_minimum_required(VERSION 2.8)

set( project miso )
project( ${project} )

set(${project}_sources miso.cpp)

add_executable( ${project} ${${project}_sources}) 
