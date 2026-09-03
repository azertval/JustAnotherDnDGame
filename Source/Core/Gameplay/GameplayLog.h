// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file Core/Gameplay/GameplayLog.h
 * @brief Macros de journalisation de la catégorie « Gameplay ».
 *
 * Sous-catégorie de log du module `Gameplay` (voir `HMI/HmiLog.h` pour le modèle). Réservée aux
 * événements rares (changement d'état d'un mécanisme), jamais aux opérations exécutées à chaque
 * pas fixe.
 */

#define GAMEPLAY_LOG_TRACE(message) JADG_LOG_TRACE("Gameplay", message)
#define GAMEPLAY_LOG_INFO(message) JADG_LOG_INFO("Gameplay", message)
#define GAMEPLAY_LOG_WARNING(message) JADG_LOG_WARNING("Gameplay", message)
#define GAMEPLAY_LOG_ERROR(message) JADG_LOG_ERROR("Gameplay", message)
