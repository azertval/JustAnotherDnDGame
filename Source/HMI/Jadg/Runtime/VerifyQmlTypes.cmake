# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Build-time guard for qmltyperegistrar output. Shared by every QML module that calls this script,
# so messages must not hardcode a single module's name (MODULE_LABEL identifies the caller).
if(NOT DEFINED MODULE_LABEL OR MODULE_LABEL STREQUAL "")
    set(MODULE_LABEL "qmltypes check")
endif()

if(NOT DEFINED EXPECTED_QMLTYPES OR EXPECTED_QMLTYPES STREQUAL "")
    message(FATAL_ERROR "${MODULE_LABEL}: qt_query_qml_module() did not return a qmltypes path.")
endif()

if(NOT EXISTS "${EXPECTED_QMLTYPES}")
    message(FATAL_ERROR
        "${MODULE_LABEL}: qmltyperegistrar metadata was not generated: ${EXPECTED_QMLTYPES}")
endif()

file(SIZE "${EXPECTED_QMLTYPES}" _qmltypes_size)
if(_qmltypes_size EQUAL 0)
    message(FATAL_ERROR
        "${MODULE_LABEL}: generated qmltypes metadata is empty: ${EXPECTED_QMLTYPES}")
endif()

message(STATUS "${MODULE_LABEL}: validated qmltypes metadata (${EXPECTED_QMLTYPES}).")
