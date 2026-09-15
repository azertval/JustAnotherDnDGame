// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Audio/AudioEngine.h"

#include <QAudioDevice>
#include <QMediaDevices>
#include <QSoundEffect>
#include <QString>
#include <QUrl>
#include <algorithm>

#include "HMI/Audio/AudioLog.h"

namespace hmi {

namespace {
[[nodiscard]] float clampVolume(float volume) noexcept {
    return std::clamp(volume, 0.0f, 1.0f);
}
}  // namespace

AudioEngine::AudioEngine() {
    // QMediaDevices::defaultAudioOutput() renvoie un QAudioDevice nul si aucun périphérique de
    // sortie n'est disponible (machine sans carte son, pilote absent) : c'est le seul signal
    // fiable avant toute tentative de lecture, vérifiable sans construire le moindre QSoundEffect.
    if (QMediaDevices::defaultAudioOutput().isNull()) {
        _muted = true;
        AUDIO_LOG_WARNING("Aucun peripherique de sortie audio detecte : moteur audio muet.");
    }
}

AudioEngine::AudioEngine(ForceMuted) : _muted(true) {}

AudioEngine::~AudioEngine() = default;

void AudioEngine::setVolume(float volume) {
    _volume = clampVolume(volume);
    for (auto& entry : _samples) {
        for (auto& instance : entry.second.instances) {
            instance->setVolume(_volume);
        }
    }
}

void AudioEngine::preload(const std::string& id, const std::filesystem::path& file) {
    if (_muted) {
        return;
    }
    const QUrl source = QUrl::fromLocalFile(QString::fromStdString(file.string()));
    Sample sample;
    sample.instances.reserve(MAX_INSTANCES_PER_EVENT);
    for (std::size_t i = 0; i < MAX_INSTANCES_PER_EVENT; ++i) {
        auto effect = std::make_unique<QSoundEffect>();
        effect->setSource(source);
        effect->setVolume(_volume);
        sample.instances.push_back(std::move(effect));
    }
    _samples[id] = std::move(sample);
}

void AudioEngine::play(const std::string& id) {
    if (_muted) {
        return;
    }
    const auto it = _samples.find(id);
    if (it == _samples.end() || it->second.instances.empty()) {
        return;
    }
    Sample& sample = it->second;
    sample.instances[sample.nextInstance]->play();
    sample.nextInstance = (sample.nextInstance + 1) % sample.instances.size();
}

}  // namespace hmi
