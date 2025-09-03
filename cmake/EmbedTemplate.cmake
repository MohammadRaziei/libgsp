# Helper: turn "figure.mustache.html" into "html_mustache_figure"
function(make_identifier_from_filename INPUT OUT_VAR)
    get_filename_component(_FNAME "${INPUT}" NAME)
    string(REPLACE "." ";" _PARTS "${_FNAME}")
    string(REPLACE "-" "_" _PARTS "${_PARTS}")
    list(REVERSE _PARTS)
    string(JOIN "_" RESULT ${_PARTS})
    set(${OUT_VAR} "${RESULT}" PARENT_SCOPE)
endfunction()

# Function: embed one template file into a header
function(embed_template INPUT OUTPUT)
    set(TEMPLATES_HEADER_IN ${PROJECT_SOURCE_DIR}/cmake/templates.in.h)
    if(NOT EXISTS "${TEMPLATES_HEADER_IN}")
        message(FATAL_ERROR "embed_template: TEMPLATES_HEADER_IN not set or missing: ${TEMPLATES_HEADER_IN}")
    endif()

    if(NOT EXISTS "${INPUT}")
        message(FATAL_ERROR "embed_template: INPUT not found: ${INPUT}")
    endif()

    # Read template content
    file(READ "${INPUT}" CONTENT)

    # Vars used by templates.in.h
    get_filename_component(INPUT_BASENAME "${INPUT}" NAME)
    make_identifier_from_filename("${INPUT}" VAR)

    # Ensure output dir exists
    get_filename_component(_OUTDIR "${OUTPUT}" DIRECTORY)
    file(MAKE_DIRECTORY "${_OUTDIR}")

    # Generate the header from the template (configure-time)
    configure_file("${TEMPLATES_HEADER_IN}" "${OUTPUT}" @ONLY)
endfunction()

# Where your source templates live
set(TEMPLATES_DIR ${PROJECT_SOURCE_DIR}/src/sources/templates)

# Collect all files (reconfigure when the set changes)
file(GLOB TEMPLATE_FILES CONFIGURE_DEPENDS "${TEMPLATES_DIR}/*")

# Generate one header per template into the build tree
set(GENERATED_TEMPLATE_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_TEMPLATE_HEADERS)

foreach(TPL IN LISTS TEMPLATE_FILES)
    make_identifier_from_filename("${TPL}" VAR_NAME)
    set(OUT_HDR "${GENERATED_TEMPLATE_HEADERS_DIR}/templates/${VAR_NAME}.h")

    embed_template("${TPL}" "${OUT_HDR}")
    list(APPEND GENERATED_TEMPLATE_HEADERS "${OUT_HDR}")
endforeach()
