// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Ecs/Components/Player.h
 * @brief Composant marqueur du personnage jouable (données pures).
 */

namespace core {

/**
 * @brief Marque l'entité **contrôlable par le joueur** et porte son état de contact au sol.
 *
 * Donnée pure sans logique (`EX-ARCH-011`). Sa présence sur une entité signale qu'elle est
 * pilotée par l'intention d'entrée (::core::PlayerInput).
 *
 * @note Ses champs décrivent encore l'état d'un personnage de **plateforme** (contact au sol,
 *       coyote time, jump buffering, dash, combos) : le `LOT-01` a retiré le système qui les
 *       calculait, ils sont donc **inertes** jusqu'à ce que le `LOT-06` remplace ce composant par
 *       son équivalent en vue de dessus. Les conserver tels quels évite de toucher au rendu, à
 *       l'animation et aux événements dans un lot qui ne fait que retirer.
 */
struct Player {
    /// Vrai si le personnage repose sur une surface solide ; calculé par la physique.
    bool grounded = false;
    /// Temps restant (secondes) pendant lequel un saut est encore permis après avoir quitté le
    /// sol (*coyote time*) ; rechargé au contact du sol, décompté par la physique au pas fixe.
    float coyoteTimer = 0.0f;
    /// Temps restant (secondes) de validité d'un saut pré-appuyé avant l'atterrissage
    /// (*jump buffering*) ; rechargé à l'appui, décompté par la physique au pas fixe.
    float jumpBufferTimer = 0.0f;
    /// Sauts **aériens** restants (double/multi saut, `EX-GP-015`) ; rechargé au contact du sol.
    int airJumpsRemaining = 0;
    /// Orientation courante : -1 (gauche) / +1 (droite) ; sert de direction de dash par défaut.
    float facing = 1.0f;
    /// Sens du **mur** au contact en l'air : -1 (mur à gauche) / +1 (à droite) / 0 (aucun),
    /// calculé par la physique (`EX-GP-016`).
    float wallDirection = 0.0f;
    /// Temps restant (secondes) de **verrouillage** du contrôle horizontal après un wall jump
    /// (la vitesse d'éjection persiste tant qu'il n'est pas écoulé).
    float wallJumpLockTimer = 0.0f;
    /// Charges de **dash** restantes avant de retoucher le sol (`EX-GP-017`, `EX-GP-055`) ;
    /// consommées une par dash, toutes rechargées au contact du sol. Un compteur, et non plus un
    /// booléen : un tableau peut accorder plusieurs dashs par saut (`PhysicsConfig::dashCharges`).
    int dashChargesRemaining = 0;
    /// Durée restante (secondes) du dash en cours ; > 0 pendant la ruée (gravité suspendue).
    float dashTimer = 0.0f;
    /// Sauts **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**. Décompté par
    /// la physique ; initialisé au spawn depuis le niveau.
    int jumpsRemaining = -1;
    /// Dashs **restants** dans le tableau (budget, `EX-GP-024`) ; **-1 = illimité**.
    int dashesRemaining = -1;
    /// Masse du personnage, en unité de jeu arbitraire (`EX-GP-019`). Détermine, avec la traînée
    /// de `PhysicsConfig`, la vitesse terminale de chute (poids = masse × gravité effective) ; une
    /// masse plus grande tombe plus vite. Sert aussi de seuil pour les mécanismes sensibles au
    /// poids (plaque de pression, `EX-GP-025`).
    float mass = 1.0f;
    /// Étendue horizontale (bords gauche/droit, unité monde) couverte par la boîte depuis le
    /// **début de la montée courante** (dernier saut), pas seulement le pas précédent — remise à
    /// l'étendue courante à chaque pas où le personnage est au sol ou ne monte pas
    /// (`velocity.y >= 0`), puis étendue à chaque pas de montée. Sans cette mémoire, marcher tout
    /// en sautant sous un arrondi de plafond peut faire « disparaître » une colonne pourtant
    /// franchie plus tôt dans la même montée, avant que le seuil vertical de blocage n'y soit
    /// atteint (`core::resolveCeilingSlopeFollow`, `EX-GP-007`).
    float ascentSweepMinX = 0.0f;
    float ascentSweepMaxX = 0.0f;
    /// Vrai pendant le pas où un saut (sol, coyote, mur ou aérien) vient de se déclencher ; remis
    /// à faux au début du pas suivant. Contrairement aux autres champs, ne porte aucun état de
    /// simulation persistant — c'est un **front** à usage externe (`hmi::GameEvents`, `LOT-60`) :
    /// aucune des minuteries existantes (`jumpBufferTimer`...) ne permet de distinguer un saut
    /// déclenché d'un buffer simplement expiré sans saut.
    bool justJumped = false;
    /// Vrai pendant le pas où le personnage a été **écrasé** par une plateforme mobile contre un
    /// plafond (`EX-GP-026`, cas d'écrasement — décision de cadrage : mortel, comme un danger).
    /// Même nature que `justJumped` : un front à usage externe, remis à faux au début du pas
    /// suivant, sans effet sur la simulation elle-même (l'appelant traduit ce champ en issue
    /// `LevelOutcome::Lost`, `core::evaluateOutcome`).
    bool squished = false;
    /// Temps accumulé (secondes) en tenant la direction **opposée** à `dashChargeReferenceFacing`
    /// (`EX-GP-056`) ; remis à zéro dès que l'opposition cesse **avant** que la charge ne soit
    /// complète (bankée).
    float dashChargeTimer = 0.0f;
    /// Orientation de référence contre laquelle l'opposition est mesurée pour charger un dash
    /// (`EX-GP-056`) : figée au **début** de la charge (`facing` juste avant que ce pas ne le mette
    /// à jour), jamais `facing` lui-même -- `facing` continue de suivre l'entrée à l'identique
    /// d'avant ce lot (sprite/caméra/direction de dash par défaut inchangés hors charge active).
    float dashChargeReferenceFacing = 1.0f;
    /// Vrai dès que la charge de dash est complète : le **prochain** dash sera boosté (vitesse et
    /// durée majorées), quelle que soit la direction tenue au moment où il se déclenche. Consommée
    /// (remise à `false`) par ce dash, qu'il vise ou non `dashBoostFacing`.
    bool dashBoostReady = false;
    /// Orientation figée au moment où `dashBoostReady` passe à vrai (copie de
    /// `dashChargeReferenceFacing`) : sert de direction par défaut au dash boosté si l'entrée est
    /// neutre au déclenchement.
    float dashBoostFacing = 1.0f;
    /// Vrai si le dash **actuellement en cours** (`dashTimer > 0`) a été déclenché boosté
    /// (`EX-GP-056`). Conditionne le jump-cancel et la poussée renforcée (`EX-GP-061`/`EX-GP-057`
    /// côté appelant) : un dash **normal**, non boosté, se comporte exactement comme avant ce lot
    /// (aucune régression sur du contenu qui n'utilise jamais la charge, puisqu'il ne peut jamais
    /// produire de dash boosté).
    bool dashIsBoosted = false;
    /// Vrai pendant un **ground pound** (`EX-GP-058`) : chute verticale à vitesse imposée, en l'air
    /// uniquement, jusqu'au contact du sol (remis à `false` par la physique à l'atterrissage).
    bool groundPounding = false;
    /// Vitesse horizontale à **conserver** pendant le verrou de jump-cancel (`EX-GP-061`) :
    /// capturée au moment où un saut interrompt un dash en cours (au lieu d'attendre son
    /// expiration), pour que le saut résultant reparte de la vitesse du dash plutôt que de la
    /// physique normale.
    float dashJumpMomentumX = 0.0f;
    /// Temps restant (secondes) du verrou de jump-cancel : tant qu'il court, le contrôle horizontal
    /// normal est suspendu au profit de `dashJumpMomentumX` (même patron que `wallJumpLockTimer`).
    float dashJumpLockTimer = 0.0f;
    /// Nombre de jump-cancels enchaînés sans contact au sol (`EX-GP-061`) : incrémenté à chaque
    /// nouveau jump-cancel dans la fenêtre de combo (`comboWindowTimer`), remis à zéro au contact
    /// du sol.
    int comboChainCount = 0;
    /// Temps restant (secondes) de la fenêtre de combo courante : tant qu'il court, un nouveau
    /// jump-cancel est considéré comme « rapproché » du précédent (bonus cumulatif de vitesse).
    float comboWindowTimer = 0.0f;
    /// Temps restant (secondes) de la fenêtre d'héritage de momentum après une **poussée
    /// renforcée** (`EX-GP-057`/`EX-GP-061`) : un saut déclenché avant expiration hérite d'une
    /// fraction de `pushMomentumVelocityX` (`PhysicsConfig::momentumCarryRatio`).
    float pushMomentumWindowTimer = 0.0f;
    /// Vitesse horizontale (signée) du bloc poussé par la dernière poussée renforcée, à hériter par
    /// le prochain saut si `pushMomentumWindowTimer` n'a pas expiré.
    float pushMomentumVelocityX = 0.0f;
};

}  // namespace core
