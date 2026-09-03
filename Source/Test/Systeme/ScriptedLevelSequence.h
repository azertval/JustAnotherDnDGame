// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <functional>
#include <vector>

#include "Core/Ecs/Components/Player.h"
#include "Core/Physics/PlayerInput.h"

/**
 * @file ScriptedLevelSequence.h
 * @brief Séquence des niveaux `demo-*.json` livrés et leur scénario d'entrées déterministe,
 *        extraite de `test_parcours_complet.cpp` (LOT-ANNEXE-08) pour être rejouée à l'identique
 *        par un autre test (`Source/Test/Integration/test_recompense_demo_niveaux.cpp`) sans
 *        dupliquer ces scripts.
 *
 * Header inclus directement (anonymous namespace, comme tout autre fichier de test) : chaque
 * inclusion obtient sa propre copie à liaison interne, aucune violation de l'ODR.
 */

namespace {

// Script d'entrées réactif : fonction du pas ET de l'état courant (au sol, position). Nécessaire
// dès qu'un scénario dépend de la trajectoire (double saut, wall jump, bloc) plutôt que d'un
// numéro de pas fixé à l'avance.
using ReactiveInput =
    std::function<core::PlayerInput(int step, const core::Player& player, float x, float y)>;

// Un niveau de la séquence et son scénario d'entrées.
struct ScriptedLevel {
    const char* file;
    ReactiveInput input;
};

// Script constant : avancer à droite, rien d'autre (déplacement, pente, arrondi, interrupteur —
// la trajectoire seule suffit, aucun timing particulier).
ReactiveInput rightOnly() {
    return [](int, const core::Player&, float, float) { return core::PlayerInput{1.0f}; };
}

// Vrai quand le personnage approche le bord droit @p edge (en cases) d'une plateforme, dans une
// fenetre de @p window cases. Declencher un saut ou une ruee AVANT cette fenetre gaspille la portee
// horizontale sur du sol encore solide -- constat deja fait au LOT-65 TACHE-02 sur demo-budget, et
// qui vaut pour toute fosse.
bool atLedge(float x, float edge, float window) {
    return x >= edge - window && x <= edge - 0.05f;
}

// Un danger temporise est-il mortel au pas @p step ? Meme formule que
// `core::DangerController::isBlinkActive`. Un tableau qui pose un danger temporise SUR le chemin
// exige d'attendre sa fenetre inoffensive, ce qu'aucune entree reactive ne peut deviner depuis la
// seule position du personnage : le script doit pouvoir le calculer.
bool blinkActive(int step, int phase, int period, int activeDuration) {
    const int cursor = (((step - phase) % period) + period) % period;
    return cursor < activeDuration;
}

// Distance parcourue par une plateforme mobile le long de son segment au pas @p step, meme formule
// que `core::PlatformController::boxAtStep` (aller-retour triangulaire 0 -> distance -> 0). Meme
// raison que ci-dessus : attendre qu'une plateforme revienne ne se deduit pas de la position du
// personnage.
float platformOffset(int step, float speed, float distance) {
    const float travelled = static_cast<float>(step) * speed / 60.0f;
    const float cycle = distance * 2.0f;
    float phase = std::fmod(travelled, cycle);
    if (phase < 0.0f) {
        phase += cycle;
    }
    return phase <= distance ? phase : cycle - phase;
}

// La séquence jouée et son scénario d'entrées, un par tableau. Fonction plutôt que littéral : les
// appelants (garde de fidélité pas-à-pas, non-régression de récompense) la rejouent tous, et chaque
// appel rend des scripts à l'état NEUF (chaque lambda possède son état par capture-valeur
// `mutable`, jamais partagé).
std::vector<ScriptedLevel> scriptedSequence() {
    return {
        // 1. Déplacement, chute, sol : escalier descendant, aucun saut nécessaire.
        {"demo-deplacement.json", rightOnly()},
        // 2. Saut simple : TROIS fosses de difficulte croissante -- la premiere a un fond dont on
        //    ressort, les deux suivantes sont garnies de pics. Un saut par fosse, declenche au
        //    bord : sauter en continu ferait consommer le saut aerien juste apres le decollage et
        //    retomber court.
        {"demo-saut.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 6.0f, 0.6f) || atLedge(x, 12.0f, 0.6f) || atLedge(x, 18.0f, 0.6f));
             return in;
         }},
        // 3. Double saut (EX-GP-015) : DEUX paliers a trois cases, hors de portee d'un saut simple
        //    (2,4 cases). Saut au bord de chaque palier, puis saut aerien pres de l'apex -- un
        //    saut aerien declenche trop tot ne monterait pas assez haut.
        {"demo-double-saut.json",
         [airborne = 0, airJumpDone = false](int, const core::Player& player, float x,
                                             float) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (player.grounded) {
                 airborne = 0;
                 airJumpDone = false;
                 // Sauter au BORD seulement : decoller trop tot consomme la portee horizontale
                 // sur du sol deja solide.
                 in.jumpPressed = (x >= 6.2f && x <= 6.9f) || (x >= 12.2f && x <= 12.9f);
             } else if (++airborne >= 20 && !airJumpDone) {
                 in.jumpPressed = true;  // ~0,33 s : proche de l'apex du premier saut
                 airJumpDone = true;
             }
             return in;
         }},
        // 4. Wall jump/wall slide (EX-GP-016) : puits étroit, franchi en enchaînant les wall jumps
        //    entre les deux parois (script réactif : pousse toujours à l'opposé du dernier mur
        //    touché).
        {"demo-wall-jump.json",
         [wallJumpLastPush = 1.0f](int, const core::Player& player, float, float) mutable {
             core::PlayerInput in;
             in.jumpPressed = true;
             in.jumpHeld = true;
             if (player.wallDirection != 0.0f) {
                 wallJumpLastPush = -player.wallDirection;
             }
             in.moveX = wallJumpLastPush;
             return in;
         }},
        // 5. Dash (EX-GP-017) : couloir d'une case de haut (le saut y est impossible, et le budget
        //    de sauts est nul), troue de TROIS fosses de deux cases garnies de pics. La ruee part
        //    au bord de chaque fosse : declenchee plus tot, elle est deja finie au moment de
        //    decoller, et le budget ne la recharge qu'au contact du sol.
        {"demo-dash.json",
         [](int, const core::Player&, float x, float) {
             core::PlayerInput in{1.0f};
             in.dashPressed =
                 atLedge(x, 5.0f, 0.35f) || atLedge(x, 13.0f, 0.35f) || atLedge(x, 21.0f, 0.35f);
             return in;
         }},
        // 6. Synthese de l'acte I (LOT-65 TACHE-07) : ruee sous un plafond bas au-dessus d'une
        //    fosse, deux paliers au double saut, puis un puits au wall jump jusqu'a la sortie.
        {"demo-mouvement.json",
         [airborne = 0, airJumpDone = false, lastPush = 1.0f](int, const core::Player& player,
                                                              float x, float y) mutable {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             if (y > 6.5f) {
                 // Segments A et B : couloir bas (ruee) puis paliers (double saut).
                 in.dashPressed = atLedge(x, 6.0f, 0.35f);
                 if (player.grounded) {
                     airborne = 0;
                     airJumpDone = false;
                     in.jumpPressed = atLedge(x, 13.0f, 0.8f) || atLedge(x, 20.0f, 0.8f);
                 } else if (++airborne >= 20 && !airJumpDone) {
                     in.jumpPressed = true;
                     airJumpDone = true;
                 }
             } else {
                 // Segment C : puits, wall jump alterne (pousse toujours a l'oppose du mur).
                 if (x < 25.6f) {
                     return in;  // encore sur le palier : marcher jusqu'au puits
                 }
                 in.jumpPressed = true;
                 if (player.wallDirection != 0.0f) {
                     lastPush = -player.wallDirection;
                 }
                 in.moveX = lastPush;
             }
             return in;
         }},
        // 7. Interrupteur ↔ porte (EX-GP-020) : chaque interrupteur est loge dans une alcove du
        //    plafond -- il faut sauter pour l'atteindre, et la porte reste fermee sinon. L'ancien
        //    tableau le posait sur le trajet direct vers sa porte : impossible de ne pas le
        //    resoudre, donc rien a resoudre.
        {"demo-interrupteur.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded && ((x >= 4.0f && x <= 4.6f) || (x >= 10.0f && x <= 10.6f));
             return in;
         }},
        // 8. Plaque de pression (EX-GP-025, LOT-65) : le poids doit RESTER. Le personnage pousse
        //    un bloc dans la fosse, ou il enfonce la plaque et l'y maintient, puis saute par-dessus
        //    la fosse et franchit la porte restee ouverte derriere lui. L'ancien tableau reposait
        //    sur un saut qui prenait la porte de vitesse pendant qu'elle se refermait -- il
        //    enseignait l'inverse de la mecanique.
        {"demo-plaque-pression.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 9.0f, 0.5f);
             return in;
         }},
        // 9. Cle ↔ porte verrouillee (EX-GP-023) : la cle est logee dans une alcove du plafond,
        //    et son ramassage exige le contact ET « Interagir » (EX-CTRL-022). Deux paires, pour
        //    que la lecon se confirme plutot que de passer inapercue.
        {"demo-cle.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             const bool sousUneCle = (x >= 4.0f && x <= 4.6f) || (x >= 10.0f && x <= 10.6f);
             in.jumpPressed = player.grounded && sousUneCle;
             in.interactPressed = sousUneCle;  // maintenu : le contact ne dure que le saut
             return in;
         }},
        // 10. Bloc poussable (EX-GP-022) : un SEUL saut disponible. Le bloc comble la premiere
        //     fosse a ras (on marche dessus), le saut sert pour la seconde -- sans le bloc, il en
        //     faudrait deux, et le budget les refuse.
        {"demo-bloc.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 14.0f, 0.5f);
             return in;
         }},
        // 11. Bloc a taille reduite (EX-GP-005) : la fosse fait DEUX cases, hors de portee du saut
        //     unique. Le demi-bloc n'en comble qu'une, et son sommet reste 0,25 case sous le sol :
        //     on descend dessus, puis le saut franchit la seconde.
        {"demo-bloc-reduit.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded && ((x >= 9.8f && x <= 10.8f) || (x >= 14.8f && x <= 15.8f));
             return in;
         }},
        // 12. Bloc a taille quart (EX-GP-005) : trop petit pour combler quoi que ce soit, il
        //     obstrue en revanche un couloir d'une case de haut ou il faut le pousser devant soi.
        //     La fosse qui suit, hors du couloir, exige le saut unique.
        {"demo-bloc-quart.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Le premier quart de bloc, pousse, tombe dans la fosse (case 11) sans la combler --
             // il est bien trop petit. Le seul saut du tableau sert a la franchir ; le second
             // quart se degage ensuite en marchant.
             in.jumpPressed = player.grounded && atLedge(x, 11.0f, 0.6f);
             return in;
         }},
        // 12bis. Bloc DESCENDANT (EX-GP-027, LOT-74) : deux fosses d'une case enjambees par des
        //     blocs qui s'enfoncent des qu'on les touche -- par le dessus comme par le cote. Le
        //     trace saute des qu'il est au sol : la traversee ne depend donc PAS du chrono, que le
        //     bloc ait eu le temps de descendre ou non. Un cinquieme bloc, pose sur du sol plein,
        //     montre qu'un bloc descendant ne traverse jamais la matiere.
        {"demo-bloc-descendant.json",
         [](int, const core::Player& player, float, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded;
             return in;
         }},
        // 12ter. Bloc FRAGILE (EX-GP-028, LOT-74) : cinq dalles fragiles dans le sol. Le tableau
        //     n'accorde aucune charge de dash, si bien que « dash + bas » en l'air n'agit plus
        //     vertical mais un GROUND POUND (EX-GP-058) -- le seul geste qui brise ces dalles. Le
        //     trace en declenche un au-dessus de la premiere ; le reste du parcours marche sur les
        //     autres sans les casser, ce qui montre que le contact seul ne suffit pas.
        {"demo-bloc-fragile.json",
         [](int, const core::Player& player, float x, float y) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Ground pound arme UNIQUEMENT au sommet d'un saut parti de la passerelle (y <= 1.5) :
             // le plafond y borne l'apogee vers y = 1, alors qu'un saut parti du trou d'une case
             // ouvert par la dalle brisee culmine une case plus bas. Sans cette garde d'altitude,
             // le trace repound indefiniment depuis le trou et n'avance plus -- il ne s'agit pas
             // d'un reglage cosmetique mais de la condition de terminaison du parcours.
             if (!player.grounded && y <= 1.5f && x >= 5.6f && x <= 7.6f) {
                 in.moveX = 0.0f;  // arme le pound (EX-GP-058 exige une visee purement verticale)
                 in.moveY = 1.0f;
                 in.dashPressed = true;
                 return in;
             }
             in.jumpPressed = player.grounded;
             return in;
         }},
        // 12quater. Bloc EPHEMERE (EX-GP-029, LOT-74) : cinq dalles qui s'effacent un court delai
        //     apres qu'on les a QUITTEES -- le chemin se referme derriere soi. Les fosses ainsi
        //     ouvertes ne font qu'une case : le tableau reste franchissable meme en tombant dedans,
        //     ce qui est deliberé -- une disparition definitive (EX-GP-029) ne doit jamais pouvoir
        //     enfermer le personnage dans un tableau de demonstration.
        {"demo-bloc-ephemere.json",
         [](int, const core::Player& player, float, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded;
             return in;
         }},
        // 14. Pentes et arrondis (EX-GP-003/004) : fusionne l'ancien demo-arrondi, qui reprenait le
        //     meme trace a une tuile pres. Trois marches inclinees a monter, une fosse a franchir
        //     -- c'est elle qui interdit de traverser le tableau en marchant --, puis une descente
        //     par une pente et un arrondi orientes a gauche. Aucun dash : une pente franchissable
        //     a la marche ne l'est pas au dash (defaut moteur consigne, TACHE-06).
        {"demo-pente.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 15.0f, 0.6f);
             return in;
         }},
        // 15. Pentes vers la gauche (EX-GP-003/004) : miroir exact du precedent. On entre a DROITE
        //     et la sortie est a gauche -- maintenir « droite » n'y mene nulle part, ce qui est
        //     precisement le propos de quatre silhouettes orientees a gauche.
        {"demo-pente-gauche.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{-1.0f};
             in.jumpHeld = true;
             // Fosse franchie vers la GAUCHE : le bord utile est celui de gauche de la plateforme.
             in.jumpPressed = player.grounded && x >= 13.0f && x <= 13.7f;
             return in;
         }},
        // 16. Arrondis concaves (EX-GP-007) : les concaves de SOL se montent et se descendent sur
        //     le chemin (trois montees, trois descentes), les concaves de PLAFOND bordent le
        //     couloir juste au-dessus de la tete -- et la fosse a franchir est placee sous eux,
        //     pour que le saut vienne buter dans leur silhouette. Ils flottaient jusqu'ici a deux
        //     hauteurs de saut au-dessus du chemin.
        {"demo-concave.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed = player.grounded && atLedge(x, 8.0f, 0.6f);
             return in;
         }},
        // 17. Plafond incline (EX-GP-006) : les quatre variantes bordent le couloir juste au-dessus
        //     de la tete, et chaque fosse a franchir est surmontee d'une silhouette qui raccourcit
        //     le saut -- il faut passer SOUS elle. Elles etaient jusqu'ici en ligne 2 au-dessus
        //     d'un couloir en ligne 7, soit deux fois la hauteur d'un saut : le tableau se
        //     traversait en ligne droite sans jamais approcher son sujet.
        {"demo-plafond.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 6.0f, 0.6f) || atLedge(x, 11.0f, 0.6f) || atLedge(x, 16.0f, 0.6f));
             return in;
         }},
        // 18. Dangers directionnels (EX-GP-050) : les quatre orientations bordent le couloir. Les
        //     pointes vers le HAUT sont des fosses mortelles a franchir ; celles qui pendent du
        //     plafond et celles qui garnissent la rangee haute rendent le saut dangereux ailleurs.
        //     Marcher est sur, sauter au mauvais endroit ne l'est pas -- c'est la lecon, et elle
        //     etait jusqu'ici impossible a recevoir : les quatre variantes flottaient dans des
        //     alcoves que le personnage ne pouvait pas atteindre.
        {"demo-dangers-directionnels.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 7.0f, 0.6f) || atLedge(x, 15.0f, 0.6f) || atLedge(x, 23.0f, 0.6f));
             return in;
         }},
        // 19. Dangers avances (EX-GP-051/052/053) : l'interrupteur ARME les dangers commutes places
        //     plus loin sur le chemin -- il faut donc sauter par-dessus lui, pas marcher dessus.
        //     Trois dangers temporises jalonnent ensuite le couloir : chacun impose d'attendre sa
        //     fenetre inoffensive, calculee ici avec la meme formule que le controleur. Les dangers
        //     mobiles patrouillent la rangee haute, juste au-dessus de la tete.
        {"demo-dangers-avances.json",
         [](int step, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Sauter par-dessus l'interrupteur (case 5) sans le toucher : l'armer condamne la
             // suite du couloir.
             in.jumpPressed = player.grounded && atLedge(x, 5.0f, 0.5f);

             // Attendre devant chaque danger temporise que sa fenetre mortelle soit passee, avec
             // assez de marge pour traverser la case avant qu'elle ne revienne.
             constexpr int PERIOD = 180;
             constexpr int ACTIVE = 45;
             constexpr int CROSSING = 40;  // pas necessaires pour degager la case
             const int phases[3] = {0, 60, 120};
             const float holds[3] = {13.4f, 19.4f, 25.4f};
             for (int index = 0; index < 3; ++index) {
                 if (x < holds[index] || x > holds[index] + 0.2f) {
                     continue;
                 }
                 if (blinkActive(step, phases[index], PERIOD, ACTIVE) ||
                     blinkActive(step + CROSSING, phases[index], PERIOD, ACTIVE)) {
                     in.moveX = 0.0f;  // patienter : la case est (ou redevient) mortelle
                 }
             }
             return in;
         }},
        // 20. Plateforme mobile (EX-GP-026) : trois plateformes en ascenseur au-dessus du vide,
        //     dont une verticale et une portant un bloc poussable -- le portage d'un bloc est exige
        //     par EX-GP-026 et n'etait mis en scene par aucun tableau. Manquer une plateforme est
        //     mortel : il n'y a aucun sol entre les paliers. L'ancien tableau se franchissait sans
        //     AUCUNE entree.
        {"demo-plateforme.json",
         [](int step, const core::Player& player, float x, float y) {
             core::PlayerInput in;
             in.jumpHeld = true;
             if (x < 4.6f) {
                 // Porte par l'ascenseur des l'apparition : ne rien faire jusqu'en haut, puis
                 // sauter vers le palier intermediaire.
                 if (y <= 5.4f) {
                     in.moveX = 1.0f;
                     in.jumpPressed = player.grounded;
                 }
             } else if (x < 7.5f) {
                 in.moveX = 1.0f;  // palier intermediaire
             } else if (x < 8.0f) {
                 // Attendre que la plateforme horizontale revienne a son point de depart.
                 in.moveX = platformOffset(step, 1.0f, 4.0f) < 0.25f ? 1.0f : 0.0f;
             } else if (platformOffset(step, 1.0f, 4.0f) > 3.6f) {
                 in.moveX = 1.0f;  // arrivee au bout : rejoindre le palier de sortie
             }
             return in;
         }},
        // 21. Budget de mouvements (EX-GP-024) : le trajet demande EXACTEMENT quatre sauts puis
        //     deux ruees -- un budget borné à quatre (marge d'un saut).
        {"demo-budget.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Quatre marches ascendantes, un saut chacune -- exactement le budget.
             in.jumpPressed =
                 player.grounded && (atLedge(x, 6.0f, 0.6f) || atLedge(x, 12.0f, 0.6f) ||
                                     atLedge(x, 18.0f, 0.6f) || atLedge(x, 24.0f, 0.6f));
             // Puis un couloir d'une case de haut : deux fosses, deux ruees -- le reste du budget.
             in.dashPressed = atLedge(x, 29.0f, 0.35f) || atLedge(x, 35.0f, 0.35f);
             return in;
         }},
        // 22. Synthese (LOT-65 TACHE-09) : mecanismes, terrain et dangers ENTRELACES plutot que
        //     juxtaposes. Une meme porte est commandee par DEUX declencheurs -- un interrupteur et
        //     une plaque --, croisement etabli en TACHE-06 et jamais mis en scene ; un demi-bloc
        //     comble ensuite une fosse gardee par des pics, et un arrondi mene a la sortie.
        {"demo-synthese.json",
         [](int, const core::Player& player, float x, float) {
             core::PlayerInput in{1.0f};
             in.jumpHeld = true;
             // Trois sauts : toucher l'interrupteur du plafond (case 4), franchir la fosse ou le
             // bloc est tombe sur la plaque (case 17), puis les pics du couloir (case 25).
             in.jumpPressed =
                 player.grounded &&
                 (atLedge(x, 4.0f, 0.7f) || atLedge(x, 17.0f, 0.6f) || atLedge(x, 25.0f, 0.6f));
             return in;
         }},
        // 23. Final multi-salles (LOT-65 TACHE-09) : absorbe l'ancien demo-salles, qui portait 272
        //     tuiles et ZERO mecanique, dont 40 % scellees sous le sol -- et qui etait joue APRES
        //     le final. Quatre salles, une enigme par salle, cadrage par salle avec des ZONES
        //     dessinees a la main et une taille de salle propre au niveau (EX-LVL-007, EX-REN-017,
        //     les deux variantes du LOT-64 qu'aucun tableau n'employait).
        //
        //     A (haut gauche) : poser le bloc sur la plaque tient la porte ouverte, et il faut
        //     repartir SANS lui -- le poids doit rester, c'est tout le propos de EX-GP-025.
        //     B (haut droite) : arrondis a gravir, quart de bloc a degager, puis un puits.
        //     C (bas droite)  : la cle, gardee par trois dangers temporises dephases.
        // 23. Final (LOT-71) : reprend le tracé de `Test-IA`, un gauffre unique bien plus dense que
        //     l'ancien final multi-salles -- clé/porte verrouillée dans le puits de wall jump,
        //     bloc/plaque tenant une porte, ascenseur à plateformes synchronisées au-dessus d'un
        //     danger mobile, seconde clé gardée par un interrupteur/porte, sortie derrière trois
        //     portes verrouillées. Machine à états explicite (`phase`) : la position seule ne dit
        //     pas où on en est sur un tracé qui repasse par les mêmes abscisses à des hauteurs
        //     différentes.
        {"demo-final.json",
         [phase = 0, wallJumpLastPush = 1.0f, committedToLift = false,
          dashedBackFromSecondBlink = false, startedBottomRun = false,
          lastY = 0.0f](int step, const core::Player& player, float x, float y) mutable {
             core::PlayerInput in;
             in.jumpHeld = true;
             // Le script ne voit pas la vitesse : l'apex se déduit du moment où la
             // hauteur cesse de diminuer, ce qui suffit à placer le second saut.
             const bool falling = y > lastY;
             lastY = y;

             // 0. Case clé #1 (5, 12) : marcher jusqu'à elle depuis l'entrée et interagir.
             if (phase == 0) {
                 in.moveX = 1.0f;
                 in.interactPressed = atLedge(x, 5.6f, 0.6f);
                 if (x >= 5.3f) {
                     phase = 1;
                 }
                 return in;
             }
             // 1. Revenir au puits (colonnes 1-2) avant d'entamer l'ascension.
             if (phase == 1) {
                 in.moveX = -1.0f;
                 if (x <= 1.6f) {
                     phase = 2;
                 }
                 return in;
             }
             // 2. Puits de wall jump (EX-GP-016) jusqu'au sommet (rangée 2) : même patron que
             //    demo-wall-jump, poussée systématique à l'opposé du mur touché.
             if (phase == 2) {
                 // Ne presser le saut qu'au contact d'un mur ou du sol : au sommet du puits, le
                 // mur s'arrete avant la rangee de danger (1) -- presser le saut en l'air y
                 // consommerait un saut aerien de trop, projetant le personnage dedans.
                 in.jumpPressed = player.wallDirection != 0.0f || player.grounded;
                 if (player.wallDirection != 0.0f) {
                     wallJumpLastPush = -player.wallDirection;
                 }
                 in.moveX = wallJumpLastPush;
                 if (y <= 2.7f) {
                     phase = 3;
                 }
                 return in;
             }
             // 3. Rangée 2 (au-dessus du danger de la rangée 3, colonne 4 dangereuse jusqu'à la
             //    rangée 8) : une ruée (EX-GP-017, ignore la gravité le temps de la ruée) traverse
             //    tout le danger en restant à cette hauteur, avant de retomber au-delà (colonne
             //    6+). Marche ensuite jusqu'à ce que le sol s'arrête (colonne 9) et tombe dans le
             //    vestibule de la porte verrouillée (9-10, rangée 5, sol rangée 6).
             if (phase == 3) {
                 in.moveX = 1.0f;
                 in.dashPressed = atLedge(x, 3.6f, 0.6f);
                 if (x >= 9.6f) {
                     in.moveX = 0.0f;
                 }
                 if (x >= 9.3f && y >= 4.4f) {
                     phase = 4;
                 }
                 return in;
             }
             // 4. Vestibule (9-10, 4-5) : revenir vers la porte verrouillée (8, 5) et interagir
             //    (clé #1 déjà en poche).
             if (phase == 4) {
                 in.moveX = -1.0f;
                 in.interactPressed = atLedge(x, 9.0f, 0.8f);
                 if (x <= 7.6f) {
                     phase = 5;
                 }
                 return in;
             }
             // 5. Descendre les pentes (7, 6)/(6, 7) jusqu'à la salle du bloc (rangée 11) : tenir
             //    la direction suffit, le suivi de pente fait le reste -- mais s'ARRÊTER avant la
             //    colonne du bloc (6, 10) : continuer à gauche le pousserait dans le mauvais sens
             //    (loin de la plaque). Tomber tout droit une fois passé les pentes.
             if (phase == 5) {
                 in.moveX = x > 5.3f ? -1.0f : 0.0f;
                 if (player.grounded && y >= 9.5f) {
                     phase = 6;
                 }
                 return in;
             }
             // 6. Pousser le bloc (6, 10) jusqu'à la plaque (8, 10), puis SAUTER PAR-DESSUS lui :
             //    continuer à marcher le pousserait hors de la plaque et refermerait la porte
             //    (11, 10) -- le poids doit rester, c'est tout le propos de EX-GP-025.
             if (phase == 6) {
                 // Un bloc pousse par CASE ENTIERE, au contact -- pas en glissement continu
                 // (`BlockController::pushBlocks`) : marcher jusqu'a la case 7 pousse le bloc en
                 // deux fois (6->7 a l'approche, 7->8 des que le contact reprend, PILE sur la
                 // plaque). Continuer a marcher le repousserait aussitot hors de la plaque -- il
                 // faut d'abord sauter tout DROIT (aucun elan lateral) pour degager sa hauteur
                 // avant de reprendre la marche, sans quoi le contact horizontal reste actif
                 // pendant la montee et pousse le bloc une troisieme fois.
                 if (x < 6.9f) {
                     in.moveX = 1.0f;
                 } else if (y > 9.3f) {
                     in.moveX = 0.0f;
                     in.jumpPressed = player.grounded;
                 } else {
                     in.moveX = 1.0f;
                 }
                 if (x >= 9.0f) {
                     phase = 7;
                 }
                 return in;
             }
             // 7. Rejoindre la porte (11, 10) ouverte, puis le puits à plateformes (colonnes
             //    12-14) : attendre que la plateforme soit à son point bas (rangée 11) avant d'y
             //    marcher -- la rangée 9 est un plafond plein au-dessus de la porte (sauter y cogne
             //    la tête après 0,2 case), et tant que la plateforme n'a pas atteint tout à fait sa
             //    rangée basse, elle chevauche partiellement cette hauteur et bloque comme un mur.
             if (phase == 7) {
                 in.moveX = 1.0f;
                 in.jumpHeld = true;
                 if (x >= 11.3f && !committedToLift) {
                     in.moveX = 0.0f;
                     // +1 : `PlatformController::update()` incrémente son compteur de pas AVANT
                     // que ce script ne soit consulté au pas suivant -- sans ce décalage, le calcul
                     // rate systématiquement la fenêtre de tolérance.
                     if (platformOffset(step + 1, 1.0f, 9.0f) < 0.1f) {
                         committedToLift = true;
                         in.moveX = 1.0f;
                     }
                 }
                 if (committedToLift) {
                     in.moveX = 1.0f;
                 }
                 if (x >= 13.0f) {
                     phase = 8;
                 }
                 return in;
             }
             // 8. Monter avec la plateforme (aucune entrée horizontale : elle porte) jusqu'à la
             //    rangée 2 -- la colonne de danger (15) couvre TOUTE sa hauteur (4 à 13), seule la
             //    rangée 2 (alcôve, sol étendu 15-21) passe au-dessus.
             if (phase == 8) {
                 in.moveX = 0.0f;
                 if (y <= 2.6f) {
                     phase = 9;
                 }
                 return in;
             }
             // 9. Traverser l'alcôve (15-21, rangée 2) vers la droite jusqu'à la colonne 21,
             //    attendre la fenêtre inoffensive du danger temporisé (22, 3), puis tomber par ce
             //    puits jusqu'au palier (18-23, rangée 5).
             if (phase == 9) {
                 in.moveX = x < 21.4f ? 1.0f : 0.0f;
                 constexpr int PERIOD = 120;
                 constexpr int ACTIVE = 60;
                 if (x >= 21.4f && !blinkActive(step, 0, PERIOD, ACTIVE) &&
                     !blinkActive(step + 20, 0, PERIOD, ACTIVE)) {
                     in.moveX = 1.0f;
                 }
                 if (y >= 4.15f && player.grounded) {
                     phase = 10;
                 }
                 return in;
             }
             // 10. Palier (18-23, rangée 4, sol rangée 5) : marcher à gauche, SAUTER par-dessus le
             //     danger (17, 5, juste sous le palier) au bord (colonne 18), puis continuer
             //     jusqu'à la colonne 16 et tomber par ce puits (5-6 ouvertes) jusqu'au palier
             //     (16-21, rangée 7).
             if (phase == 10) {
                 in.moveX = -1.0f;
                 // La rangée 3 est un plafond plein ici aussi (comme au-dessus de la porte,
                 // phase 7) : un saut n'y gagne que 0,2 case, jamais assez pour dégager le danger
                 // (17, 5) sous le palier. Une ruée (2,25 cases, à hauteur fixe) franchit la case
                 // d'un trait -- déclenchée à la colonne 18,7 pour atterrir au centre de la
                 // colonne 16 (18,7 - 2,25 = 16,45), sans déborder sur la colonne de danger
                 // permanent (15).
                 in.dashPressed = player.grounded && atLedge(-x, -18.7f, 0.3f);
                 if (x <= 16.3f) {
                     in.moveX = 0.0f;
                 }
                 if (y >= 6.15f && player.grounded) {
                     phase = 11;
                 }
                 return in;
             }
             // 11. Palier (16-21, rangée 6, sol rangée 7) : marcher à droite jusqu'à la colonne 21,
             //     attendre la fenêtre inoffensive du second danger temporisé (22, 7), le franchir,
             //     puis revenir IMMÉDIATEMENT vers la gauche (colonne 22 est un danger permanent à
             //     la rangée 9, seules les colonnes 18-21 y portent un sol) avant de retomber sur
             //     le palier (18-21, rangée 9).
             if (phase == 11) {
                 // La rangée 7 (sol du palier de la rangée 6) porte aussi un danger permanent
                 // (18, 7) : une ruée, déclenchée à la colonne 17,3, atterrit vers la colonne
                 // 19,55 (17,3 + 2,25), au-delà.
                 in.dashPressed = player.grounded && atLedge(x, 17.3f, 0.3f);
                 if (x < 21.4f) {
                     in.moveX = 1.0f;
                 } else if (x < 22.3f) {
                     constexpr int PERIOD = 120;
                     constexpr int ACTIVE = 60;
                     in.moveX = (!blinkActive(step, 0, PERIOD, ACTIVE) &&
                                 !blinkActive(step + 10, 0, PERIOD, ACTIVE))
                                    ? 1.0f
                                    : 0.0f;
                 } else {
                     // La chute (rangée 8, ouverte) est plus rapide que la marche : sans ruée, la
                     // rangée 9 (danger permanent à la colonne 22) est atteinte avant d'avoir
                     // assez reculé vers le palier (18-21).
                     //
                     // La ruée n'est lancée qu'une fois la boîte ENTIÈREMENT sous la rangée 7
                     // (`y >= 8`, hauteur 0,8) : tant qu'elle la chevauche, le sol du palier
                     // (19-21, rangée 7) bloque tout déplacement vers la gauche, et une ruée
                     // lancée là se consomme pour un tiers de case contre ce mur. La fenêtre est
                     // d'une seule image -- la boîte dégage la rangée 7 à `y = 8` et se pose sur
                     // la rangée 9 à `y = 8,2` -- mais la ruée fige la hauteur, ce qui suffit à
                     // franchir les 2,25 cases jusqu'au palier.
                     in.moveX = -1.0f;
                     if (!dashedBackFromSecondBlink && y >= 8.0f) {
                         dashedBackFromSecondBlink = true;
                         in.dashPressed = true;
                     } else if (x <= 20.0f) {
                         in.moveX = 0.0f;
                     }
                 }
                 if (y >= 8.15f && player.grounded) {
                     phase = 12;
                 }
                 return in;
             }
             // 12. Palier (18-21, rangée 8, sol rangée 9) : marcher à gauche jusqu'à la colonne 16,
             //     où le danger mobile vertical (16, 9, portée 3) laisse parfois passer -- tomber
             //     dès qu'il est loin, puis continuer tout droit jusqu'au palier final (16-20,
             //     rangée 13), en travers du danger mobile horizontal (16, 11, portée 6) lui aussi
             //     surveillé.
             if (phase == 12) {
                 const float horizontalMoverColumn = 16.0f + platformOffset(step, 2.0f, 6.0f);
                 // Le sol du palier s'arrête à la colonne 18 : marcher au-delà fait tomber dans le
                 // puits de la colonne 17, libre de la rangée 9 à la 12 et donnant droit sur le
                 // palier final (16-20, rangée 13). Le mobile VERTICAL reste confiné à la colonne
                 // 16 et ne croise jamais ce puits ; le seul danger du trajet est donc le mobile
                 // HORIZONTAL (16, 11, portée 6), traversé à la rangée 11.
                 //
                 // Trois cases de marge (colonne >= 19) plutôt que le simple dégagement : la chute
                 // dure une quinzaine de pas, pendant lesquels le mobile en parcourt un demi -- une
                 // marge d'une case le laisserait revenir juste à temps.
                 const bool shaftClear = horizontalMoverColumn >= 19.0f;
                 if (player.grounded && (x > 18.0f || shaftClear)) {
                     in.moveX = -1.0f;
                 }
                 // Aucune entrée une fois en l'air : la chute doit rester dans la colonne 17,
                 // puisque la 16 est le couloir du mobile vertical et la 15 un mur de danger.
                 if (y >= 12.15f && player.grounded) {
                     phase = 13;
                 }
                 return in;
             }
             // 13. Marcher jusqu'au bord du palier (colonne 20-21) puis tomber dans le puits
             //     (colonnes 21-22) jusqu'au long couloir (rangée 15).
             if (phase == 13) {
                 in.moveX = 1.0f;
                 if (x >= 21.3f) {
                     in.moveX = 0.0f;
                 }
                 // Le couloir de la rangée 15 a son sol à la rangée 16 : on s'y pose à 15,2,
                 // jamais à 15,6.
                 if (y >= 15.15f && player.grounded) {
                     phase = 14;
                 }
                 return in;
             }
             // 14. Longer le couloir (rangée 15) vers la GAUCHE jusqu'à l'ouverture (colonnes 7-8).
             if (phase == 14) {
                 in.moveX = -1.0f;
                 if (x <= 7.6f) {
                     phase = 15;
                 }
                 return in;
             }
             // 15. Tomber dans le puits (colonnes 7-9) jusqu'à la salle de l'interrupteur (rangée
             //     20).
             if (phase == 15) {
                 in.moveX = 0.0f;
                 // Sol de la salle à la rangée 20 : on s'y pose à 19,2 (boîte haute de 0,8).
                 if (y >= 19.15f && player.grounded) {
                     phase = 16;
                 }
                 return in;
             }
             // 16. Marcher jusqu'à l'interrupteur (12, 20) : bascule la porte (15, 17).
             if (phase == 16) {
                 in.moveX = 1.0f;
                 if (x >= 12.6f) {
                     phase = 17;
                 }
                 return in;
             }
             // 17. Revenir au pied du puits. La salle de l'interrupteur (rangée 20, colonnes
             //     10-12) est fermée à gauche par le bloc plein (6-9, rangée 20) : on ne longe pas
             //     la rangée 20 vers la gauche, on SAUTE sur ce bloc, dont le dessus (rangée 19)
             //     mène au pied du puits (colonnes 7-8).
             if (phase == 17) {
                 in.moveX = -1.0f;
                 // Quitter la case de l'interrupteur d'une RUÉE, jamais d'un saut : le plafond
                 // (12, 18) plafonne le saut à une case, et le personnage retombe alors SUR
                 // l'interrupteur. Or un interrupteur à bascule change d'état à chaque entrée --
                 // la porte (15, 17) se refermerait aussitôt après s'être ouverte.
                 if (x > 11.5f) {
                     in.dashPressed = player.grounded;
                 } else {
                     in.jumpPressed = player.grounded;
                 }
                 if (x <= 8.6f && player.grounded) {
                     phase = 18;
                 }
                 return in;
             }
             if (phase == 18) {
                 in.jumpPressed = player.wallDirection != 0.0f || player.grounded;
                 if (player.wallDirection != 0.0f) {
                     wallJumpLastPush = -player.wallDirection;
                 }
                 in.moveX = wallJumpLastPush;
                 // 17,15 et pas 17,4 : la boîte fait 0,8 de haut, elle ne dégage entièrement la
                 // rangée 18 qu'à partir de 17,2. Sortir du puits plus bas ferait heurter le FLANC
                 // du sol de la rangée 18 (colonnes 10-16) au lieu de passer par-dessus.
                 if (y <= 17.15f) {
                     phase = 19;
                 }
                 return in;
             }
             // 19. Traverser la rangée 17 vers la droite, à travers la porte (15, 17) désormais
             //     ouverte, jusqu'à la colonne 17 (puits vers le palier de la rangée 19-20).
             if (phase == 19) {
                 in.moveX = 1.0f;
                 // Le sol de la rangée 17 (la rangée 18) ne commence qu'à la colonne 10 : sortir du
                 // puits en marchant ferait retomber à la rangée 19 avant de l'atteindre. Une ruée
                 // à hauteur fixe franchit d'un trait les trois colonnes sans sol.
                 in.dashPressed = x < 10.0f;
                 if (x >= 17.3f) {
                     in.moveX = 0.0f;
                 }
                 // Sol à la rangée 19 : on s'y pose à 18,2.
                 if (y >= 18.15f && player.grounded) {
                     phase = 20;
                 }
                 return in;
             }
             // 20. La salle de la clé #2 (colonnes 19-22, rangées 17-19) est fermée à gauche par
             //     le mur plein de la colonne 18 (rangées 16 à 19) et par-dessus par la rangée 16 :
             //     elle ne s'atteint que PAR LE BAS. Redescendre par le trou de la colonne 14
             //     jusqu'au fond (rangée 22), puis longer vers la droite jusqu'au pied de la
             //     cheminée (colonne 19).
             if (phase == 20) {
                 // En haut, au sol : vers la GAUCHE, jusqu'au trou de la colonne 14. En chute :
                 // AUCUNE entrée -- dériver vers la gauche plaquerait le personnage contre le mur
                 // (13, 20-21) et le ferait longer le danger temporisé (13, 22) en arrivant au
                 // fond. Une fois au fond : vers la DROITE, jusqu'au pied de la cheminée.
                 if (y >= 22.15f) {
                     in.moveX = x < 19.0f ? 1.0f : 0.0f;
                 } else if (player.grounded) {
                     in.moveX = -1.0f;
                 }
                 if (y >= 22.15f && player.grounded && x >= 19.0f) {
                     phase = 21;
                 }
                 return in;
             }
             // 21. Cheminée de la colonne 19 (murs 18 et 20) : la remonter en wall jump jusqu'à la
             //     rangée 17, puis une ruée vers la droite pour se poser sur la clé (22, 17) -- les
             //     colonnes 20 et 21 n'ont pas de sol à cette rangée, seule la 22 en a un. La clé
             //     se ramasse au contact ET à l'action Interagir (EX-CTRL-022).
             if (phase == 21) {
                 if (player.wallDirection != 0.0f) {
                     wallJumpLastPush = -player.wallDirection;
                 }
                 if (y > 17.25f) {
                     in.jumpPressed = player.wallDirection != 0.0f || player.grounded;
                     in.moveX = wallJumpLastPush;
                 } else {
                     in.moveX = 1.0f;
                     in.dashPressed = x < 21.0f;
                     in.interactPressed = true;
                 }
                 if (x >= 21.6f && player.grounded) {
                     phase = 22;
                 }
                 return in;
             }
             // 22. Fond du puits (rangée 22) : le danger mobile horizontal (7, 22, portée 8) et
             //     deux dangers temporisés (9, 22 / 13, 22) gardent le passage vers la GAUCHE.
             //     Attendre que chacun soit inoffensif avant de le franchir.
             if (phase == 22) {
                 constexpr int PERIOD = 120;
                 constexpr int ACTIVE = 60;
                 constexpr int CROSSING = 30;  // pas nécessaires pour dégager une case, à 0,05/pas

                 in.moveX = -1.0f;

                 // Devant chaque danger temporisé : attendre à DROITE de sa case tant qu'il est
                 // allumé, ou qu'il le redeviendrait avant qu'on l'ait dépassé. Les deux étant en
                 // phase, cette attente est indispensable : à vitesse constante, aucun instant de
                 // départ ne dégage les deux à la fois.
                 const float holds[2] = {14.05f, 10.05f};
                 for (int index = 0; index < 2; ++index) {
                     if (x < holds[index] || x > holds[index] + 0.3f) {
                         continue;
                     }
                     if (blinkActive(step, 0, PERIOD, ACTIVE) ||
                         blinkActive(step + CROSSING, 0, PERIOD, ACTIVE)) {
                         in.moveX = 0.0f;
                         in.dashPressed = false;  // jamais s'élancer sur un danger allumé
                     }
                 }

                 if (x <= 1.6f) {
                     phase = 23;
                 }
                 return in;
             }
             // 23. Remonter le puits gauche (murs colonnes 0 et 6) en wall jump jusqu'au bloc
             //     (2, 18), seul sol de la rangée 17. Près du sommet, on quitte le va-et-vient
             //     pour viser sa verticale : sans cela le personnage monte le long du mur droit et
             //     se cogne sous le bloc plein (5, 16).
             if (phase == 23) {
                 in.jumpPressed = player.wallDirection != 0.0f || player.grounded;
                 if (player.wallDirection != 0.0f) {
                     wallJumpLastPush = -player.wallDirection;
                 }
                 in.moveX = y > 18.3f ? wallJumpLastPush : (x > 2.3f ? -1.0f : 1.0f);
                 if (y <= 17.25f && player.grounded) {
                     phase = 24;
                 }
                 return in;
             }
             // 24. Sortie. Les portes verrouillées (2-4, 16) sont déjà ouvertes : la clé #2,
             //     ramassée à la phase 21, les ouvre DÉFINITIVEMENT (EX-GP-023). Il ne reste qu'à
             //     monter au travers de l'une d'elles et à toucher la sortie (5, 15), posée sur le
             //     seul sol de la rangée 15 -- le bloc (5, 16).
             if (phase == 24) {
                 // Double saut depuis le bloc (2, 18) : il traverse les portes ouvertes (2-4, 16)
                 // et débouche à la rangée 15. La dérive vers la droite suffit alors à toucher la
                 // sortie (5, 15).
                 in.jumpPressed = player.grounded || falling;
                 // Le bloc (2, 18) ne fait qu'une case : marcher à droite en tombe. On saute
                 // d'abord à la verticale (les portes ouvertes 2-4 laissent passer), puis, une
                 // fois à la rangée 15, une ruée couvre d'un trait les trois cases qui restent
                 // jusqu'à la sortie -- la marche n'y suffirait pas avant la retombée.
                 // 15,2 et pas 15,9 : la boîte fait 0,8 de haut et ne dégage entièrement la
                 // rangée 16 qu'à partir de 15,2. Partir plus bas ferait heurter le flanc du bloc
                 // (5, 16) et s'arrêter juste avant la sortie.
                 in.moveX = y <= 15.2f ? 1.0f : 0.0f;
                 in.dashPressed = y <= 15.2f && x < 4.6f;
                 return in;
             }
             return in;
         }},
    };
}

}  // namespace
