// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file pch.h
 * @brief En-têtes précompilés : includes lourds et stables, partagés par les modules.
 *
 * N'y placer que des en-têtes rarement modifiés (bibliothèque standard, puis
 * plus tard Windows/DirectX). Ne jamais y mettre d'en-tête du projet en cours
 * d'évolution : cela annulerait le bénéfice de compilation.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
