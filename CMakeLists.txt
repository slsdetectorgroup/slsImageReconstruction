include_directories(
    ../slsDetectorCalibration
    ../slsReceiverSoftware/include
    ../slsDetectorSoftware/slsDetector 
    ../slsDetectorSoftware/slsDetectorAnalysis
    ../slsDetectorSoftware/commonFiles 
)
add_executable (image
    src/dummymain.cpp
)
if (${CBF_FOUND})
    include_directories(
        ${CBF_INCLUDE_DIRS}
    )
    add_executable (cbfMaker
        src/main_csaxs.cpp
    )
    target_link_libraries(cbfMaker
        ${CBF_LIBRARY}
    )
    add_executable (cbfMakerHalf
        src/main_half.cpp
    )
    add_executable (cbfMakerMulti
        src/main_csaxs_multi.cpp
    )
endif()



