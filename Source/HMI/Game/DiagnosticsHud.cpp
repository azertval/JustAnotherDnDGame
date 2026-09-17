// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Game/DiagnosticsHud.h"

#include <array>
#include <cstdio>

#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Remplace la premiere occurrence de placeholder ("%1", "%2"...) par value (pas de dependance Qt,
// meme discipline que hmi::gameHudLines).
std::string formatPlaceholder(const std::string& templateText, const std::string& placeholder,
                              const std::string& value) {
    const std::size_t position = templateText.find(placeholder);
    if (position == std::string::npos) {
        return templateText;
    }
    std::string result = templateText;
    result.replace(position, placeholder.size(), value);
    return result;
}

// Remplace "%1" (patron a un seul compte).
std::string formatCount(const std::string& templateText, const std::string& value) {
    return formatPlaceholder(templateText, "%1", value);
}

// Remplace "%1" PUIS "%2" -- ligne des primitives (composees / soumises).
std::string formatTwoCounts(const std::string& templateText, int first, int second) {
    return formatPlaceholder(formatPlaceholder(templateText, "%1", std::to_string(first)), "%2",
                             std::to_string(second));
}

// Une decimale, sans dependance Qt/locale (std::to_string tronque a l'entier pour un float).
std::string formatOneDecimal(float value) {
    std::array<char, 32> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "%.1f", static_cast<double>(value));
    return {buffer.data()};
}

}  // namespace

// Ajoute la duree d'une image ecoulee et purge les echantillons sortis de la fenetre (voir
// en-tete).
void FrameRateAverage::addSample(float deltaSeconds) noexcept {
    if (deltaSeconds <= 0.0F) {
        return;  // robustesse : un appelant ne doit jamais en produire (cf. contrat de l'en-tete).
    }
    _recentDeltas.push_back(deltaSeconds);
    _accumulatedSeconds += deltaSeconds;
    while (_accumulatedSeconds > DIAGNOSTICS_FPS_WINDOW_SECONDS && _recentDeltas.size() > 1) {
        _accumulatedSeconds -= _recentDeltas.front();
        _recentDeltas.pop_front();
    }
}

// Cadence moyenne sur la fenetre courante (voir en-tete : 0 si aucun echantillon).
float FrameRateAverage::framesPerSecond() const noexcept {
    if (_recentDeltas.empty() || _accumulatedSeconds <= 0.0F) {
        return 0.0F;
    }
    return static_cast<float>(_recentDeltas.size()) / _accumulatedSeconds;
}

// Oublie tous les echantillons (voir en-tete).
void FrameRateAverage::reset() noexcept {
    _recentDeltas.clear();
    _accumulatedSeconds = 0.0F;
}

// Compose les lignes du compteur de diagnostic a partir de valeurs deja mesurees (voir en-tete).
std::vector<std::string> composeDiagnosticsHudLines(const DiagnosticsMeasurements& measurements,
                                                    const Localization& localization) {
    std::vector<std::string> lines;
    lines.push_back(
        formatCount(localization.text("diag.fps"), formatOneDecimal(measurements.framesPerSecond)));
    lines.push_back(formatTwoCounts(localization.text("diag.primitives"),
                                    measurements.sceneStatistics.considered,
                                    measurements.sceneStatistics.submitted));
    lines.push_back(formatCount(localization.text("diag.batches"),
                                std::to_string(measurements.sceneStatistics.batches)));
    lines.push_back(
        formatCount(localization.text("diag.steps"), std::to_string(measurements.simulationSteps)));
    return lines;
}

}  // namespace hmi
