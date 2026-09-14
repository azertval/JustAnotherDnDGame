// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQmlIntegration>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Core/Combat/Arena.h"
#include "Core/Rpg/Bestiary.h"
#include "Core/Rpg/CharacterSheet.h"

/**
 * @file HMI/Runtime/ArenaModel.h
 * @brief Le Colisée, tel que l'écran de mise en place le voit et le commande (`LOT-50`).
 */

namespace hmi {

/**
 * @brief La vue-modèle de l'arène : composer deux camps, lancer, jouer, rejouer.
 *
 * ## Ce qu'elle tient, et ce qu'elle ne décide pas
 *
 * Elle tient une `core::ArenaSession` — la première session de jeu à tenir un
 * `core::CombatState` — et les catalogues dont l'écran compose son affrontement : le bestiaire
 * (`LOT-33`), le personnage de démonstration (`LOT-38`), les arènes et les Marques Héroïques.
 * Elle ne décide **rien** du combat : chaque geste de l'écran devient un appel à la session, et
 * ce que l'écran affiche est relu de la machine à états après chaque geste.
 *
 * ## Pourquoi une liste de cases et non une scène
 *
 * L'écran de l'arène est un écran de développeur, en QML, sans charte (feuille de route, §5) : il
 * dessine la grille lui-même, depuis `cells`, plutôt que par la surface de rendu — qui n'affiche
 * encore aucune scène. C'est l'IHM de combat (`LOT-24`) qui dessinera le combat sur la carte ;
 * elle lira la même session, par les mêmes appels.
 *
 * Un seul signal, `changed`, pour tout ce qui découle d'un geste : la grille, l'ordre, le
 * journal et l'issue changent ensemble, et les distinguer n'épargnerait aucun rafraîchissement.
 */
class ArenaModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    /// Nom de l'arène jouable, ou ce qui manque pour en avoir une.
    Q_PROPERTY(QString arenaName READ arenaName NOTIFY changed)
    /// Ce que l'écran doit dire : erreurs de chargement, refus de montage, issue.
    Q_PROPERTY(QString status READ status NOTIFY changed)
    /// Vrai entre `launch` et `backToSetup` : la grille se joue, la composition est figée.
    Q_PROPERTY(bool inCombat READ inCombat NOTIFY changed)
    /// Vrai quand le combat en cours a une issue.
    Q_PROPERTY(bool ended READ ended NOTIFY changed)
    /// Tout ce qu'on peut enrôler : `{id, name, kind, hitPoints, armorClass}`.
    Q_PROPERTY(QVariantList roster READ roster NOTIFY changed)
    /// Les alliés composés : `{id, name, mark}`.
    Q_PROPERTY(QVariantList allies READ allies NOTIFY changed)
    /// Les ennemis composés : `{id, name, mark}`.
    Q_PROPERTY(QVariantList enemies READ enemies NOTIFY changed)
    /// Les huit Marques Héroïques, par identifiant.
    Q_PROPERTY(QStringList marks READ marks NOTIFY changed)
    /// La graine du prochain lancement. Deux lancements à graine égale donnent le même déroulé.
    Q_PROPERTY(int seed READ seed WRITE setSeed NOTIFY changed)
    /// Vrai si les ennemis sont joués par l'IA (`LOT-23`) : leurs tours se jouent seuls, par les
    /// profils de `behaviors.json`. Faux : on commande les deux camps, comme au `LOT-50`.
    Q_PROPERTY(bool enemyAi READ enemyAi WRITE setEnemyAi NOTIFY changed)
    Q_PROPERTY(int gridColumns READ gridColumns NOTIFY changed)
    Q_PROPERTY(int gridRows READ gridRows NOTIFY changed)
    /// Une entrée par case, ligne par ligne : `{column, row, wall, occupant, side, reachable,
    /// active, down, hitPoints, hitPointsRatio}` -- la part de vie restante, de 0 à 1, pour la
    /// jauge de la case.
    Q_PROPERTY(QVariantList cells READ cells NOTIFY changed)
    /// L'ordre d'initiative : `{name, total, side, active, down}`.
    Q_PROPERTY(QVariantList turnOrder READ turnOrder NOTIFY changed)
    Q_PROPERTY(QString activeName READ activeName NOTIFY changed)
    /// Les ressources restantes du combattant actif, lisibles.
    Q_PROPERTY(QString activeResources READ activeResources NOTIFY changed)
    Q_PROPERTY(QStringList journal READ journal NOTIFY changed)

public:
    explicit ArenaModel(QObject* parent = nullptr);
    ~ArenaModel() override;

