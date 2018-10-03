project(lib_list)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "*.c")

#Generate the static library from the sources
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} itf_lib_convention lib_thread)

if (TARGET lib_FREERTOS)
	#target_link_libraries(${PROJECT_NAME} itf_lib_convention itf_lib_thread lib_FREERTOS)
	target_compile_definitions(${PROJECT_NAME} PUBLIC CONFIG_LIST__LOCK_TYPE_FREERTOS)
	message(STATUS "freertos available")
else()
 	#target_link_libraries(${PROJECT_NAME} itf_lib_convention itf_lib_thread)
endif()


#target_compile_definitions(${PROJECT_NAME} PUBLIC CONFIG_LIST__LOCK_TYPE_MTX)
#target_compile_definitions(${PROJECT_NAME} PUBLIC CONFIG_LIST__LOCK_TYPE_CAS)


target_include_directories(${PROJECT_NAME} PUBLIC ./)  
