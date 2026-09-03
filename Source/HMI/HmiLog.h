// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file HMI/HmiLog.h
 * @brief Macros de journalisation de la catégorie « HMI ».
 *
 * Chaque module possède son propre en-tête de ce type (sa « sous-catégorie » de
 * log). Modèle à dupliquer pour les autres modules (Core, Ecs, Graphics…).
 */

#define HMI_LOG_TRACE(message) JADG_LOG_TRACE("HMI", message)
#define HMI_LOG_INFO(message) JADG_LOG_INFO("HMI", message)
#define HMI_LOG_WARNING(message) JADG_LOG_WARNING("HMI", message)
#define HMI_LOG_ERROR(message) JADG_LOG_ERROR("HMI", message)
