cmake_minimum_required(VERSION 2.8)

set( project miso )
project( ${project} )

set(${project}_sources 
	miso.cpp
	miso.h
)

add_executable( ${project} ${${project}_sources}) 

set_property(TARGET ${project} PROPERTY CXX_STANDARD 14)

