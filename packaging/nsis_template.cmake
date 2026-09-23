# CPack's stock NSIS script adds to PATH with NSIS strings, which are capped at
# 1024 characters, so "Add to the system PATH" fails with "PATH too long" on
# most real machines. Generate a copy of CMake's own template with AddToPath
# and un.RemoveFromPath swapped for nsis_path_functions.nsh, and have CPack use
# it. Starting from the running CMake's template keeps the rest of the installer
# in step with whatever CPack version builds it.
#
# Sets BPQ_NSIS_TEMPLATE_DIR to the directory holding the new NSIS.template.in.

set(_bpq_stock_template "${CMAKE_ROOT}/Modules/Internal/CPack/NSIS.template.in")
file(READ "${_bpq_stock_template}" _bpq_template)
file(READ "${CMAKE_CURRENT_LIST_DIR}/nsis_path_functions.nsh" _bpq_path_functions)

# Cuts "Function <name>" ... "FunctionEnd" out of the template, leaving
# <marker> in its place.
function(_bpq_cut_nsis_function name marker)
    string(FIND "${_bpq_template}" "\nFunction ${name}\n" _start)
    if(_start EQUAL -1)
        string(FIND "${_bpq_template}" "\nFunction ${name}\r\n" _start)
    endif()
    if(_start EQUAL -1)
        message(FATAL_ERROR
            "${_bpq_stock_template} has no 'Function ${name}'; "
            "packaging/nsis_template.cmake needs updating for this CMake version.")
    endif()
    string(SUBSTRING "${_bpq_template}" 0 ${_start} _before)
    string(SUBSTRING "${_bpq_template}" ${_start} -1 _rest)
    string(FIND "${_rest}" "\nFunctionEnd" _end)
    math(EXPR _end "${_end} + 12")
    string(SUBSTRING "${_rest}" ${_end} -1 _after)
    set(_bpq_template "${_before}\n${marker}${_after}" PARENT_SCOPE)
endfunction()

_bpq_cut_nsis_function(AddToPath "")
_bpq_cut_nsis_function(un.RemoveFromPath "${_bpq_path_functions}")

set(BPQ_NSIS_TEMPLATE_DIR "${CMAKE_BINARY_DIR}/nsis-template")
file(WRITE "${BPQ_NSIS_TEMPLATE_DIR}/NSIS.template.in" "${_bpq_template}")
