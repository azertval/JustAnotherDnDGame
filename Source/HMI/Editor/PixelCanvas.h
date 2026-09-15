// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QPointF>
#include <QSize>
#include <QWidget>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "HMI/Editor/EditContextTarget.h"
#include "HMI/Editor/PixelCanvasGeometry.h"
#include "HMI/Editor/PixelHistory.h"
#include "HMI/Editor/PixelOperations.h"
#include "HMI/Editor/PixelTool.h"
#include "HMI/Graphics/TextureLoader.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QPixmap;
class QWheelEvent;

namespace hmi {
class Localization;
}  // namespace hmi

/**
 * @file HMI/Editor/PixelCanvas.h
 * @brief Canevas de l'atelier pixel art : affiche et édite un `hmi::DecodedImage` en mémoire
 *        (`LOT-54` TACHE-03, `EX-EDIT-045`).
 */

namespace hmi {

/**
 * @brief Widget affichant l'image en cours d'édition et transformant les gestes de souris en
 *        appels aux opérations pures de `hmi::PixelOperations` (TACHE-02).
 *
 * **Zoom entier**, agrandi au plus proche voisin (`EX-ARCH-022`), rendu à la résolution **réelle**
 * de l'écran (`hmi::pixelCanvasRealSize`, réutilise `LOT-56` TACHE-05) : jamais flou à 125 %/150 %
 * d'échelle d'affichage. La **surface de peinture** — fond et damier de transparence sous les zones
 * à alpha nul — appartient à la portée **invariante** des jetons (`hmi::identityTokens()`) : elle
 * ne change jamais avec le thème clair/sombre du châssis d'édition (`LOT-56` TACHE-06), à la
 * différence du châssis qui l'entoure (barre d'outils, panneaux — TACHE-04). Un fond qui change de
 * clarté selon l'heure de la journée fausserait la perception des couleurs posées.
 *
 * Un geste complet (appui, glisser, relâchement) produit **une seule** entrée dans `hmi::
 * PixelHistory`, locale à ce canevas et totalement indépendante de `core::LevelDraft` — annuler un
 * coup de pinceau n'annule jamais une action d'édition de niveau, et réciproquement.
 *
 * Implémente `hmi::EditContextTarget` (TACHE-04) : `MainWindow` y dispatche Annuler/Refaire/
 * Copier/Coller quand ce canevas est le contexte d'édition actif, exactement comme `GameViewport`
 * pour le niveau — le seuil de dispatch prévu depuis `LOT-57` TACHE-04, sans le réécrire. Copier et
 * coller (TACHE-06) visent un presse-papiers **propre à l'atelier** (`hmi::PixelClipboard`),
 * distinct de celui de l'éditeur de niveaux.
 */
class PixelCanvas : public QWidget, public EditContextTarget {
    Q_OBJECT

public:
    explicit PixelCanvas(QWidget* parent = nullptr);

    /// Catalogue de traduction utilisé pour l'infobulle de case en mode planche à raccords
    /// (`hmi::autotileConfigurationLabelKey`, TACHE-08). Même patron que `GameViewport::
    /// setLocalization`.
    void setLocalization(const Localization* loc) noexcept {
        _loc = loc;
    }

    /// Remplace l'image éditée et réinitialise l'historique local (ouverture d'un asset, TACHE-05).
    void setImage(DecodedImage image);

    /**
     * @brief Pose un **repère dessiné sous** le contenu édité (mode création, `EX-EDIT-046`).
     *
     * Trois entrées — celle-ci, `setOverlay` et `setReferenceGridStep` — sont **optionnelles et
     * additives** : l'atelier pixel art qui les ignore se comporte exactement comme avant
     * (`LOT-54`). Ce qu'elles montrent n'entre **jamais** dans le tampon édité, ni dans
     * l'historique, ni dans le copier : ce sont des repères, pas du contenu.
     *
     * La référence est un repère **géométrique**, pas un aperçu : ni raccords automatiques, ni
     * skins, ni animation. L'aperçu fidèle reste l'essai.
     * @param image   Image du repère, aux dimensions du contenu édité ; vide pour l'enlever.
     * @param opacity Opacité d'affichage, dans `[0, 1]`.
     */
    void setUnderlay(DecodedImage image, float opacity = 1.0f);

