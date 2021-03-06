project(Hy3S Fortran)    
    
set (F90_SRC_FILES 
		src/f2kcli.f90
		src/globalvariables.f90
		src/ratelaws.f90
		src/dataio.f90
		src/randomgen.f90
		src/priorityqueue.f90
		src/specialevents.f90
		src/SDEintegration.f90
		src/propagators-HyJCMSS.f90
		src/mainprogram-HyJCMSS.f90
)
set_source_files_properties( ${F90_SRC_FILES} PROPERTIES LANGUAGE Fortran)
include_directories(${CMAKE_BINARY_DIR}/netcdf-3.6.2/f90 ${JMS_INCS})
set(DEP_LIBS ${NETCDF_F90_LIBRARIES})
#  target_link_libraries (uses_everthing ${NETCDF_LIBRARIES})
#  target_link_libraries (only_uses_f90 ${NETCDF_F90_LIBRARIES})


if (OPTION_TARGET_MESSAGING)
	set (C_SRC_FILES
		src/msgwrapper.c 
		../VCell/src/SimulationMessaging.cpp
	)
	include_directories(../VCell/include)
	add_definitions(${JMS_DEFS})
	set_source_files_properties(${C_SRC_FILES} PROPERTIES LANGUAGE CXX)
	add_library(Hy3S_Msg STATIC ${C_SRC_FILES})
	list(APPEND DEP_LIBS Hy3S_Msg ${JMS_LIBS})
	if (MSVC)
		list(APPEND DEP_LIBS Advapi32)
	elseif (UNIX)
		add_definitions(-DUNIX)
		if (LINUX_32bit_BINARIES)
	 		add_definitions(-DLINUX)
		elseif (LINUX_64bit_BINARIES)
			add_definitions(-DLINUX64)
		endif()
	endif()	
endif()

#set (CMAKE_Fortran_COMPILER ${IFORT_EXECUTABLE})
if (APPLE OR LINUX)
	#make sure preprocessor called
	set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fpp")
endif (APPLE OR LINUX)

set(EXE_FILE_EM Hybrid_EM)
if (ARCH_64bit) 
	set(EXE_FILE_EM ${EXE_FILE_EM}_x64)
endif()
add_executable(${EXE_FILE_EM} ${F90_SRC_FILES})
target_link_libraries(${EXE_FILE_EM} ${DEP_LIBS})
set_target_properties(${EXE_FILE_EM} PROPERTIES LINKER_LANGUAGE Fortran)

set(EXE_FILE_MIL Hybrid_MIL)
if (ARCH_64bit) 
	set(EXE_FILE_MIL ${EXE_FILE_MIL}_x64)
endif()
add_executable(${EXE_FILE_MIL} ${F90_SRC_FILES})
target_link_libraries(${EXE_FILE_MIL} ${DEP_LIBS})
set_target_properties(${EXE_FILE_MIL} PROPERTIES LINKER_LANGUAGE Fortran)
set_property(TARGET ${EXE_FILE_MIL} PROPERTY COMPILE_DEFINITIONS MILSTEIN)

set(EXE_FILE_MIL_Adaptive Hybrid_MIL_Adaptive)
if (ARCH_64bit) 
	set(EXE_FILE_MIL_Adaptive ${EXE_FILE_MIL_Adaptive}_x64)
endif()
add_executable(${EXE_FILE_MIL_Adaptive} ${F90_SRC_FILES})
target_link_libraries(${EXE_FILE_MIL_Adaptive} ${DEP_LIBS})
set_target_properties(${EXE_FILE_MIL_Adaptive} PROPERTIES LINKER_LANGUAGE Fortran)
set_property(TARGET ${EXE_FILE_MIL_Adaptive} PROPERTY COMPILE_DEFINITIONS MILSTEIN ADAPTIVE)

install(TARGETS ${EXE_FILE_EM} ${EXE_FILE_MIL} ${EXE_FILE_MIL_Adaptive}
        RUNTIME DESTINATION ${OPTION_EXE_DIRECTORY})
