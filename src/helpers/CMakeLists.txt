
if ( HAVE_HYPERSCAN )
    set(HYPER_HEADERS
        hyper_scratch_allocator.h
        hyper_search.h
    )
    set(HYPER_SOURCES
        hyper_scratch_allocator.cc
        hyper_search.cc
    )
endif ()

set (HELPERS_INCLUDES
    ${HYPER_HEADERS}
    base64_encoder.h
    boyer_moore_search.h
    literal_search.h
    scratch_allocator.h
)

add_library (helpers OBJECT
    ${HELPERS_INCLUDES}
    ${HYPER_SOURCES}
    base64_encoder.cc
    boyer_moore_search.cc
    chunk.cc
    chunk.h
    directory.cc
    directory.h
    discovery_filter.cc
    discovery_filter.h
    flag_context.h
    literal_search.cc
    markup.cc
    markup.h
    process.cc
    process.h
    ring.h
    ring_logic.h
    scratch_allocator.cc
)

install (FILES ${HELPERS_INCLUDES}
    DESTINATION "${INCLUDE_INSTALL_PATH}/helpers"
)

add_catch_test( base64_encoder_test
    NO_TEST_SOURCE
    SOURCES
        base64_encoder.cc
)

add_subdirectory(test)

