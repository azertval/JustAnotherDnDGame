# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Build-time guard for qmltyperegistrar output.
if(NOT DEFINED EXPECTED_QMLTYPES OR EXPECTED_QMLTYPES STREQUAL "")
    message(FATAL_ERROR "Jadg.Runtime: qt_query_qml_module() did not return a qmltypes path.")
endif()

if(NOT EXISTS "${EXPECTED_QMLTYPES}")
    message(FATAL_ERROR
        "Jadg.Runtime: qmltyperegistrar metadata was not generated: ${EXPECTED_QMLTYPES}")
endif()

file(SIZE "${EXPECTED_QMLTYPES}" _qmltypes_size)
if(_qmltypes_size EQUAL 0)
    message(FATAL_ERROR
        "Jadg.Runtime: generated qmltypes metadata is empty: ${EXPECTED_QMLTYPES}")
endif()

message(STATUS "Jadg.Runtime: validated qmltypes metadata (${EXPECTED_QMLTYPES}).")
