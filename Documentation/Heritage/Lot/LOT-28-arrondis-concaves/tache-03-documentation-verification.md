# TACHE-03 — Documentation et vérification {#lot-28-tache-03-documentation-verification}

**Lot :** [LOT-28](epic.md) · **Emplacement :** `Documentation` · **Statut :** ✅

## Contexte
Dernière tâche du lot : aligner la documentation sur le comportement livré par TACHE-01/TACHE-02.

## Travail à réaliser
- **`Documentation/Specification/gameplay.md`** : nouvelle exigence `EX-GP-007` (section 1, « Monde
  en tuiles », à la suite d'`EX-GP-006`), et mise à jour du tableau récapitulatif des types de tuile
  s'il en existe une ligne par famille.
- **`Documentation/Specification/niveaux.md`** : les quatre nouveaux types JSON ajoutés au format
  de niveau documenté.
- **`Source/Elements/Levels/README.md`** : les quatre nouveaux types listés.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-28`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant le lot (formule concave, sous-groupe de
  palette, aucune régression).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`, `niveaux.md`.
- `Source/Elements/Levels/README.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.

## Points d'attention
- **Écart de cadrage constaté** (voir TACHE-02) : l'épic évaluait à **sept** le nombre de cases
  libres dans la grille d'atlas après l'agrandissement `LOT-26`, mais l'une d'elles (`(4,4)`) est en
  réalité réservée au damier de transparence (`TextureAtlas::transparentTileIndex`) — il n'y en
  avait donc réellement que **six**. `ConcaveDownLeft`, initialement assignée à `(4,4)`, affichait
  le damier au lieu de sa silhouette ; corrigé en la réassignant à `(2,4)`, authentiquement libre.
  Épic et TACHE-02 mis à jour en conséquence.
- **Écart de cadrage majeur constaté** (voir TACHE-01, rapporté lors d'essais manuels en jeu par le
  demandeur) : `core::resolveSlopeFollow`/`core::resolveCeilingSlopeFollow` — que l'épic prévoyait
  explicitement de laisser inchangées — sélectionnaient la case à consulter uniquement par le
  **centre** de la boîte du personnage, un défaut **antérieur à ce lot** (reproduit sur
  `SlopeDownRight`, `LOT-26`) mais jamais exposé faute d'avoir déjà chaîné deux tuiles non solides
  adjacentes. Corrigé en élargissant la sélection aux colonnes réellement couvertes par la boîte,
  sans changer le comportement de la colonne centrale.
- **Second écart, plus profond, constaté juste après le premier** (rapporté par le demandeur : le
  personnage passait toujours au travers en sautant sous un plafond, y compris après le premier
  correctif, puis confirmé sur les pentes linéaires) : marcher **pendant** un saut pouvait faire
  sortir la boîte d'une colonne pertinente sur plusieurs pas consécutifs, avant que le seuil
  vertical de blocage n'y soit atteint — la mémoire d'un seul pas précédent (premier correctif) ne
  suffisait pas toujours. Corrigé en faisant mémoriser à `core::CharacterPhysicsSystem` l'étendue
  horizontale couverte depuis le **début de la montée courante** (`Player::ascentSweepMinX/MaxX`),
  pas seulement le pas précédent. Vérifié généralisé aux pentes linéaires de plafond (`LOT-26`), pas
  spécifique au concave. Épic et TACHE-01 mis à jour en conséquence ; zéro régression sur la suite
  complète après les deux correctifs.
- **Troisième écart, distinct des deux précédents, constaté juste après leur correction** (voir
  TACHE-01, rapporté par le demandeur : « toujours le problème sur le bord fin du bloc ») : un saut
  bloqué tout près du bord **fin** (silhouette quasi vide) d'un arrondi concave de plafond se
  retrouvait, un pas après le blocage, téléporté au-dessus du plafond — `core::resolveSlopeFollow`
  interprétait à tort le chevauchement résiduel (bord bas du personnage encore dans la case de
  plafond après un blocage par en dessous) comme un atterrissage sur la face du haut de la tuile.
  Corrigé en exigeant que le bord bas ait déjà été au-dessus de la case avant le pas pour qu'un tel
  calage soit accepté. Diagnostiqué à l'aide d'une nouvelle infrastructure de journalisation
  réutilisable (`Source/Core/Physics/PhysicsLog.h`, macros `PHYSICS_LOG_*`, réservées aux
  événements rares) plutôt qu'une instrumentation temporaire. Épic et TACHE-01 mis à jour en
  conséquence ; zéro régression sur la suite complète après les trois correctifs.
- **Écart constaté en cours de lot** (voir TACHE-02) : `slopeShapePixel` (`TextureAtlas.cpp`)
  échantillonnait au centre de chaque pixel, ne touchant jamais exactement les bords `0`/`1` de la
  case — sans conséquence pour les pentes/arrondis existants, mais laissant une encoche visible
  près du bord **plein** (tangente raide) d'un arrondi concave. Corrigé en échantillonnant au bord
  des pixels de coin. TACHE-02 mise à jour en conséquence.
- **Vérifié en jeu (essai immédiat, `P`, depuis l'éditeur)** après les deux correctifs ci-dessus :
  deux arrondis concaves de sol adjacents (`ConcaveUpRight`/`ConcaveUpLeft`, formant un pic/une
  arche) se traversent en marchant sans chute, silhouette rendue comme un pic net sans encoche —
  reproduit fidèlement le scénario rapporté par le demandeur (capture d'écran à l'appui).

## Définition de fait (DoD)
- Documentation cohérente avec le comportement livré ; `epic.md` marqué **terminé**, chaque tâche
  marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
