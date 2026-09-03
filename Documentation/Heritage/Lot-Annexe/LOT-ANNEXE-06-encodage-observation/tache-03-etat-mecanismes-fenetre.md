# TACHE-03 — État des mécanismes actifs dans la fenêtre {#lot-annexe-06-tache-03-etat-mecanismes-fenetre}

**Lot :** [LOT-ANNEXE-06](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
La fenêtre de tuiles (TACHE-01) encode le **type statique** de chaque case (`core::TileType::Door`,
`PressurePlate`, `DangerSwitched`…), mais pas son **état dynamique** : une porte fermée et une porte
ouverte partagent le même `TileType::Door` dans `TileMap` (seule la grille de collision, recomposée à
chaque pas par `core::MechanismController`, change). Sans ce canal, l'agent ne pourrait jamais
distinguer une porte franchissable d'un mur. Cette tâche superpose cet état à la fenêtre.

## Travail à réaliser
- **Étendre `HeadlessLevelEnvironment`** (`Source/AiSolver/Env/HeadlessLevelEnvironment.h`,
  LOT-ANNEXE-05) de deux accesseurs en lecture seule : `[[nodiscard]] const core::
  MechanismController& mechanisms() const noexcept;` et `[[nodiscard]] const core::DangerController&
  dangers() const noexcept;` — jusqu'ici internes, nécessaires pour que l'encodeur interroge l'état
  courant sans dupliquer la simulation (`isDoorOpen`, `isDangerActive`, `moverBox`, `isBlinkActive`).
- **`aisolver::MechanismStateEncoder`** (`Source/AiSolver/Env/MechanismStateEncoder.h/.cpp`) :
  - `[[nodiscard]] aisolver::Tensor encode(const core::MechanismController& mechanisms, const core::
    DangerController& dangers, const core::Level& level, core::GridPosition center, int radius)
    const;` — renvoie un tenseur `(2, 2·radius + 1, 2·radius + 1)`, même fenêtre géométrique que
    `TileWindowEncoder` (TACHE-01), deux canaux :
    - **Canal 0 (« porte ouverte »)** : pour chaque case de la fenêtre correspondant à
      `mechanism.doorPosition` d'un `core::Mechanism` de `mechanisms.mechanisms()`,
      `mechanisms.isDoorOpen(index) ? 1.0f : 0.0f` ; `0.0f` sur toute case qui n'est pas une porte.
    - **Canal 1 (« danger actif »)** : `1.0f` sur toute case de la fenêtre recouverte par (a) une
      boîte de danger mobile courante (`dangers.moverBox(index)`, rastérisée sur les cases qu'elle
      chevauche), (b) une case `DangerBlink` dont `dangers.isBlinkActive(position)` est vrai, ou (c)
      une case `DangerSwitched` dont `mechanisms.isDangerActive(position)` est vrai ; `0.0f` ailleurs
      — un danger **statique** (`TileType::Danger`/`DangerUp`/…/`DangerRight`) n'active **pas** ce
      canal (il est déjà identifiable sans ambiguïté par son canal catégoriel dans `TileWindowEncoder`,
      toujours actif : pas besoin de le dupliquer).
  - Concaténé au tenseur de `TileWindowEncoder` par l'appelant (assembleur d'observation complet, à
    la charge de qui consomme les trois encodeurs de ce lot — la classe d'assemblage elle-même
    revient à LOT-ANNEXE-10/11, premiers consommateurs).

## Fichiers impactés
- `Source/AiSolver/Env/HeadlessLevelEnvironment.h`/`.cpp` (deux accesseurs).
- `Source/AiSolver/Env/MechanismStateEncoder.h` (nouveau).
- `Source/AiSolver/Env/MechanismStateEncoder.cpp` (nouveau).
- Tests : `Source/Test/Unit/AiSolver/Env/test_mechanism_state_encoder.cpp` (nouveau).

## Tests (obligatoires)
- **Porte fermée puis ouverte** (`demo-interrupteur.json`) : avant contact avec l'interrupteur, le
  canal 0 à la position de la porte vaut `0.0f` ; après (`mechanisms.update` appelé), il vaut `1.0f`.
- **Plaque de pression** (`demo-plaque-pression.json`) : le canal 0 revient à `0.0f` dès que le
  personnage quitte la plaque (activation continue, `EX-GP-025`), à la différence de l'interrupteur.
- **Danger mobile rastérisé** (`demo-dangers-avances.json`) : le canal 1 s'active/se désactive sur
  les cases traversées par `moverBox`, en cohérence avec sa trajectoire déterministe (`EX-GP-051`).
- **Danger temporisé** : le canal 1 alterne `0.0f`/`1.0f` à la case du danger selon le cycle
  `period`/`phase`/`activeDuration` (`EX-GP-053`), reproductible par calcul du pas attendu.
- **Fenêtre sans aucun mécanisme** : un niveau sans interrupteur ni danger avancé (`demo-pente.json`)
  produit un tenseur entièrement nul sur les deux canaux — pas d'erreur, pas de cas particulier.
- **Déterminisme** : deux appels sur le même état (`mechanisms`/`dangers`/`center` identiques)
  produisent des tenseurs bit-à-bit identiques.

## Points d'attention
- **Le danger mobile n'a pas de case fixe** : contrairement à `DangerSwitched`/`DangerBlink` (une
  case précise dans `TileMap`), sa boîte mortelle se déplace en continu — la rastérisation sur le
  canal 1 doit suivre `moverBox(index)`, jamais la case `DangerMover` d'origine (qui ne représente
  que le point de départ, voir `Core/Levels/TileType.h`).
- **Ne pas dupliquer les dangers statiques sur le canal « danger actif »** : ils sont invariants et
  déjà représentés sans ambiguïté par leur `TileType` — le canal 1 n'apporte de l'information que là
  où l'état **varie** dans le temps (voir décision de cadrage : mobile, commuté, temporisé).
- **`radius` doit être identique entre `TileWindowEncoder` et `MechanismStateEncoder`** pour que les
  deux tenseurs soient concaténables sans redimensionnement — pas garanti par le type, à la charge de
  l'appelant (documenté ici, vérifié par les tests d'intégration du premier consommateur).

## Définition de fait (DoD)
- `HeadlessLevelEnvironment` étendu et `MechanismStateEncoder` testés (`ctest` vert) sur les cas
  ci-dessus ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (état et observation, conception d'une observation :
one-hot, normalisation, observation partielle), en particulier sa section 3.1 (complétude contre
taille de l'observation).

## Exigences
`EX-IA-006` (nouvelle).
