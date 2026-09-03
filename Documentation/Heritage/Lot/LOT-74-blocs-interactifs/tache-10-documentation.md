# TACHE-10 — Documentation, exigences, CHANGELOG {#lot-74-tache-10-documentation}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Documentation/`, `CHANGELOG.md`,
`Source/*/README.md` · **Statut :** fait

## Contexte
Les trois exigences du lot sont **citées** par l'épic et par les tâches depuis le premier jour ; elles
doivent être **déclarées** dans les spécifications, faute de quoi `scripts/lint_exigences.py` échoue
sur des références orphelines. Cette tâche ferme la boucle documentaire et clôt le lot.

Numéros retenus : `EX-GP-027`, `EX-GP-028`, `EX-GP-029` — libres, et surtout ils prolongent
**exactement** la section « 3. Mécanismes de puzzle » de `gameplay.md`, qui s'arrête aujourd'hui à
`EX-GP-026` (plateforme mobile). Les trois blocs volatils y sont à leur place : ce sont des
mécanismes, pas des dangers ni de la géométrie. Reconfirmer la disponibilité par
`python scripts/lint_exigences.py --next` avant d'écrire.

## Travail à réaliser
- `Documentation/Specification/gameplay.md`, fin de la section 3 : les trois exigences, au format
  **exact** de leurs voisines — la commande d'ancre Doxygen et son identifiant, puis l'identifiant
  répété en gras, le texte, et enfin « Concrétisé en `LOT-74`. ». Y consigner les décisions de cadrage qui ont valeur de règle, en
  particulier : armement par tout contact et aller simple du bloc descendant ; ground pound comme
  **seul** briseur du bloc fragile ; caractère **définitif** de la disparition du bloc éphémère et
  règle retenue pour un retour pendant le compte à rebours (TACHE-05).
- `Documentation/Specification/niveaux.md` : les trois noms JSON (`sinkingBlock`, `fragileBlock`,
  `vanishingBlock`) dans la longue énumération des types de tuile, avec le champ optionnel `speed` du
  bloc descendant et son défaut — même forme rédactionnelle que `dangerBlink` ou `movingPlatform`.
- `Documentation/Specification/editeur-niveaux.md` : le sous-groupe « Bloc volatil » de la palette.
- `Documentation/Guide/` : le guide de physique/niveaux touché par les deux nouveaux contrôleurs.
- `Documentation/Lot/lots.md` : **deux** ajouts distincts — la ligne `- @subpage lot-74` à la fin de
  la liste `## Lots`, **et** un paragraphe en **tête** de la section « Apres le programme `0.1.0` »,
  qui est antéchronologique (le plus récent en premier). Le paragraphe suit la forme des voisins :
  `Le [LOT-74](@ref lot-74) — **livré** — …`, avec le déclencheur (l'exclusion consignée par le
  `LOT-72`), les exigences, et ce que le lot a coûté ou corrigé en route.
- `CHANGELOG.md`, sous `## [Non publié]` : une entrée `- **Gameplay — blocs interactifs volatils**
  (`LOT-74`, `EX-GP-027` à `EX-GP-029`). …` avec une sous-puce par bloc, plus une sous-puce sur la
  **rupture de compatibilité des modèles d'IA** (36 canaux d'observation, réentraînement nécessaire)
  — c'est le seul point du lot qui casse quelque chose pour l'utilisateur.
- `Source/Core/Gameplay/README.md` (deux contrôleurs) et `Source/Elements/Levels/README.md` (trois
  tableaux), si TACHE-03/05/08 ne l'ont pas déjà fait.
- **Une fois TACHE-01 à TACHE-09 réellement terminées** : repasser sur `epic.md` — statut « fait »
  avec sa preuve de vérification chiffrée (nombre de tests `ctest`, linters, ce qui reste à vérifier
  à la main), et les dix cases du découpage en ✅.

## Fichiers impactés
- `Documentation/Specification/{gameplay,niveaux,editeur-niveaux}.md`, `Documentation/Guide/`.
- `Documentation/Lot/lots.md`, `Documentation/Lot/LOT-74-blocs-interactifs/epic.md`.
- `CHANGELOG.md`, `Source/Core/Gameplay/README.md`, `Source/Elements/Levels/README.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` vert : les trois exigences sont déclarées **et** référencées,
  aucune référence orpheline.
- Doxygen vert (`WARN_AS_ERROR = FAIL_ON_WARNINGS`) : les dix ancres de tâche correspondent aux dix
  `@subpage` de l'épic, `lot-74` est atteignable depuis `lots.md`, et aucun `@ref` n'est cassé.
- `python scripts/generate_cahier_test.py` rejoué après TACHE-09.
- Les autres linters (`check_demo_sequence.py`, `check_design_tokens.py`, `check_qt_version_pin.py`).

## Points d'attention
- **Ne pas marquer l'épic « fait » avant que les neuf autres tâches le soient réellement** — le
  statut est une affirmation vérifiable, pas une intention.
- Suivre le format des exigences **sans en dévier** : `EX-GP-026` en est l'exemple le plus proche
  (mécanisme, position continue, décision de cadrage consignée dans le texte même de l'exigence).
- `Documentation/CahierTest.md` est **généré** (plus de 900 Ko) : ne jamais l'éditer à la main.
- Le paragraphe de `lots.md` : les entrées récentes de ce fichier sont rédigées sans accents, par
  incohérence assumée. S'aligner sur le paragraphe du `LOT-72`, accentué, plutôt que de propager
  l'écart.
- Dire dans le CHANGELOG ce que le lot **n'a pas** fait autant que ce qu'il a fait : pas de
  réapparition, pas de casse au dash, pas de réentraînement des modèles livrés. Ces trois exclusions
  seront les premières questions posées à l'usage.

## Définition de fait (DoD)
- Exigences déclarées, spécifications et guides à jour, `lots.md` et `CHANGELOG.md` complétés, épic
  passé en « fait » avec ses dix cases cochées ; tous les linters et Doxygen verts.

## Exigences
`EX-GP-027`, `EX-GP-028`, `EX-GP-029`, `EX-ARCH-011`.
