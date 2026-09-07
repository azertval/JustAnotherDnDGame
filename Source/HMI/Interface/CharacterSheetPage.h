// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "HMI/Interface/CharacterSheetPlate.h"

class QLabel;

/**
 * @file HMI/Interface/CharacterSheetPage.h
 * @brief La planche 1 de la feuille de personnage de Tanares (`LOT-38`) : **la gravure du livre**,
 *        relettrée dans la langue active.
 */

namespace hmi {

class Localization;

/**
 * @brief L'écran de fiche : la **planche gravée du livre**, et les champs posés dessus.
 *
 * ## Ce que cet écran n'est plus
 *
 * La première version composait la feuille en widgets Qt — cartouches peints, roue des
 * caractéristiques tracée au pinceau, encadrements de parchemin. Elle *ressemblait* à la planche
 * sans en venir : les six cadrans de la roue suivaient un arc réglé à la main, les bandeaux
 * étaient des rectangles, et les rinceaux, la rose des vents, les cabochons n'existaient pas.
 *
 * La planche a depuis été **vectorisée** (corpus, `Character_Sheets_Tanares.svg`). L'écran ne la
 * réinvente donc plus : il la **dessine**, et se contente d'écrire dessus. Ce qui était approché
 * est désormais exact, et `hmi::AbilityWheel` a disparu — la roue est gravée, elle n'est plus
 * peinte.
 *
 * ## Le lettrage est à nous, l'ornement est au livre
 *
 * La planche livrée (`Assets/UI/character-sheet-plate.svg`) a été **débarrassée de son lettrage**
 * anglais par `scripts/build_character_sheet_plate.py`. Les intitulés sont reposés ici, traduits,
 * aux rectangles que la même table (`character-sheet-plate.json`) donne au script pour les
 * effacer. Une seule source décrit donc les deux moitiés de l'opération.
 *
 * ## Aucun champ n'est câblé en C++
 *
 * Ce fichier ne nomme ni compétence, ni caractéristique, ni section : il parcourt la table et pose
 * une étiquette par ligne. Un champ ajouté à la planche est une ligne de JSON, pas une ligne de
 * C++ — et un champ dont le `valueId` est vide garde son tiret cadratin, ce qui est la façon dont
 * la planche déclare ce que rien n'alimente encore.
 */
class CharacterSheetPage : public QWidget {
    Q_OBJECT

public:
    /// Nom de fichier du portrait posé au centre de la roue, sous `Assets/UI/`.
    static constexpr const char* PORTRAIT_FILE = "portrait-demonstration.png";
    /// Le **masque d'encre** de la gravure, sans son lettrage : un PNG monochrome a alpha,
    /// produit par `scripts/build_character_sheet_plate.py`.
    ///
    /// Un PNG, et non le SVG dont il sort : `QSvgHandler` rejette tout `<path>` de plus de 32768
    /// elements, et ce trace en demande seize fois plus. Le decouper en plusieurs chemins ne
    /// marche pas non plus -- un remplissage se calcule sur l'ensemble des contours d'un meme
    /// chemin, et separer deux contours qui se recouvrent change ce qui est plein et ce qui est
    /// vide. Le script porte la demonstration.
    static constexpr const char* PLATE_FILE = "character-sheet-plate.png";
    /// La table des champs : ce que le script efface, ce que cet écran repose.
    static constexpr const char* PLATE_TABLE_FILE = "character-sheet-plate.json";

    explicit CharacterSheetPage(QWidget* parent = nullptr);
    ~CharacterSheetPage() override;

    /// Applique la langue active à tous les intitulés que la table nomme.
    void retranslateUi(const Localization& loc);

    /// Pose les valeurs. Ce qu'aucune entrée ne nomme garde le tiret cadratin.
    void applyValues(const std::map<std::string, std::string>& values,
                     const std::string& emptyMark);

    /// La planche garde le rapport de la maquette : elle ne s'étire jamais.
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;
    [[nodiscard]] bool hasHeightForWidth() const override;
    [[nodiscard]] int heightForWidth(int width) const override;

protected:
    /// Peint la gravure — mise en cache, et teintée à l'encre du thème — puis les rectangles de
    /// parchemin des quelques cartouches que la découpe vectorielle ne pouvait pas nettoyer.
    void paintEvent(QPaintEvent* event) override;
    /// Replace les champs : ils suivent la planche, qui suit la fenêtre.
    void resizeEvent(QResizeEvent* event) override;

private:
    /// Une étiquette de la planche, et le champ dont elle vient.
    struct Placed {
        QLabel* widget = nullptr;
        const PlateField* field = nullptr;
        bool isValue = false;  ///< Valeur (rectangle `value`) plutôt qu'intitulé (`label`).
    };

    /// Construit une étiquette par intitulé et par valeur de la table.
    void buildFields();
    /// Recalcule le rectangle de la planche dans le widget, et y replace tous les champs.
    void layoutPlate();
    /// @return Le rectangle de la planche : la maquette, à l'échelle, centrée. Jamais étirée —
    ///         une gravure étirée cesse d'être un rond, et cela se voit avant tout le reste.
    [[nodiscard]] QRectF plateRect() const;
    /// @return @p rect, exprimé en unités de planche, placé dans le widget.
    [[nodiscard]] QRectF toWidget(const PlateRect& rect) const;

    CharacterSheetPlateLayout _layout;
    QPixmap _plate;  ///< Le masque d'encre, à sa taille d'origine.
    /// Le masque réduit à la taille courante et teinté. Une réduction lissée d'une image de 2448
    /// pixels de large ne se refait pas à chaque `paintEvent` : elle est calculée une fois par
    /// taille, puis recopiée.
    QPixmap _plateCache;
    QPixmap _portrait;
    QLabel* _portraitLabel = nullptr;
    std::vector<Placed> _placed;
    /// Les onglets de jets de sauvegarde : leur abréviation se peint **tournée**, faute de tenir
    /// autrement dans un onglet plus haut que large.
    std::vector<const PlateField*> _rotated;
    QHash<QString, QString> _rotatedText;  ///< Identifiant de champ -> abréviation traduite.
    /// Dernières valeurs reçues, rejouées après un changement de langue.
    std::map<std::string, std::string> _values;
    QString _emptyMark = QStringLiteral("—");
};

}  // namespace hmi
