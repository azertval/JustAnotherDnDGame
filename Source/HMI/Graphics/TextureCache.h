// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/CacheRegistry.h"
#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/TileSilhouette.h"
#include "HMI/Graphics/TextureLoader.h"

/**
 * @file HMI/Graphics/TextureCache.h
 * @brief Registre de textures chargées à la demande par **nom logique** (`EX-REN-043`).
 */

namespace hmi {

/**
 * @brief Charge, valide et met en cache des textures GPU désignées par leur nom de fichier
 *        logique.
 *
 * `hmi::TextureAtlas` ne connaît qu'**une** texture fixe ; le programme d'habillage (`LOT-42` →
 * `LOT-55`) a besoin d'un nombre variable de textures indépendantes (skins, fonds, objets, décors,
 * spritesheets). Ce registre les charge au **premier accès**, les valide contre le contrat de leur
 * famille (`EX-REN-007`) et les conserve : les accès suivants renvoient la même ressource, sans
 * relire le disque.
 *
 * Ne réimplémente ni le décodage ni l'upload : il compose `hmi::AssetPaths::resolve` et
 * `hmi::loadTextureFromFile` (`LOT-39`), exactement comme `hmi::TextureAtlas`. Toutes les
 * ressources Direct3D sont détenues en RAII (`Microsoft::WRL::ComPtr`) et libérées à la
 * destruction du cache (`EX-NFR-041`).
 *
 * Un asset absent, illisible ou non conforme est un cas **attendu** (`EX-NFR-040`) : `get`
 * renvoie `nullptr` sans exception, après un avertissement journalisé **une seule fois** par nom
 * — le résultat négatif est mémorisé pour ne pas retenter une lecture disque à chaque image, et
 * pour ne pas noyer le journal. Les appelants qui veulent une texture *quoi qu'il arrive* passent
 * par `hmi::resolveOrPlaceholder`, point de résolution unique du repli en damier (`LOT-40`
 * TACHE-03).
 *
 * **Aucune éviction** : le cache grossit avec les assets rencontrés et n'est jamais purgé de
 * lui-même (décision de cadrage `LOT-40` — les assets sont bornés). `invalidate` sert au
 * **rechargement** (à chaud, `LOT-43` ; aperçu de l'atelier, `LOT-54`), pas à une politique
 * mémoire.
 *
 * **Durée de vie des handles** : `get` renvoie un pointeur vers une entrée du cache, valide tant
 * que l'entrée n'est pas invalidée. `invalidate`/`invalidateAll` **détruisent** l'entrée, donc les
 * ressources Direct3D si personne d'autre ne les retient : ils ne doivent être appelés
 * qu'**entre deux images**, jamais pendant la composition d'une image. Un appelant qui souhaite
 * conserver une texture au-delà d'une image en copie le `ComPtr` — c'est le contrat explicite,
 * pas une invalidation différée cachée.
 */
class TextureCache {
public:
    /**
     * @brief Construit un cache vide pour un contexte de rendu et un dossier d'assets donnés.
     * @param context Interface de rendu et lot de mises à jour de l'image courante (référencé, non
     *                copié : le lot change à chaque image).
     * @param paths   Résolveur de chemins d'assets (copié : le cache en garde sa propre copie).
     */
    TextureCache(const RhiContext& context, AssetPaths paths);

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    /**
     * @brief Obtient la texture d'un asset, en la chargeant au premier accès.
     * @param fileName Nom logique du fichier, relatif au dossier d'assets (ex. « forest.png »).
     * @param family   Famille de l'asset, qui fixe les dimensions attendues (`EX-REN-007`).
     * @return La texture chargée (propriété du cache), ou `nullptr` si l'asset est absent,
     *         illisible ou non conforme — jamais d'exception (`EX-NFR-040`).
     */
    [[nodiscard]] const LoadedTexture* get(const std::string& fileName, AssetFamily family);

