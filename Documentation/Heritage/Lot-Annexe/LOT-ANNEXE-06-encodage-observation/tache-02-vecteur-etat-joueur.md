# TACHE-02 — Vecteur d'état joueur {#lot-annexe-06-tache-02-vecteur-etat-joueur}

**Lot :** [LOT-ANNEXE-06](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
La fenêtre de tuiles (TACHE-01) décrit l'**environnement immédiat** du personnage, mais rien de sa
**cinématique propre** (vitesse, contact au sol, mur touché, minuteries de *game feel*, budgets
restants) — pourtant déterminante pour décider de l'action suivante (ex. ne pas re-sauter tant que
`coyoteTimer` est épuisé). Cette tâche encode ce second bloc, lu depuis `core::Player`/`core::
Velocity`, tels qu'exposés par `HeadlessLevelEnvironment::StepObservation` (LOT-ANNEXE-05).

## Travail à réaliser
- **`aisolver::PlayerStateEncoder`** (`Source/AiSolver/Env/PlayerStateEncoder.h/.cpp`) :
  - `[[nodiscard]] aisolver::Tensor encode(const core::Player& player, const core::Velocity&
    velocity, const core::Level& level) const;` — renvoie un vecteur (tenseur à une dimension) de
    taille fixe `kPlayerStateSize`, dans cet ordre documenté :
    1. `velocity.value.x`, `velocity.value.y` (bruts, unités monde/s — non normalisés : leur échelle
       est déjà bornée par le modèle physique, `EX-GP-019`, sans plafond artificiel à reproduire ici).
    2. `grounded ? 1.0f : 0.0f`.
    3. `wallDirection` (déjà dans `{-1, 0, 1}`, utilisé tel quel).
    4. `coyoteTimer / kNominalCoyoteTime`, `jumpBufferTimer / kNominalJumpBufferTime`,
       `wallJumpLockTimer / kNominalWallJumpLockTime`, `dashTimer / kNominalDashDuration` — quatre
       constantes `constexpr float` nommées, propres à `AiSolver` (voir décision de cadrage de
       l'épic : pas d'accès à `PhysicsConfig`, privé à `CharacterPhysicsSystem`), calibrées sur les
       valeurs par défaut documentées de `Documentation/Specification/gameplay.md` (§ ressenti).
    5. `dashAvailable ? 1.0f : 0.0f`.
    6. Budget de saut normalisé : `level.jumpBudget() < 0 ? 1.0f : static_cast<float>(player.
       jumpsRemaining) / static_cast<float>(level.jumpBudget())`. Même formule pour le budget de dash
       (`level.dashBudget()`/`player.dashesRemaining`).
  - `[[nodiscard]] static constexpr int size() noexcept { return kPlayerStateSize; }` (= `10` avec
    l'ordre ci-dessus) — exposé pour l'assemblage du tenseur complet et pour la couche d'entrée du
    réseau (LOT-ANNEXE-03), qui a besoin d'une dimension connue à la construction.

## Fichiers impactés
- `Source/AiSolver/Env/PlayerStateEncoder.h` (nouveau).
- `Source/AiSolver/Env/PlayerStateEncoder.cpp` (nouveau).
- Tests : `Source/Test/Unit/AiSolver/Env/test_player_state_encoder.cpp` (nouveau).

## Tests (obligatoires)
- **Ordre et taille fixes** : `PlayerStateEncoder::size() == 10` ; un `core::Player` par défaut
  produit un vecteur dont chaque composant correspond exactement à la formule documentée (vérifié
  composant par composant, pas seulement la taille).
- **Budget illimité** : `level.jumpBudget() == -1` produit `1.0f` à la position du budget de saut,
  quel que soit `player.jumpsRemaining` (qui n'est pas décrémenté en pratique quand illimité).
- **Budget fini partiellement consommé** : `jumpBudget() == 2`, `jumpsRemaining == 1` produit `0.5f`.
- **Timers à zéro** : un `core::Player` fraîchement apparu (toutes minuteries à `0.0f`, comme après
  `HeadlessLevelEnvironment::reset`) produit des composants de timer tous à `0.0f`.
- **Déterminisme** : deux appels sur le même `core::Player`/`core::Velocity` produisent des vecteurs
  bit-à-bit identiques.

## Points d'attention
- **`wallDirection`/`grounded`/`dashAvailable` ne sont pas normalisés par une constante** : ce sont
  déjà des grandeurs bornées par construction (`{-1, 0, 1}` ou booléen) — leur appliquer une
  normalisation supplémentaire n'apporterait rien et compliquerait la relecture du vecteur.
- **Les constantes `kNominal…`** sont un point de recalibration explicite si `PhysicsConfig` change
  ses valeurs par défaut (aucun couplage automatique, voir décision de cadrage de l'épic) : documentées
  en commentaire avec la valeur `PhysicsConfig` dont elles s'inspirent, pour qu'une revue future
  retrouve facilement le lien.
- **La vitesse n'est volontairement pas plafonnée/normalisée** : contrairement aux timers, il
  n'existe plus de plafond arbitraire depuis LOT-19 (traînée, vitesse terminale émergente) — imposer
  une normalisation par une constante fixe romprait ce choix de conception au niveau `Core`.

## Définition de fait (DoD)
- `PlayerStateEncoder` compile et est testé (`ctest` vert) sur les cas ci-dessus ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (état et observation, conception d'une observation :
one-hot, normalisation, observation partielle), en particulier ses sections 3.3 (mise à la même
échelle des grandeurs continues) et 3.4 (observation partielle : pourquoi la vitesse fait partie de
l'observation).

## Exigences
`EX-IA-006` (nouvelle).
