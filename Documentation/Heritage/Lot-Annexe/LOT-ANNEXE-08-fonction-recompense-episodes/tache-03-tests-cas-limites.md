# TACHE-03 — Tests : cas limites et non-régression sur demo-*.json {#lot-annexe-08-tache-03-tests-cas-limites}

**Lot :** [LOT-ANNEXE-08](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Env` · **Statut :** à faire

## Contexte
TACHE-01/02 introduisent chacune leurs cas de test locaux ; cette tâche vérifie le comportement
combiné (récompense + classification d'épisode) sur des scénarios limites et sur les niveaux réels
du jeu, condition posée par le critère d'acceptation 4 de l'épic.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Env/test_reward_episode.cpp`** : scénarios combinés — mort
  immédiate (premier pas), complétion immédiate (personnage qui commence déjà sur la sortie, cas
  synthétique), stagnation prolongée (position figée artificiellement) menant à `Stuck`.
- **Test de non-régression sur `demo-*.json`** : pour chaque niveau de `Source/Elements/Levels/
  demo-*.json`, rejoue le script d'entrée déjà existant dans `Source/Test/Systeme/
  test_parcours_complet.cpp` via `HeadlessLevelEnvironment`, calcule la récompense cumulée à chaque
  pas (TACHE-01) et vérifie `classifyEpisode(...) == Won` avec une récompense cumulée dominée par le
  bonus de complétion (critère d'acceptation 4 de l'épic) — placé en `Source/Test/Integration`
  (dépend de fichiers de niveau réels via `JADG_LEVELS_DIR`), pas en `Unit`.

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Env/test_reward_episode.cpp` — nouveau.
- `Source/Test/Integration/test_recompense_demo_niveaux.cpp` — nouveau.
- `Source/Test/CMakeLists.txt` — ajout des nouveaux fichiers à `UnitTests` et `IntegrationTests`.

## Tests (obligatoires)
- **Mort immédiate** : récompense cumulée dominée par `deathPenalty`, `classifyEpisode(...) ==
  Lost`.
- **Complétion immédiate** : récompense cumulée dominée par `completionBonus`, `classifyEpisode(...)
  == Won`.
- **Stagnation** : après le seuil configuré de pas sans progression, `classifyEpisode(...) ==
  Stuck`, jamais `Ongoing` au-delà de ce seuil.
- **Chaque niveau `demo-*.json`** : `Won` avec récompense cumulée positive et dominée par le bonus
  de complétion, en réutilisant les scripts d'entrée déjà écrits pour `test_parcours_complet.cpp`
  (pas de réécriture des séquences d'entrée, seule la mesure de récompense est ajoutée par-dessus).

## Points d'attention
- **Réutiliser les scripts d'entrée existants de `test_parcours_complet.cpp`, ne pas les
  réécrire** : le but est de vérifier que la récompense/classification se comporte correctement sur
  des trajectoires déjà connues comme valides, pas de revalider la physique elle-même (déjà couverte
  par ailleurs).
- **Le test de stagnation construit un scénario synthétique** (personnage figé sur place par
  construction du script d'entrée, pas un niveau réel où la stagnation serait accidentelle) — un
  scénario déterministe et contrôlé, pas un niveau du jeu détourné pour l'occasion.

## Définition de fait (DoD)
- Les deux suites de tests vertes (`ctest`) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-009` déclarée dans l'`epic.md` du lot.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (récompense, conception d'une fonction de récompense
(*shaping*, *reward hacking*), épisode et horizon), en particulier sa section 6.2 (*reward hacking*
: l'agent optimise la lettre de la récompense, pas l'intention).

## Exigences
`EX-IA-009` (nouvelle, du même lot).
