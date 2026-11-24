find_program(DOTNET_COMMAND "dotnet" REQUIRED)

if(WIN32)
   find_program(DOTNET_T4_COMMAND "t4" REQUIRED)
endif()
