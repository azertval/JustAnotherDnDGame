// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_session_log.cpp
 * @brief Tests unitaires de la sérialisation des logs de session.
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Diagnostics/MemoryLogSink.h"
#include "HMI/Diagnostics/SessionLog.h"

/**
 * @brief Chaque message donne une ligne, dans l'ordre d'arrivée.
 * \castest{<b>Chaque message donne une ligne, dans l'ordre d'arrivée.</b><br/>
 * \tcat Unitaire · Session Log<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque message donne une ligne, dans l'ordre d'arrivée.
 * }
 */
TEST(SessionLogTest, SerialiseUneLigneParMessage) {
    const std::vector<core::MemoryLogSink::Entry> entries = {
        {core::LogLevel::Info, "premier message"},
        {core::LogLevel::Warning, "second message"},
    };

    EXPECT_EQ(hmi::serializeSessionLog(entries), "premier message\nsecond message\n");
}

/**
 * @brief Une session sans message produit un texte vide.
 * \castest{<b>Une session sans message produit un texte vide.</b><br/>
 * \tcat Unitaire · Session Log<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une session sans message produit un texte vide.
 * }
 */
TEST(SessionLogTest, VideDonneChaineVide) {
    EXPECT_EQ(hmi::serializeSessionLog({}), "");
}
