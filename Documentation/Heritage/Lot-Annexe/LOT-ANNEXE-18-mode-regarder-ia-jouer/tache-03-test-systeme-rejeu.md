# TACHE-03 — Test système : le rejeu aboutit à Won {#lot-annexe-18-tache-03-test-systeme-rejeu}

**Lot :** [LOT-ANNEXE-18](epic.md) · **Emplacement :** `Source/Test/Systeme` · **Statut :** fait

## Contexte
`test_parcours_complet.cpp` prouve, pour la séquence `demo-*.json`, que des scripts d'entrée codés
en dur mènent bien à `Won` sur chaque niveau. Cette tâche apporte la preuve symétrique pour un
fichier de rejeu réel : que la relecture d'un fichier produit par un algorithme d'entraînement
(génération 2 ou 3) reproduit fidèlement, à travers `GameSession`/`ReplayPlayback` (TACHE-01), la
victoire observée à l'entraînement.

## Travail à réaliser
- **`Source/Test/Systeme/test_rejeu_ia.cpp`** (nouveau) : charge un fichier de rejeu de test fixe
  (produit une fois, à l'écriture de ce test, par un entraînement réel sur un niveau simple —
  committé comme fixture, à l'image des fichiers `demo-*.json` de `Source/Elements/Levels`),
  construit un `hmi::GameSession` et un `hmi::ReplayPlayback` sur ce fichier, boucle
  `ReplayPlayback::nextInput()` → `GameSession::update(playerInput, fixedDelta)` jusqu'à `Won`/
  `Lost` ou épuisement de la séquence, et vérifie `core::LevelOutcome::Won`.
- Fixture de rejeu de test : `Source/Test/Systeme/Fixtures/rejeu-test-deplacement.json` (ou
  équivalent), généré une fois manuellement (ou via `LOT-ANNEXE-19`, CLI, une fois ce lot livré) à
  partir d'un modèle entraîné sur un niveau simple et déterministe.

## Fichiers impactés
- `Source/Test/Systeme/test_rejeu_ia.cpp` — nouveau.
- `Source/Test/Systeme/Fixtures/rejeu-test-deplacement.json` (ou chemin équivalent) — nouveau,
  fixture committée.
- `Source/Test/CMakeLists.txt` — ajout du nouveau fichier à la cible `SystemTests`, avec le même
  mécanisme `JADG_LEVELS_DIR` déjà en place pour localiser le niveau référencé par la
  fixture.

## Tests (obligatoires)
- **Rejeu de la fixture aboutit à `Won`** : condition d'acceptation directe, symétrique à
  `test_parcours_complet.cpp`.
- **Rejeu invalidé si la fixture de niveau change** : un test complémentaire modifie
  volontairement (dans une copie temporaire, pas le fichier committé) le contenu du niveau
  référencé et vérifie que `ReplayPlayback`/`aisolver::validateReplay` (`LOT-ANNEXE-17`) refuse la
  lecture, cohérent avec le critère d'acceptation 2 de l'épic.

## Points d'attention
- **La fixture de rejeu doit rester stable dans le temps** : si `Core` évolue de façon à changer la
  physique (peu probable une fois le moteur stabilisé, mais possible), ce test peut casser
  légitimement — dans ce cas, régénérer la fixture (via un entraînement réel ou `LOT-ANNEXE-19`
  CLI) plutôt que d'ajuster artificiellement le test.
- **Ce test est le pendant système du test unitaire d'équivalence des deux surcharges de
  `GameSession::update`** (TACHE-01) : celui-là vérifie l'équivalence mécanique, celui-ci vérifie le
  résultat de bout en bout sur un rejeu réel — les deux sont nécessaires, aucun ne remplace l'autre.

## Définition de fait (DoD)
- `test_rejeu_ia.cpp` vert (`ctest`), fixture committée et documentée ; build `/W4 /WX` sans
  avertissement ; `EX-IA-019` déclarée dans l'`epic.md` du lot.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-019` (nouvelle, du même lot).
