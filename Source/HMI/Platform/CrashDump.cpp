// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Platform/CrashDump.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <system_error>
#include <utility>

#include <Windows.h>

// dbghelp.h exige Windows.h avant lui.
#include <DbgHelp.h>

#include "HMI/HmiLog.h"

namespace hmi {
namespace {

/// Réglages retenus par installCrashDumpWriter. Lus depuis le filtre : aucune allocation n'y a lieu
/// avant l'écriture du dump, le tas pouvant être la cause du plantage.
struct CrashDumpSettings {
    std::filesystem::path directory;
    std::string application;
    std::string version;
};

CrashDumpSettings& settings() {
    static CrashDumpSettings instance;
    return instance;
}

[[noreturn]] void raiseFatalError() {
    RaiseException(kFatalErrorExceptionCode, EXCEPTION_NONCONTINUABLE, 0, nullptr);
    // RaiseException avec EXCEPTION_NONCONTINUABLE ne revient pas ; si un débogueur reprend quand
    // même l'exécution, on ne poursuit pas dans un état incohérent.
    std::abort();
}

void onTerminate() {
    raiseFatalError();
}

void onPureCall() {
    raiseFatalError();
}

void onInvalidParameter(const wchar_t* /*expression*/, const wchar_t* /*function*/,
                        const wchar_t* /*file*/, unsigned int /*line*/, uintptr_t /*reserved*/) {
    raiseFatalError();
}

LONG WINAPI onUnhandledException(EXCEPTION_POINTERS* exception) {
    // Un seul dump : un second plantage pendant l'écriture (tas corrompu) terminerait sinon en
    // boucle, ou écraserait le premier fichier par un dump inutilisable.
    static LONG entered = 0;
    if (InterlockedExchange(&entered, 1) != 0) {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    const CrashDumpSettings& current = settings();
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    const std::filesystem::path path =
        current.directory / crashDumpFileName(current.application, current.version, local);
    const bool written = writeMiniDump(path, exception);

    const unsigned long code =
        exception != nullptr && exception->ExceptionRecord != nullptr
            ? static_cast<unsigned long>(exception->ExceptionRecord->ExceptionCode)
            : 0UL;
    std::array<char, 16> codeText{};
    std::snprintf(codeText.data(), codeText.size(), "0x%08lX", code);
    if (written) {
        HMI_LOG_ERROR(std::string("Plantage (exception ") + codeText.data() +
                      ") : minidump ecrit dans " + path.string());
    } else {
        HMI_LOG_ERROR(std::string("Plantage (exception ") + codeText.data() +
                      ") : minidump NON ecrit (" + path.string() + ")");
    }
    // FileLogSink écrit et vide son tampon à chaque message : la ligne est sur le disque.
    return EXCEPTION_EXECUTE_HANDLER;
}

}  // namespace

std::string crashDumpFileName(std::string_view application, std::string_view version,
                              const std::tm& localTime) {
    std::array<char, 32> stamp{};
    std::strftime(stamp.data(), stamp.size(), "%Y%m%d_%H%M%S", &localTime);

    std::string name;
    name.reserve(application.size() + version.size() + 24);
    const auto append = [&name](std::string_view part) {
        for (const char c : part) {
            const bool safe = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                              (c >= '0' && c <= '9') || c == '.' || c == '-';
            name.push_back(safe ? c : '_');
        }
    };
    append(application);
    name.push_back('_');
    append(version);
    name.push_back('_');
    name.append(stamp.data());
    name.append(".dmp");
    return name;
}

bool writeMiniDump(const std::filesystem::path& path, _EXCEPTION_POINTERS* exception) {
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }

    const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION exceptionInformation{};
    exceptionInformation.ThreadId = GetCurrentThreadId();
    exceptionInformation.ExceptionPointers = exception;
    exceptionInformation.ClientPointers = FALSE;

    // Piles, contexte, modules chargés et déchargés, et la mémoire que les piles désignent : de
    // quoi lire les variables locales et un objet pointé, pour quelques Mio plutôt que tout le tas.
    const auto type =
        static_cast<MINIDUMP_TYPE>(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory |
                                   MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
    const BOOL written =
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, type,
                          exception != nullptr ? &exceptionInformation : nullptr, nullptr, nullptr);
    CloseHandle(file);
    return written != FALSE;
}

void installCrashDumpWriter(std::filesystem::path directory, std::string application,
                            std::string version) {
    CrashDumpSettings& current = settings();
    current.directory = std::move(directory);
    current.application = std::move(application);
    current.version = std::move(version);

    SetUnhandledExceptionFilter(&onUnhandledException);
    std::set_terminate(&onTerminate);
    _set_purecall_handler(&onPureCall);
    _set_invalid_parameter_handler(&onInvalidParameter);
    // abort() ne passe par aucun des chemins ci-dessus : sans ce réglage, la CRT afficherait sa
    // boîte « abort() has been called » en Debug au lieu de laisser std::terminate conclure.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
}

void triggerCrashForTest() {
    // Adresse nulle lue par un pointeur volatile : l'optimiseur ne peut pas en déduire que la
    // lecture est indéfinie et la retirer.
    int* volatile target = nullptr;
    std::printf("%d\n", *target);
    std::abort();
}

}  // namespace hmi
