// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file HMI/Audio/AudioLog.h
 * @brief Macros de journalisation de la catégorie « Audio ».
 *
 * Sous-catégorie de log de l'audio (voir `HMI/HmiLog.h` pour le modèle).
 */

#define AUDIO_LOG_TRACE(message) JADG_LOG_TRACE("Audio", message)
#define AUDIO_LOG_INFO(message) JADG_LOG_INFO("Audio", message)
#define AUDIO_LOG_WARNING(message) JADG_LOG_WARNING("Audio", message)
#define AUDIO_LOG_ERROR(message) JADG_LOG_ERROR("Audio", message)