    /**
     * @brief Obtient la variante d'un asset **détourée** à la silhouette d'un type de tuile.
     *
     * Un skin fourni par l'auteur est une image carrée ; l'afficher tel quel sur une pente
     * donnerait un carré plein là où le personnage passe, et la lecture du niveau serait fausse
     * (`LOT-42` TACHE-03). Le détourage est appliqué **une fois**, au chargement, puis conservé
     * comme toute autre entrée du cache.
     *
     * La variante détourée est mémorisée sous une **clé distincte** de l'originale : un même
     * fichier peut être assigné à la fois à un type carré et à une pente sans que l'un abîme
     * l'autre.
     *
     * Un type **sans** silhouette (`hmi::hasSilhouette`) n'est pas détouré : l'appel équivaut
     * alors à `get`.
     * @param fileName Nom logique du fichier, relatif au dossier d'assets.
     * @param family   Famille de l'asset, qui fixe les dimensions attendues (`EX-REN-007`).
     * @param type     Type de tuile dont la silhouette est appliquée.
     * @return La texture détourée (propriété du cache), ou `nullptr` si l'asset est absent,
     *         illisible ou non conforme — jamais d'exception (`EX-NFR-040`).
     */
    [[nodiscard]] const LoadedTexture* getMasked(const std::string& fileName, AssetFamily family,
                                                 core::TileType type);

    /**
     * @brief Obtient la description d'animation d'un asset (`nom-asset.anim.json`, `LOT-46`
     *        TACHE-03), en la chargeant et la validant au premier accès.
     *
     * **Absence de fichier** : cas par défaut (asset non animé), renvoie `nullptr` **sans
     * avertissement**. **Fichier présent mais invalide**, ou incohérent avec les dimensions
     * décodées du PNG (@p textureWidth/@p textureHeight, déjà connues via `get`/`getMasked`) :
     * `nullptr`, avec un avertissement journalisé une seule fois (même mémoïsation des échecs que
     * `get`, `EX-NFR-040`).
     * @param fileName      Nom logique de l'asset animé (ex. « water.png »), pas du descripteur.
     * @param textureWidth  Largeur décodée du PNG de cet asset, en pixels.
     * @param textureHeight Hauteur décodée du PNG de cet asset, en pixels.
     * @return La description, ou `nullptr` si l'asset n'est pas animé ou que sa description est
     *         invalide.
     */
    [[nodiscard]] const AnimationDescription* getAnimation(const std::string& fileName,
                                                           int textureWidth, int textureHeight);

    /**
     * @brief Obtient la texture d'un fichier désigné par son **chemin**, hors du dossier d'assets.
     *
     * Les **plans picturaux** (`LOT-69`) sont des données de **niveau**, pas des assets
     * réutilisables : leurs images vivent à côté des niveaux, et aucune famille
     * (`hmi::AssetFamily`) ne leur impose de dimensions — c'est le format du niveau qui les borne
     * (`EX-DEC-044`), au chargement, avant même qu'on arrive ici. D'où cette entrée distincte de
     * `get`, qui résout un nom logique sous `Assets/` et valide un contrat de dimensions.
     *
     * Mémoïsation et invalidation identiques à `get` (échec compris), sous le chemin **absolu**
     * comme clé : deux niveaux peuvent nommer leurs plans pareillement sans se marcher dessus.
     * @param path Chemin du fichier image.
     * @return La texture chargée (propriété du cache), ou `nullptr` si le fichier est absent ou
     *         illisible — jamais d'exception (`EX-NFR-040`).
     */
    [[nodiscard]] const LoadedTexture* getFromPath(const std::filesystem::path& path);

    /**
     * @brief Retire une entrée du cache, de sorte que le prochain `get` relise le fichier.
     *
     * Retire aussi un éventuel **échec** mémorisé, et la description d'animation associée le cas
     * échéant (`getAnimation`, `LOT-46` TACHE-03, invalidation **conjointe**) : c'est ce qui
     * permet à un asset créé ou modifié après coup — texture ou son fichier d'animation — d'être
     * pris en compte sans redémarrer (rechargement à chaud, `LOT-43`).
     * @param fileName Nom logique du fichier à oublier (sans effet s'il n'est pas en cache).
     */
    void invalidate(const std::string& fileName);