    /**
     * @brief Pose un repère dessiné **par-dessus** le contenu édité — mêmes règles que
     *        `setUnderlay`.
     * @param image   Image du repère ; vide pour l'enlever.
     * @param opacity Opacité d'affichage, dans `[0, 1]`.
     */
    void setOverlay(DecodedImage image, float opacity = 1.0f);

    /**
     * @brief Pose une grille de **tuiles**, distincte de la grille de pixels existante.
     *
     * Sur un plan pictural, la grille de pixels ne dit rien de la géométrie du niveau : c'est le
     * pas de tuile qui permet de viser une case. Les deux coexistent, à des pas différents.
     * @param step Pas de la grille, en pixels image (la densité du plan) ; `0` l'enlève.
     */
    void setReferenceGridStep(int step);
    [[nodiscard]] const DecodedImage& image() const noexcept {
        return _image;
    }

    /// Nom de l'asset ouvert (barre d'état, TACHE-04), vide si aucun. `setImage` le remet à vide
    /// (nouveau document) ; l'appelant le renseigne ensuite après une ouverture ou un
    /// enregistrement réussi (TACHE-05).
    void setAssetName(std::string name) {
        _assetName = std::move(name);
    }
    [[nodiscard]] const std::string& assetName() const noexcept {
        return _assetName;
    }
    /// Modifications non enregistrées depuis l'ouverture/le dernier enregistrement (barre d'état
    /// TACHE-04, garde-fou de perte de travail TACHE-05) : un coup de pinceau la marque, un
    /// enregistrement (`markSaved`) la lève.
    [[nodiscard]] bool isDirty() const noexcept {
        return _dirty;
    }
    /// Marque l'image comme enregistrée — appelé après une écriture réussie (TACHE-05). Ne touche
    /// pas à l'historique : annuler après un enregistrement reste possible et remarque « modifié »,
    /// comme dans tout éditeur (revenir en arrière s'écarte du fichier tout juste écrit).
    void markSaved() noexcept {
        _dirty = false;
    }

    void setActiveTool(PixelTool tool) noexcept {
        _activeTool = tool;
    }
    [[nodiscard]] PixelTool activeTool() const noexcept {
        return _activeTool;
    }

    /// Couleur courante (pinceau, pot de peinture). Point d'entrée **unique** de la mutation :
    /// la pipette (au clic) passe par ici aussi, pour qu'un seul signal (`currentColorChanged`)
    /// couvre toutes les sources — pipette, pastille de palette, sélecteur dédié de la barre
    /// d'outils (`MainWindow`).
    void setCurrentColor(std::uint32_t color);
    [[nodiscard]] std::uint32_t currentColor() const noexcept {
        return _currentColor;
    }

    /// Couleurs de la palette de projet (`LOT-54` TACHE-07), consultées quand le mode contraint est
    /// actif. Simple copie des valeurs : le canevas n'est jamais propriétaire de la palette
    /// elle-même (`MainWindow`/`hmi::PixelPalette`).
    void setPaletteColors(std::vector<std::uint32_t> colors) {
        _paletteColors = std::move(colors);
    }
    /// Active/désactive le mode « contraindre à la palette » : toute couleur posée (pinceau, pot de
    /// peinture) est ramenée à l'entrée la plus proche de `setPaletteColors` (`hmi::
    /// nearestPaletteColor`) ; désactivable à tout moment, jamais imposé.
    void setPaletteConstrained(bool enabled) noexcept {
        _paletteConstrained = enabled;
    }
    [[nodiscard]] bool paletteConstrained() const noexcept {
        return _paletteConstrained;
    }

    [[nodiscard]] const PixelHistory& history() const noexcept {
        return _history;
    }

