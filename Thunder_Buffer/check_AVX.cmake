# Check if SSE instructions are available on the machine where
# the project is compiled.

MACRO (FindSSE)
  IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    EXEC_PROGRAM(bash ARGS "-c \"cat /proc/cpuinfo\" | grep avx2" OUTPUT_VARIABLE CPUINFO)
    STRING(FIND ${CPUINFO} "avx2" pos)
    if (${pos} GREATER -1)
      MESSAGE(STATUS "AVX2 Found")
      SET(AVX2 True)
    ENDIF()
  ENDIF()
ENDMACRO(FindSSE)