    /// Retire **toutes** les entrées du cache (rechargement global, `LOT-43`/`LOT-54`), textures
    /// **et** descriptions d'animation (invalidation conjointe, `LOT-46` TACHE-03).
    void invalidateAll();

    /// @return Le nombre d'entrées mémorisées (succès **et** échecs), pour le diagnostic.
    [[nodiscard]] std::size_t entryCount() const noexcept {
        return _entries.size();
    }

    /**
     * @brief Texture de repli en damier magenta, créée **une seule fois** à la demande.
     *
     * Partagée par tous les appelants : le damier ne dépend d'aucun asset, il n'y a donc aucune
     * raison d'en créer plusieurs (`LOT-40` TACHE-03).
     * @return La texture de repli, ou `nullptr` si même sa création GPU a échoué (cas extrême :
     *         device perdu — le rendu saute alors la primitive plutôt que de planter).
     */
    [[nodiscard]] const LoadedTexture* missingTexture();

    /// @return Le résolveur de chemins d'assets utilisé par ce cache.
    [[nodiscard]] const AssetPaths& assetPaths() const noexcept {
        return _paths;
    }

private:
    /// Charge et valide un asset depuis le disque, sans passer par le cache. Applique la
    /// silhouette de @p maskType si ce type en a une (détourage des pentes, `LOT-42`).
    [[nodiscard]] std::optional<LoadedTexture> load(
        const std::string& fileName, AssetFamily family,
        std::optional<core::TileType> maskType = std::nullopt) const;

    /// Obtient une entrée du cache sous une clé donnée, en la chargeant au premier accès.
    [[nodiscard]] const LoadedTexture* getUnderKey(const std::string& cacheKey,
                                                   const std::string& fileName, AssetFamily family,
                                                   std::optional<core::TileType> maskType);

    /// Charge et valide la description d'animation d'un asset depuis le disque (sans cache),
    /// via `hmi::AnimationCatalog`. `std::nullopt` couvre aussi bien l'absence de fichier
    /// (silencieuse) qu'un échec de lecture/validation (journalisé par `getAnimation`).
    [[nodiscard]] std::optional<AnimationDescription> loadAnimation(const std::string& fileName,
                                                                    int textureWidth,
                                                                    int textureHeight) const;

    const RhiContext& _context;  // possédé par le viewport
    AssetPaths _paths;
    /// Nom logique → texture chargée ; mémorise aussi un **échec** déjà journalisé. La
    /// mémoïsation/invalidation elle-même est factorisée dans `hmi::CacheRegistry` (`LOT-43`),
    /// testable sans GPU ; seul le **chargement** (`load`) reste ici, spécifique au GPU.
    CacheRegistry<LoadedTexture> _entries;
    /// Nom logique → description d'animation, sous la **même clé** que `_entries` (`LOT-46`
    /// TACHE-03) : c'est ce qui permet à `invalidate`/`invalidateAll` de rester le seul point
    /// d'invalidation, conjointe entre texture et animation.
    CacheRegistry<AnimationDescription> _animationEntries;
    std::optional<LoadedTexture> _missingTexture;
};

/**
 * @brief Obtient la texture d'un asset, ou le **repli en damier magenta** à défaut.
 *
 * **Point de résolution unique** du repli : tous les lots d'habillage (`LOT-42`, `LOT-44`,
 * `LOT-45`, `LOT-49`) passent par ici plutôt que de réimplémenter chacun son propre comportement
 * de secours — c'est la seule façon de garantir qu'un asset manquant se voie toujours de la même
 * manière et soit toujours journalisé (`EX-NFR-040`, `EX-REN-007`).
 * @param cache    Cache interrogé (et propriétaire du damier partagé).
 * @param fileName Nom logique de l'asset attendu, repris dans l'avertissement journalisé.
 * @param family   Famille de l'asset, qui fixe les dimensions attendues.
 * @return La texture de l'asset si elle est disponible, sinon le damier de repli ; `nullptr`
 *         seulement si même le damier n'a pas pu être créé.
 */
[[nodiscard]] const LoadedTexture* resolveOrPlaceholder(TextureCache& cache,
                                                        const std::string& fileName,
                                                        AssetFamily family);

}  // namespace hmi
