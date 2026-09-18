// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class QSoundEffect;

/**
 * @file HMI/Audio/AudioEngine.h
 * @brief Socle de lecture audio : enveloppe fine autour de `QSoundEffect` (`EX-REN-047`).
 */

namespace hmi {

/**
 * @brief Lecture d'échantillons courts à faible latence, avec volume global et repli muet.
 *
 * Enveloppe fine autour de `QSoundEffect` (Qt Multimedia) : ouverture du périphérique de sortie à
 * la construction, un `QSoundEffect` par échantillon préchargé, arrêt propre à la destruction
 * (RAII, `EX-NFR-041`). Aucune exception ne franchit cette frontière (`EX-NFR-040`) : un
 * périphérique absent, un fichier introuvable ou une lecture demandée alors que le moteur est
 * « muet » ne font jamais rien de plus qu'un avertissement journalisé une seule fois.
 *
 * **Aucune dépendance dans `Core`** : cette classe vit exclusivement dans `HMI`
 * (`EX-ARCH-012`, `EX-NFR-010`). Aujourd'hui, seul le volume de l'option Audio le règle : aucun
 * son n'est encore livré.
 */
class AudioEngine {
public:
    /// Étiquette du constructeur qui force l'état « muet », sans tenter d'ouvrir de périphérique —
    /// réservé aux tests : le comportement en l'absence de périphérique doit être vérifiable sans
    /// dépendre du matériel audio réel de la machine qui exécute les tests.
    enum class ForceMuted { Yes };

    /// Ouvre le périphérique de sortie par défaut. Un échec (aucun périphérique) place le moteur en
    /// état muet et journalise un avertissement une seule fois — jamais une exception.
    AudioEngine();

    /// Construit un moteur déjà muet, sans toucher au périphérique réel (tests, `ForceMuted::Yes`).
    explicit AudioEngine(ForceMuted);

    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;

    /// @return true si le moteur ne produira aucun son (périphérique absent, ou forcé pour les
    ///         tests).
    [[nodiscard]] bool muted() const noexcept {
        return _muted;
    }

    /**
     * @brief Règle le volume global, appliqué immédiatement à tous les échantillons préchargés.
     * @param volume Volume voulu. Hors [0, 1], ramené à l'extrémité la plus proche — jamais
     *        propagé tel quel (une valeur négative ou explosée ne doit jamais atteindre
     *        `QSoundEffect`).
     */
    void setVolume(float volume);

    /// @return Le volume global courant, toujours dans [0, 1].
    [[nodiscard]] float volume() const noexcept {
        return _volume;
    }

    /**
     * @brief Précharge un échantillon sous un identifiant logique.
     *
     * À appeler au démarrage pour chaque son à jouer : `QSoundEffect` charge son fichier de façon
     * asynchrone, jouer immédiatement après
     * construction ne produit rien. Précharger ici, jamais au premier déclenchement. Prépare en
     * réalité `MAX_INSTANCES_PER_EVENT` échantillons identiques (une petite réserve), pour qu'un
     * même événement déclenché en rafale se recouvre sans s'interrompre lui-même de façon audible.
     * @param id Identifiant logique (ex. "coup"). Un second appel avec le même identifiant
     *        remplace la réserve précédente.
     * @param file Chemin du fichier WAV. Un fichier absent ne fait pas échouer l'appel : la
     *        lecture ultérieure de cet identifiant restera silencieuse.
     */
    void preload(const std::string& id, const std::filesystem::path& file);

    /**
     * @brief Joue l'échantillon préchargé sous cet identifiant.
     *
     * Sans effet si le moteur est muet, si @p id n'a jamais été préchargé, ou si le fichier
     * associé n'a pas pu être chargé — jamais une erreur pour l'appelant (`EX-NFR-040`). Choisit
     * une instance parmi la réserve préchargée (tourniquet) : un même événement déclenché plus de
     * `MAX_INSTANCES_PER_EVENT` fois dans le même instant réutilise la plus ancienne plutôt que
     * d'empiler indéfiniment des lectures superposées.
     * @param id Identifiant préchargé par `preload`.
     */
    void play(const std::string& id);

private:
    /// Nombre d'instances préchargées par événement — plafonne le recouvrement audible d'un même
    /// son déclenché en rafale, sans l'empêcher complètement.
    static constexpr std::size_t MAX_INSTANCES_PER_EVENT = 3;

    struct Sample {
        std::vector<std::unique_ptr<QSoundEffect>> instances;
        std::size_t nextInstance = 0;  ///< Tourniquet : prochaine instance a reutiliser.
    };

    std::unordered_map<std::string, Sample> _samples;
    float _volume = 1.0f;
    bool _muted = false;
};

}  // namespace hmi