    // hmi::EditContextTarget (TACHE-04/TACHE-06) : Annuler/Refaire/Copier/Coller à cible
    // contextuelle (EX-IHM-062).
    [[nodiscard]] bool canUndo() const override {
        return _history.canUndo();
    }
    void undo() override;
    [[nodiscard]] bool canRedo() const override {
        return _history.canRedo();
    }
    void redo() override;
    /// Revient à l'état immédiatement après l'entrée @p index de `history().appliedEntries()`
    /// (panneau d'historique visuel, TACHE-04) ; sans effet si @p index est hors bornes.
    void jumpHistoryTo(std::size_t index);
    [[nodiscard]] bool canCopy() const override {
        return _image.width > 0 && _image.height > 0;
    }
    /// Copie la sélection courante, ou l'image entière à défaut de sélection.
    void copy() override;
    [[nodiscard]] bool canPaste() const override {
        return !_clipboard.empty();
    }
    /// Colle le presse-papiers de l'atelier au pixel survolé (coin haut-gauche), tronqué au cadre ;
    /// sans effet si le presse-papiers est vide. Sélectionne le contenu collé, pour l'ajuster
    /// (déplacer, transformer) immédiatement après.
    void paste() override;

    /// @return Le pas de la grille de tuiles, `0` si aucune n'est posée.
    [[nodiscard]] int referenceGridStep() const noexcept {
        return _referenceGridStep;
    }

    /// Région actuellement sélectionnée (outil Sélection), vide si aucune.
    [[nodiscard]] const PixelRegion& selection() const noexcept {
        return _selection;
    }
    /// Retourne la sélection horizontalement, ou l'image entière à défaut de sélection.
    void applyFlipHorizontal();
    /// Retourne la sélection verticalement, ou l'image entière à défaut de sélection.
    void applyFlipVertical();
    /// Pivote la sélection d'un quart de tour horaire, ou l'image entière à défaut de sélection.
    void applyRotateClockwise();
    /// Pivote la sélection d'un quart de tour antihoraire, ou l'image entière à défaut de
    /// sélection.
    void applyRotateCounterClockwise();

    [[nodiscard]] const PixelCanvasView& view() const noexcept {
        return _view;
    }
    void zoomIn();
    void zoomOut();

    /// Pixel image sous une position widget donnée (coordonnées **logiques**), ou `std::nullopt`
    /// hors de l'image — alimente le pixel survolé de la barre d'état (TACHE-04).
    [[nodiscard]] std::optional<std::pair<int, int>> imagePixelAt(
        const QPointF& widgetPosition) const;
    /// Dernier pixel survolé par la souris à l'intérieur de l'image, ou `std::nullopt` si la souris
    /// n'y est pas (curseur sorti du widget, ou hors de l'image à zoom/décalage donnés).
    [[nodiscard]] std::optional<std::pair<int, int>> hoveredPixel() const noexcept {
        return _hoveredPixel;
    }

