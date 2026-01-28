find_package(cpptrace GLOBAL)

if(NOT cpptrace_FOUND) 
    include(FetchContent;)
    FetchContent_Declare(
      cpptrace
      GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
      GIT_TAG        v0.7.0 # <HASH or TAG>
    )
    FetchContent_MakeAvailable(cpptrace)
endif()
