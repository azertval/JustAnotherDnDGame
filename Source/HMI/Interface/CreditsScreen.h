// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/CreditsScreen.h
 * @brief Écran « Crédits » du menu principal (`LOT-60`). Mise en page dans `CreditsScreen.ui`.
 */

namespace Ui {
class CreditsScreen;
}

namespace hmi {

class Localization;

/**
 * @brief Écran de crédits (développement, bruitages) atteint depuis le menu principal.
 *
 * Simple page du `QStackedWidget` (comme `MainMenu`/`OptionsPage`), pas un recouvrement : pas de
 * scène de jeu à laisser visible derrière. N'émet que l'intention de retour.
 *
 * ## Ce que la mise en page porte, et que le `.ui` ne dit plus lui-meme
 *
 * Le `.ui` de cet ecran est un fichier **produit par Qt Designer** (`EX-IHM-006`) : Designer
 * efface les commentaires XML a chaque enregistrement, et la justification de la mise en
 * page vit donc ici, ou elle survit a l'edition.
 *
 * - La racine — Les marges sont posees en code (`CreditsScreen::CreditsScreen`, `LOT-56`) depuis
 *   l'echelle d'espacement des jetons.
 * - `creditsCard` — Le contenu tient dans un cadre pixel (`LOT-68`) : la maquette pose les trois
 *   sections sur une carte, pas a nu sur le fond.
 * - `developmentCredit` — Nom propre : **jamais traduit**, cf. `CreditsScreen::retranslateUi`.
 * - `audioCredit` — Noms propres et licence : jamais traduits. Detail fichier par fichier dans
 *   `Source/Elements/Audio/CREDITS.md`.
 * - `graphicsCredit` — Noms propres et licence : jamais traduits. Detail fichier par fichier dans
 *   `Source/Elements/Assets/CREDITS.md`.
 * - `fontsCredit` — Noms propres et licence : jamais traduits. La **SIL OFL EXIGE** que sa notice
 *   accompagne les polices partout ou elles sont redistribuees ; les fichiers de licence vivent
 *   dans `Source/Elements/Assets/Fonts/`.
 * - `librariesCredit` — Mention **EXIGEE par la LGPLv3** : l'usage de Qt et sa licence doivent etre
 *   indiques, et le lien reste dynamique pour que l'utilisateur puisse substituer sa propre
 *   version. Detail dans `THIRD-PARTY-NOTICES.md`.
 * - `licenseCredit` — La licence du **jeu**, distincte des precedentes : les ressources restent
 *   sous CC0 et SIL OFL, Qt sous LGPLv3. Le dire evite de laisser croire que tout est GPL.
 */
class CreditsScreen : public QWidget {
    Q_OBJECT

public:
    explicit CreditsScreen(QWidget* parent = nullptr);
    ~CreditsScreen() override;

    /// Applique la langue active aux libellés (titre, intitulés de section, bouton retour) — les
    /// noms d'auteurs et licences restent inchangés d'une langue à l'autre.
    void retranslateUi(const Localization& loc);

    /// Donne le focus clavier au bouton « Retour » (navigation manette/clavier, même patron que
    /// `PauseScreen::focusDefaultAction`) : seul élément interactif de cet écran.
    void focusDefaultAction();

signals:
    void backRequested();

private:
    std::unique_ptr<Ui::CreditsScreen> _ui;
};

}  // namespace hmi
