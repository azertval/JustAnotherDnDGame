// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Diagnostics/Log.h"

/**
 * @file Core/Physics/PhysicsLog.h
 * @brief Macros de journalisation de la catégorie « Physics ».
 *
 * Sous-catégorie de log du module `Core` (voir `HMI/HmiLog.h` pour le modèle). Réservée aux
 * événements **rares** (transitions d'état ambiguës ou notables, comme un calage sur une tuile de
 * plafond) ; jamais aux opérations exécutées à chaque pas fixe pour chaque entité — `CORE_LOG_*`
 * (`Core/CoreLog.h`) applique la même restriction, voir sa documentation.
 */

#define PHYSICS_LOG_TRACE(message) JADG_LOG_TRACE("Physics", message)
#define PHYSICS_LOG_INFO(message) JADG_LOG_INFO("Physics", message)
#define PHYSICS_LOG_WARNING(message) JADG_LOG_WARNING("Physics", message)
#define PHYSICS_LOG_ERROR(message) JADG_LOG_ERROR("Physics", message)