    [[nodiscard]] QString arenaName() const;
    [[nodiscard]] QString status() const;
    [[nodiscard]] bool inCombat() const noexcept;
    [[nodiscard]] bool ended() const;
    [[nodiscard]] QVariantList roster() const;
    [[nodiscard]] QVariantList allies() const;
    [[nodiscard]] QVariantList enemies() const;
    [[nodiscard]] QStringList marks() const;
    [[nodiscard]] int seed() const noexcept;
    void setSeed(int seed);
    [[nodiscard]] bool enemyAi() const noexcept;
    void setEnemyAi(bool enabled);
    [[nodiscard]] int gridColumns() const;
    [[nodiscard]] int gridRows() const;
    [[nodiscard]] QVariantList cells() const;
    [[nodiscard]] QVariantList turnOrder() const;
    [[nodiscard]] QString activeName() const;
    [[nodiscard]] QString activeResources() const;
    [[nodiscard]] QStringList journal() const;

    /// Enrôle une entrée du `roster` dans un camp. Sans effet pendant un combat.
    Q_INVOKABLE void addAlly(const QString& id);
    Q_INVOKABLE void addEnemy(const QString& id);
    Q_INVOKABLE void removeAlly(int index);
    Q_INVOKABLE void removeEnemy(int index);
    /// Attribue une Marque Héroïque à un allié ou un ennemi déjà composé.
    Q_INVOKABLE void assignMark(bool ally, int index, const QString& markId);
    /// Monte l'affrontement et jette l'initiative. Refuse une composition sans les deux camps.
    Q_INVOKABLE void launch();
    /// Le geste sur une case : déplacer le combattant actif si elle est atteignable, attaquer si
    /// elle porte un ennemi à portée.
    Q_INVOKABLE void tapCell(int column, int row);
    /// L'action *esquiver* du combattant actif.
    Q_INVOKABLE void dodge();
    /// L'action *se désengager* du combattant actif.
    Q_INVOKABLE void disengage();
    /// L'action *se précipiter* du combattant actif.
    Q_INVOKABLE void dash();
    Q_INVOKABLE void endTurn();
    Q_INVOKABLE void withdraw();
    /// Remonte le même affrontement à la même graine.
    Q_INVOKABLE void replay();
    /// Revient à la composition, sans la perdre.
    Q_INVOKABLE void backToSetup();

signals:
    void changed();

private:
    struct Fighter;
    struct Catalogs;

    void loadCatalogs();
    [[nodiscard]] std::optional<Fighter> fighterFor(const QString& id, core::CombatSide side) const;
    [[nodiscard]] core::ArenaBout composeBout() const;
    void refreshMessage(const core::ArenaMount& mount);
    /// Une session neuve sur la carte de l'arène, l'IA décidant des opportunités de ses créatures.
    void resetSession();
    /// Joue les tours de l'IA tant que le combattant actif en a un profil : le joueur reprend la
    /// main à son tour, ou à l'issue.
    void playAiTurns();

    std::unique_ptr<Catalogs> _catalogs;
    std::unique_ptr<core::ArenaSession> _session;
    std::vector<Fighter> _allies;
    std::vector<Fighter> _enemies;
    int _seed = 2026;
    bool _enemyAi = true;
    bool _inCombat = false;
    QString _status;
};

}  // namespace hmi