    [[nodiscard]] QSize sizeHint() const override;

signals:
    /// Émis après chaque mutation de `image()` (geste, annuler, refaire) — la barre d'état et
    /// l'aperçu live (TACHE-08) s'y abonnent plutôt que d'interroger le canevas à chaque frame.
    void imageChanged();
    /// Émis après chaque changement de l'historique (nouvelle entrée, annuler, refaire) — alimente
    /// le panneau d'historique visuel (`hmi::PixelHistoryPanel`).
    void historyChanged();
    /// Émis à chaque changement du pixel survolé (barre d'état, TACHE-04).
    void hoveredPixelChanged(std::optional<std::pair<int, int>> pixel);
    /// Émis à chaque changement de la couleur courante, quelle qu'en soit la source (pipette,
    /// pastille de palette, sélecteur dédié) — la barre d'outils y aligne son témoin de couleur.
    void currentColorChanged(std::uint32_t color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    [[nodiscard]] QPixmap renderPixmap() const;

    /// Applique l'outil actif au pixel `(x, y)` (un seul point : clic, pipette, pot de peinture) et
    /// étend `_gestureRegion` avec la région effectivement modifiée.
    void applyToolAtPoint(int x, int y);
    /// Prolonge le geste courant du dernier point traité jusqu'à `(x, y)` (pinceau/gomme : ligne
    /// sans trou entre deux positions successives, TACHE-02 ; déplacement de sélection, TACHE-06).
    void continueToolTo(int x, int y);
    void beginGesture(int x, int y);
    void endGesture();

    /// Région effective d'une transformation : la sélection courante, ou l'image entière si aucune
    /// sélection n'est active (TACHE-06 : « à défaut de sélection, à l'image entière »).
    [[nodiscard]] PixelRegion effectiveRegion() const;
    /// Capture @p beforeSnapshot/`_image` sur @p touched, pousse l'entrée d'historique nommée si
    /// non vide, et émet les signaux de mutation — factorisé entre `applyFlipHorizontal`/
    /// `applyFlipVertical`/`applyRotateClockwise`/`applyRotateCounterClockwise`/`paste`.
    void commitRegionMutation(PixelOperationKind kind, const DecodedImage& beforeSnapshot,
                              const PixelRegion& touched);
    /// Applique une transformation pure de `hmi::PixelOperations` (même signature que
    /// `flipHorizontal`/`rotateClockwise`…) à `effectiveRegion()` et la pousse dans l'historique.
    void applyRegionOperation(PixelOperationKind kind,
                              PixelRegion (*operation)(DecodedImage&, const PixelRegion&));
    /// `_currentColor`, ramenée à l'entrée de palette la plus proche si le mode contraint est actif
    /// (`hmi::nearestPaletteColor`) ; `_currentColor` telle quelle sinon.
    [[nodiscard]] std::uint32_t effectivePaintColor() const noexcept;

    DecodedImage _image;
    /// Repère dessiné **sous** le contenu édité (`setUnderlay`) — jamais dans `_image`, donc jamais
    /// dans l'historique ni dans le presse-papiers.
    DecodedImage _underlay;
    float _underlayOpacity = 1.0f;
    /// Repère dessiné **par-dessus** le contenu édité (`setOverlay`), mêmes règles.
    DecodedImage _overlay;
    float _overlayOpacity = 1.0f;
    /// Pas de la grille de tuiles, en pixels image ; `0` si aucune.
    int _referenceGridStep = 0;
    std::string _assetName;
    bool _dirty = false;
    PixelHistory _history;
    PixelCanvasView _view;
    PixelTool _activeTool = PixelTool::Brush;
    std::uint32_t _currentColor = 0xFF000000u;  // noir opaque (R8G8B8A8_UNORM, LOT-54 TACHE-01).
    std::vector<std::uint32_t> _paletteColors;  ///< Copie des couleurs de la palette (TACHE-07).
    bool _paletteConstrained = false;

    bool _gestureActive = false;
    DecodedImage _gestureBeforeSnapshot;  ///< Copie de `_image` au début du geste courant.
    PixelRegion _gestureRegion;           ///< Union des régions touchées depuis le début du geste.
    std::optional<std::pair<int, int>> _lastGesturePixel;
    std::optional<std::pair<int, int>> _hoveredPixel;

    // Outils de région (LOT-54 TACHE-06).
    PixelRegion _selection;             ///< Sélection courante, vide si aucune.
    PixelClipboard _clipboard;          ///< Presse-papiers de l'atelier, distinct du niveau.
    bool _selectionMoveActive = false;  ///< `true` : le geste en cours déplace `_selection`.
    std::pair<int, int> _selectionDragAnchor{0, 0};  ///< Point de départ d'une nouvelle sélection.

    bool _panning = false;
    QPointF _panStartWidgetPos;
    PixelCanvasView _panStartView;

    const Localization* _loc = nullptr;  ///< Pour l'infobulle de case en mode planche (TACHE-08).
};

}  // namespace hmi
