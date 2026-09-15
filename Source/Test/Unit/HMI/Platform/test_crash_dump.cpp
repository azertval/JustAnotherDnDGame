// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crash_dump.cpp
 * @brief Minidump sur plantage (HMI/Platform/CrashDump) : nom du fichier et écriture réelle.
 *
 * Le filtre d'exception non attrapée ne se teste pas en processus : il termine le processus. Le
 * test de fumée de la release le prouve de bout en bout (`--crash-test`). Ici, ce qui le compose :
 * le nom du fichier, et l'écriture d'un dump lisible, avec et sans contexte d'exception.
 */

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>

#include <Windows.h>
#include <gtest/gtest.h>

#include "HMI/Platform/CrashDump.h"

namespace {

std::tm fixedTime() {
    std::tm time{};
    time.tm_year = 2026 - 1900;
    time.tm_mon = 8;  // septembre
    time.tm_mday = 15;
    time.tm_hour = 21;
    time.tm_min = 4;
    time.tm_sec = 7;
    return time;
}

/// Dossier temporaire propre au test, supprimé à la fin.
class CrashDumpTest : public ::testing::Test {
protected:
    void SetUp() override {
        directory_ = std::filesystem::temp_directory_path() /
                     ("jadg_crash_dump_" + std::to_string(GetCurrentProcessId()));
        std::filesystem::remove_all(directory_);
    }
    void TearDown() override {
        std::error_code ignored;
        std::filesystem::remove_all(directory_, ignored);
    }

    std::filesystem::path directory_;
};

/// Les quatre premiers octets d'un minidump : la signature « MDMP ».
std::string signature(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    std::string bytes(4, '\0');
    stream.read(bytes.data(), 4);
    return bytes;
}

/// Filtre SEH : écrit le dump avec le contexte de l'exception, puis la déclare traitée.
int dumpThenHandle(const std::filesystem::path* path, EXCEPTION_POINTERS* exception,
                   bool* written) {
    *written = hmi::writeMiniDump(*path, exception);
    return EXCEPTION_EXECUTE_HANDLER;
}

/// Lève une exception structurée et l'attrape dans un filtre qui écrit le dump. Fonction à part :
/// `__try` est interdit dans une fonction qui détruit des objets C++ (C2712), comme un corps de
/// test.
bool dumpFromStructuredException(const std::filesystem::path* path) {
    bool written = false;
    __try {
        RaiseException(hmi::kFatalErrorExceptionCode, 0, 0, nullptr);
    } __except (dumpThenHandle(path, GetExceptionInformation(), &written)) {}
    return written;
}

}  // namespace

/**
 * @brief Le nom d'un minidump porte l'application, la version et l'heure du plantage.
 * \castest{<b>Le nom d'un minidump porte l'application, la version et l'heure du plantage.</b><br/>
 * \tcat Unitaire · Crash Dump<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer le nom pour JustAnotherDnDGame 0.0.4 au 15/09/2026 21:04:07.<br/>
 * \tattendu JustAnotherDnDGame_0.0.4_20260915_210407.dmp : l'archive de symboles se retrouve sans
 * ouvrir le fichier.
 * }
 */
TEST(CrashDumpFileName, PorteApplicationVersionEtHorodatage) {
    EXPECT_EQ(hmi::crashDumpFileName("JustAnotherDnDGame", "0.0.4", fixedTime()),
              "JustAnotherDnDGame_0.0.4_20260915_210407.dmp");
}

/**
 * @brief Les caractères hors nom de fichier sont remplacés dans le nom du minidump.
 * \castest{<b>Les caractères hors nom de fichier sont remplacés dans le nom du minidump.</b><br/>
 * \tcat Unitaire · Crash Dump<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer le nom pour « Level Editor » en version « 0.1.0+dev/x ».<br/>
 * \tattendu Espace, « + » et « / » deviennent « _ » ; le nom reste un fichier valide.
 * }
 */
TEST(CrashDumpFileName, RemplaceLesCaracteresHorsNomDeFichier) {
    EXPECT_EQ(hmi::crashDumpFileName("Level Editor", "0.1.0+dev/x", fixedTime()),
              "Level_Editor_0.1.0_dev_x_20260915_210407.dmp");
}

/**
 * @brief Un minidump de l'état courant s'écrit, dossier parent créé au besoin.
 * \castest{<b>Un minidump de l'état courant s'écrit, dossier parent créé au besoin.</b><br/>
 * \tcat Unitaire · Crash Dump<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Écrire un dump sans contexte d'exception dans un sous-dossier inexistant.<br/>2. Lire
 * les quatre premiers octets.<br/>
 * \tattendu Le fichier existe et commence par la signature MDMP.
 * }
 */
TEST_F(CrashDumpTest, EcritUnMinidumpSansExceptionEtCreeLeDossier) {
    const std::filesystem::path path = directory_ / "sous-dossier" / "etat.dmp";
    ASSERT_TRUE(hmi::writeMiniDump(path, nullptr));
    ASSERT_TRUE(std::filesystem::exists(path));
    EXPECT_EQ(signature(path), "MDMP");
}

/**
 * @brief Un minidump s'écrit avec le contexte d'une exception structurée.
 * \castest{<b>Un minidump s'écrit avec le contexte d'une exception structurée.</b><br/>
 * \tcat Unitaire · Crash Dump<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lever une exception structurée, l'attraper dans un filtre SEH qui écrit le
 * dump.<br/>2. Lire la signature et la taille.<br/>
 * \tattendu Le dump est écrit (signature MDMP) et porte plus qu'un en-tête.
 * }
 */
TEST_F(CrashDumpTest, EcritUnMinidumpAvecLeContexteDUneException) {
    const std::filesystem::path path = directory_ / "exception.dmp";
    ASSERT_TRUE(dumpFromStructuredException(&path));
    EXPECT_EQ(signature(path), "MDMP");
    EXPECT_GT(std::filesystem::file_size(path), 1024U);
}

/**
 * @brief Un chemin de minidump impossible échoue sans exception ni plantage.
 * \castest{<b>Un chemin de minidump impossible échoue sans exception ni plantage.</b><br/>
 * \tcat Unitaire · Crash Dump<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un dump sous un nom interdit par Windows (« a?b.dmp »).<br/>
 * \tattendu L'écriture rend false ; le filtre de plantage peut journaliser l'échec et conclure.
 * }
 */
TEST_F(CrashDumpTest, EchoueSansLeverSurUnCheminImpossible) {
    // Un nom de fichier interdit par Windows : l'écriture échoue, sans exception ni plantage.
    EXPECT_FALSE(hmi::writeMiniDump(directory_ / "a?b.dmp", nullptr));
}
