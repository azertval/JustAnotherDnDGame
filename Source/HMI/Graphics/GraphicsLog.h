// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file HMI/Graphics/GraphicsLog.h
 * @brief Macros de journalisation de la catégorie « Graphics ».
 *
 * Sous-catégorie de log du rendu (voir `HMI/HmiLog.h` pour le modèle). À réserver aux
 * événements de cycle de vie (création de ressources, redimensionnement) ; jamais dans les
 * chemins de dessin exécutés à chaque frame.
 */

#define GRAPHICS_LOG_TRACE(message) JADG_LOG_TRACE("Graphics", message)
#define GRAPHICS_LOG_INFO(message) JADG_LOG_INFO("Graphics", message)
#define GRAPHICS_LOG_WARNING(message) JADG_LOG_WARNING("Graphics", message)
#define GRAPHICS_LOG_ERROR(message) JADG_LOG_ERROR("Graphics", message)
