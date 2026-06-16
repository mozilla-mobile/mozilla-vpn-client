# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# In-tree re-implementation of Qt's qt_target_qml_from_lottie.
#
# That convenience command is only defined by the LottieTools package shipped
# with Qt 6.11+, and finding the LottieTools package does NOT imply the command
# exists: Qt 6.10 ships the runtime VectorImageHelpers (so Qt6LottieTools_FOUND
# is true) but neither the lottietoqml generator tool nor the macro. The genuine
# capability signal is the Qt6::lottietoqml tool target, so we drive that tool
# directly instead of depending on the (versionless) Qt command being present.
#
# We always generate with the curve renderer (-c) and path optimization (-p).
# Call mz_target_qml_from_lottie() only when TARGET Qt6::lottietoqml exists;
# otherwise use the checked-in generated QML.
function(mz_target_qml_from_lottie target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "FILES;OUTPUTS")

    if(NOT arg_FILES OR NOT arg_OUTPUTS)
        message(FATAL_ERROR "mz_target_qml_from_lottie: FILES and OUTPUTS are required.")
    endif()

    set(generated_files "")
    math(EXPR file_index "0")
    foreach(filepath IN LISTS arg_FILES)
        get_filename_component(file_absolute "${filepath}" ABSOLUTE)
        list(GET arg_OUTPUTS ${file_index} output_file)
        set(result "${CMAKE_CURRENT_BINARY_DIR}/.qt/lottietoqml/${output_file}")

        add_custom_command(
            OUTPUT "${result}"
            COMMAND Qt6::lottietoqml -c -p "${file_absolute}" "${result}"
            DEPENDS "${file_absolute}" Qt6::lottietoqml
            VERBATIM
        )
        # Map the build-dir output back to its module-relative resource path so
        # it lands at e.g. animations/GlobeAnimation.qml in the QML module.
        set_source_files_properties("${result}" PROPERTIES QT_RESOURCE_ALIAS "${output_file}")
        list(APPEND generated_files "${result}")
        math(EXPR file_index "${file_index}+1")
    endforeach()

    qt_target_qml_sources(${target} QML_FILES ${generated_files} NO_LINT)
endfunction()
