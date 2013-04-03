# Used for GResource.
#
# resource_dir: Directory where the .gresource.xml is located.
# resource_file: Filename of the .gresource.xml file (just the
# filename, not the complete path).
# output_dir: Directory where the C output file is written.
# output_file: This variable will be set with the complete path of the
# output C file.

function (gresource resource_dir resource_file output_dir output_source output_header)

_pkgconfig_invoke("glib-2.0" GLIB2 PREFIX
  "" "--variable=prefix")
find_program(GLIB_COMPILE_RESOURCES
             NAMES glib-compile-resources
             HINTS ${GLIB2_PREFIX})

if (NOT GLIB_COMPILE_RESOURCES)
message(FATAL "Could not find glib-compile-resources")
endif()

# Get the output file path
get_filename_component (resource_name ${resource_file} NAME_WE)
set (output_c "${output_dir}/${resource_name}-resources.c")
set (output_h "${output_dir}/${resource_name}-resources.h")
set (${output_source} ${output_c} PARENT_SCOPE)
set (${output_header} ${output_h} PARENT_SCOPE)

# Get the dependencies of the gresource
execute_process (
OUTPUT_VARIABLE _files
WORKING_DIRECTORY ${resource_dir}
COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies ${resource_file}
)

string (REPLACE "\n" ";" files ${_files})

set (depends "")
foreach (cur_file ${files})
list (APPEND depends "${resource_dir}/${cur_file}")
endforeach ()

# Command to compile the resources
add_custom_command (
OUTPUT ${output_c}
DEPENDS "${resource_dir}/${resource_file}" ${depends}
WORKING_DIRECTORY ${resource_dir}
COMMAND ${GLIB_COMPILE_RESOURCES} --generate-source --manual-register --target=${output_c} ${resource_file}
)

# Command to compile the resources
add_custom_command (
OUTPUT ${output_h}
DEPENDS "${resource_dir}/${resource_file}" ${depends}
WORKING_DIRECTORY ${resource_dir}
COMMAND ${GLIB_COMPILE_RESOURCES} --generate-header --manual-register --target=${output_h} ${resource_file}
)
endfunction ()
