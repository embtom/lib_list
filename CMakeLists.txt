project(lib_list)
cmake_minimum_required(VERSION 2.8)

######################################################################################
# Selector to choose lock for critical section
######################################################################################

# Global Registration of the available list lock practices 
FUNCTION(lib_list_add_lock)
	FOREACH(ARG ${ARGV})
  		set_property(GLOBAL APPEND PROPERTY LIB_LIST_LOCKS ${ARG})
	ENDFOREACH(ARG)
ENDFUNCTION(lib_list_add_lock)

lib_list_add_lock("lock_cas")
lib_list_add_lock("lock_freertos")
lib_list_add_lock("lock_mem")
lib_list_add_lock("lock_mtx")

set(LIB_LIST_LOCK ${LIB_LIST_LOCK} CACHE STRING "Architecture to build lib_list on")
get_property(architectures GLOBAL PROPERTY LIB_LIST_LOCKS)
list(SORT architectures)
set_property(CACHE LIB_LIST_LOCK PROPERTY STRINGS None ${architectures})

#######################################################################################
# Evaluate selection
#######################################################################################
SET(LIB_LIST_SOURCE_C   		"src/lib_list.c")
SET(LIB_LIST_ITF_HEADER          "include")

if("${LIB_LIST_LOCK}" STREQUAL "lock_cas")
	SET(LIB_LIST_PUBLIC_DEFINITION 	CONFIG_LIST__LOCK_TYPE_CAS)
	LIST(APPEND LIB_LIST_ITF_HEADER "include/cas")
	SET(LIB_LIST_DEPEND              lib_convention)
elseif("${LIB_LIST_LOCK}" STREQUAL "lock_freertos")
	SET(LIB_LIST_PUBLIC_DEFINITION 	CONFIG_LIST__LOCK_TYPE_FREERTOS)
	LIST(APPEND LIB_LIST_ITF_HEADER "include/freertos")
	SET(LIB_LIST_DEPEND              lib_convention lib_FREERTOS)
elseif("${LIB_LIST_LOCK}" STREQUAL "lock_mem")
	SET(LIB_LIST_PUBLIC_DEFINITION 	CONFIG_LIST__LOCK_TYPE_MEM)
	LIST(APPEND LIB_LIST_ITF_HEADER "include/mem")
	SET(LIB_LIST_DEPEND              lib_convention)
elseif("${LIB_LIST_LOCK}" STREQUAL "lock_mtx")
	SET(LIB_LIST_PUBLIC_DEFINITION 	CONFIG_LIST__LOCK_TYPE_MTX)
	LIST(APPEND LIB_LIST_ITF_HEADER "include/mtx")
	SET(LIB_LIST_DEPEND              lib_convention lib_thread)
elseif("${LIB_LIST_LOCK}" STREQUAL "None")
	message(FATAL_ERROR "No valid lock chosen! Please check LIB_LIST_LOCK.")
else()
	message(FATAL_ERROR "No valid lock chosen! Please check LIB_LIST_LOCK.")
endif("${LIB_LIST_LOCK}" STREQUAL "lock_cas")

message(STATUS "LIB_LIST : selected lock ${LIB_LIST_LOCK}")

#######################################################################################
#Build target settings
#######################################################################################
add_library(${PROJECT_NAME} STATIC ${LIB_LIST_SOURCE_C})
target_link_libraries(${PROJECT_NAME} ${LIB_LIST_DEPEND})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LIB_LIST_PUBLIC_DEFINITION})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_LIST_ITF_HEADER}) 


  
