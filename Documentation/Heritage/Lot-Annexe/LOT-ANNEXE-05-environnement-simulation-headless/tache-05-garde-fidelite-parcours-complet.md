# TACHE-05 — Garde de fidélité pas-à-pas (CI permanente) {#lot-annexe-05-tache-05-garde-fidelite-parcours-complet}

**Lot :** [LOT-ANNEXE-05](epic.md) · **Emplacement :** `Source/Test/Systeme` · **Statut :** à faire

## Contexte
`HeadlessLevelEnvironment` (TACHE-01/02) est une réplique **indépendante** de l'orchestration de
`playLevel()`/`hmi::GameSession::update` (décision de cadrage de l'épic : pas de partage de code,
pour ne pas faire dépendre `HMI` d'`AiSolver`). Une réplique indépendante peut diverger silencieusement
au fil des lots futurs (`Core` évolue, `playLevel()` est mis à jour, `HeadlessLevelEnvironment` oublié
— ou l'inverse). Sans garde automatisée, cette dérive ne serait détectée qu'au moment où un agent
entraîné en headless échouerait, inexplicablement, une fois rejoué en jeu. Cette tâche transforme ce
risque en échec de CI **immédiat et localisé** (le pas exact où ça diverge, pas seulement « le niveau
a échoué »).

## Travail à réaliser
- **Étendre `playLevel()`** (`Source/Test/Systeme/test_parcours_complet.cpp`) pour qu'il construise
  également un `core::DangerController` et assemble les boîtes de danger actif (même logique que
  `hmi::GameSession::collectActiveDangerBoxes`, voir TACHE-02), passées à `core::evaluateOutcome` —
  jusqu'ici, `playLevel()` était un sous-ensemble de l'orchestration réelle (pas de dangers avancés) ;
  cette extension en fait un miroir **complet** de `hmi::GameSession::update`, condition nécessaire
  pour qu'une comparaison pas-à-pas avec `HeadlessLevelEnvironment` ait un sens sur les 15 niveaux de
  la séquence, y compris `demo-dangers-avances.json`.
- **Nouveau test** `ParcoursCompletSystemeHeadlessEnvironment.FideliteParPas` (même fichier) : pour
  chaque `ScriptedLevel` de la table existante (`sequence`), exécute en parallèle, pas par pas :
  (a) l'orchestration de référence étendue ci-dessus (`playLevel()` internalisée pas-à-pas plutôt que
  jusqu'à l'issue finale) et (b) `aisolver::HeadlessLevelEnvironment::step` avec **exactement** le
  même `core::PlayerInput` calculé par le script réactif du niveau. Après chaque pas, `EXPECT_EQ`
  (ou `EXPECT_NEAR` avec une tolérance nulle/`epsilon` machine pour les flottants) sur la position
  (`Transform::position.x`/`.y`), la vitesse (`Velocity::value.x`/`.y`) et l'issue
  (`core::LevelOutcome`) — échec au **premier** pas divergent, avec le numéro de pas et le nom du
  niveau dans le message d'échec.
- **Réutilise `JADG_LEVELS_DIR`** (déjà défini sur `SystemTests`, TACHE-04) pour charger les
  mêmes fichiers `demo-*.json` des deux côtés.

## Fichiers impactés
- `Source/Test/Systeme/test_parcours_complet.cpp` (extension de `playLevel()` + nouveau `TEST`).
- `Source/Test/CMakeLists.txt` (déjà mis à jour en TACHE-04 : `AiSolver` lié à `SystemTests`).

## Tests (obligatoires)
- **Parité complète de la séquence** : les 15 niveaux de `sequence` (LOT-25) sont rejoués sans
  aucune divergence pas-à-pas entre les deux orchestrations, jusqu'à l'issue `Won` de chacun.
- **Le test échoue si on le casse volontairement** (vérification manuelle en revue, pas un `TEST`
  livré) : introduire un écart délibéré dans `HeadlessLevelEnvironment::step` (ex. omettre l'étape
  sweep boîte-boîte) fait échouer `FideliteParPas` sur le premier niveau à bloc réduit, avec le pas
  exact indiqué — preuve que la garde détecte réellement une divergence, pas seulement qu'elle passe
  par accident.
- **Non-régression** : `ParcoursCompletSysteme.FranchitTouteLaSequence` (test existant) reste vert
  sans modification de son propre comportement (seule `playLevel()`, sa dépendance interne, est
  étendue — de façon strictement additive pour les niveaux qui n'utilisent pas de danger avancé).

## Points d'attention
- **Comparaison au pas, pas seulement à l'issue finale** : c'est la différence explicite avec
  `ParcoursCompletSysteme.FranchitTouteLaSequence` (existant, qui ne vérifie que `Won` en sortie de
  boucle) — deux orchestrations peuvent atteindre la même issue finale par des trajectoires
  différentes (compensation d'erreurs), ce que la comparaison finale seule ne détecterait jamais.
- **Extension de `playLevel()` strictement additive.** Ajouter le `DangerController` ne doit rien
  changer au comportement des 14 niveaux qui n'ont pas de danger avancé (boîtes actives vides →
  `evaluateOutcome` se comporte comme avant) ; seul `demo-dangers-avances.json` est concerné, et son
  script (`rightOnly()`, couloir principal) ne croise aucun danger actif — l'issue `Won` déjà vérifiée
  par le test existant doit rester inchangée.
- **Cette garde ne remplace pas une revue de code** sur les futurs changements de
  `hmi::GameSession::update` ou de `Core` : elle détecte une divergence de **comportement
  observable** (position, vitesse, issue), pas une divergence de code qui resterait sans effet
  observable sur ces niveaux
  précis.

## Définition de fait (DoD)
- `playLevel()` étendu et `FideliteParPas` verts en CI (`ctest`) sur les 15 niveaux ; test existant
  `FranchitTouteLaSequence` toujours vert ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — agent, environnement, boucle `reset`/`step`, épisode,
propriété de Markov.

## Exigences
`EX-IA-005` (nouvelle).
