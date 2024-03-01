include(CMakeParseArguments)

function(add_flatbuffer_schema)
  cmake_parse_arguments(ADD_FB_SCHEMA_ARGS
    ""
    "TARGET;FLATC_EXE;OUTPUT_DIR;FLATC_LANGUAGE;GENERATED_FILES;FLATC_EXTRA_SCHEMA_ARGS"
    "SCHEMAS"
    ${ARGN}
  )

  # check that FLATC_LANGAUGE is either undefined (in which case set it to CXX), or is in any of
  # { CXX, CSharp }
  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE)
    set(ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE "CXX")
  endif()

  message(STATUS "ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE: ${ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE}")

  set (RL_FLATC_VALID_LANGUAGES "CXX" "CSharp")

  if(NOT ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE IN_LIST RL_FLATC_VALID_LANGUAGES)
    message(FATAL_ERROR "FLATC_LANGUAGE must be either CXX or CSharp")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_TARGET)
    message(FATAL_ERROR "Missing TARGET argument to build_flatbuffers")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_FLATC_EXE)
    message(FATAL_ERROR "Missing FLATC_EXE argument to build_flatbuffers")
  endif()

  if(NOT EXISTS ${ADD_FB_SCHEMA_ARGS_FLATC_EXE})
    message(FATAL_ERROR "FLATC_EXE ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} does not exist")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_OUTPUT_DIR)
    message(FATAL_ERROR "Missing OUTPUT_DIR argument to build_flatbuffers")
  endif()

  if(NOT DEFINED ADD_FB_SCHEMA_ARGS_SCHEMAS)
    message(FATAL_ERROR "Missing SCHEMAS argument to build_flatbuffers")
  endif()

  set(FLATC_SCHEMA_ARGS --gen-mutable)
  if(DEFINED ADD_FB_SCHEMA_ARGS_FLATC_EXTRA_SCHEMA_ARGS)
    set(FLATC_SCHEMA_ARGS ${ADD_FB_SCHEMA_ARGS_FLATC_EXTRA_SCHEMA_ARGS} ${FLATC_SCHEMA_ARGS})
  endif()

  foreach(schema IN ITEMS ${ADD_FB_SCHEMA_ARGS_SCHEMAS})
    get_filename_component(filename ${schema} NAME_WE)

    # if the target language is CXX, we generate a header file
    if(${ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE} STREQUAL "CXX")
      set(generated_file_name ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR}/${filename}_generated.h)
      add_custom_command(
          OUTPUT ${generated_file_name}
          COMMAND ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} ${FLATC_SCHEMA_ARGS} -o ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR} -c ${schema}
          DEPENDS ${schema}
      )
      list(APPEND ALL_SCHEMAS ${generated_file_name})
    endif()

    # if the target language is CSharp, we generate a C# file
    if(${ADD_FB_SCHEMA_ARGS_FLATC_LANGUAGE} STREQUAL "CSharp")
      set(generated_file_name ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR}/${filename}_generated.cs)
      add_custom_command(
          OUTPUT ${generated_file_name}
          COMMAND ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} ${FLATC_SCHEMA_ARGS} -o ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR} --csharp ${schema}
          DEPENDS ${schema}
      )
      list(APPEND ALL_SCHEMAS ${generated_file_name})
    endif()

    if (DEFINED ADD_FB_SCHEMA_ARGS_GENERATED_FILES)
      list(APPEND ${ADD_FB_SCHEMA_ARGS_GENERATED_FILES} ${generated_file_name})
    endif ()

    # set(generated_file_name ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR}/${filename}_generated.h)
    # add_custom_command(
    #     OUTPUT ${generated_file_name}
    #     COMMAND ${ADD_FB_SCHEMA_ARGS_FLATC_EXE} ${FLATC_SCHEMA_ARGS} -o ${ADD_FB_SCHEMA_ARGS_OUTPUT_DIR} -c ${schema}
    #     DEPENDS ${schema}
    # )
    # list(APPEND ALL_SCHEMAS ${generated_file_name})
  endforeach()

  add_custom_target(${ADD_FB_SCHEMA_ARGS_TARGET} DEPENDS ${ALL_SCHEMAS})
endfunction()
