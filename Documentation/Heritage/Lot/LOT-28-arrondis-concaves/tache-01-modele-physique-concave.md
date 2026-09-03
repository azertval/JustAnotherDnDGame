# TACHE-01 — Modèle de tuile et formule de courbe concave {#lot-28-tache-01-modele-physique-concave}

**Lot :** [LOT-28](epic.md) · **Emplacement :** `Core/Levels`, `Core/Physics` · **Statut :** ✅

## Contexte
Ajoute les quatre nouveaux types de tuile et leur formule de courbe, en réutilisant intégralement
l'infrastructure de suivi de surface/silhouette posée par `LOT-22`/`LOT-23` (sol) et `LOT-26`
(plafond) — aucune nouvelle passe de résolution, seulement une nouvelle formule et deux nouvelles
entrées dans un mapping existant.

## Travail à réaliser
- **`Core/Levels/TileType.h`** : quatre nouveaux énumérateurs, `ConcaveUpRight`/`ConcaveUpLeft`/
  `ConcaveDownRight`/`ConcaveDownLeft` ; `core::isSolid` **ne** les inclut **pas** (comme les
  arrondis convexes — leur collision est résolue par suivi, pas par la grille classique).
- **`Core/Physics/SlopeGeometry.h`/`.cpp`** :
  - `core::isFollowableSurface` reconnaît `ConcaveUpRight`/`ConcaveUpLeft` (variantes de sol).
  - `core::isCeilingSlope` reconnaît `ConcaveDownRight`/`ConcaveDownLeft` (variantes de plafond).
  - `core::slopeSurfaceHeight` gagne deux nouveaux `case` (formule concave, voir décision de
    cadrage de l'épic) pour les variantes de sol, et deux de plus (hauteur constante `0.0f`, face
    du haut plate) pour les variantes de plafond — même traitement que `SlopeDownRight`/etc.
  - `core::ceilingSlopeHeight` : le mapping interne `floorMirror` gagne `ConcaveDownRight` →
    `ConcaveUpRight` et `ConcaveDownLeft` → `ConcaveUpLeft`. **Aucune** autre modification de cette
    fonction (déjà générique depuis `LOT-26` : `1 - slopeSurfaceHeight(mirror, localX)`).
  - `core::resolveSlopeFollow`/`core::resolveCeilingSlopeFollow` : **modifiées en cours de lot**,
    contrairement au plan initial — voir « Écart constaté » ci-dessous. Ne connaissent toujours
    aucun type de tuile explicitement (seulement via `slopeSurfaceHeight`/`ceilingSlopeHeight` et
    `isFollowableSurface`/`isCeilingSlope`) ; le changement porte uniquement sur la **sélection de
    colonne**, pas sur les formules elles-mêmes.
- **`Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`** : mapping JSON ↔ enum (`concaveUpRight`,
  `concaveUpLeft`, `concaveDownRight`, `concaveDownLeft`).

## Fichiers impactés
- `Source/Core/Levels/TileType.h`.
- `Source/Core/Physics/SlopeGeometry.h`/`.cpp`.
- `Source/Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp` (écart de cadrage — voir ci-dessous).
- `Source/Core/Ecs/Components/Player.h` (écart de cadrage — nouveaux champs
  `ascentSweepMinX`/`ascentSweepMaxX`).
- `Source/Core/Physics/PhysicsLog.h` (nouveau fichier, écart de cadrage — macros de journalisation
  `PHYSICS_LOG_*` réutilisables, voir « Points d'attention »).
- Tests : `Source/Test/Unit/Core/Physics/test_slope_geometry.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_loader.cpp`/`test_level_writer.cpp`,
  `Source/Test/Integration/test_physique_personnage.cpp`.

## Tests (obligatoires)
- `isSolid` renvoie `false` pour les quatre nouveaux types ; `isFollowableSurface` reconnaît
  exactement `ConcaveUpRight`/`ConcaveUpLeft` (pas les variantes de plafond) ; `isCeilingSlope`
  reconnaît exactement `ConcaveDownRight`/`ConcaveDownLeft`.
- **Formule concave vérifiée aux bords et au centre**, valeurs calculées à la main (comme pour
  `RoundedUpRight`/`RoundedUpLeft`, `LOT-23`) : `ConcaveUpRight` vaut `1` à `x=0`, `0` à `x=1`
  (mêmes bords que `RoundedUpRight`, tangente horizontale/verticale respectivement — comparer avec
  `1 - sqrt(0.75) ≈ 0,134` de l'arrondi convexe au centre, pour confirmer une courbe bien
  **distincte**, pas une régression déguisée de la même formule).
- `ceilingSlopeHeight(ConcaveDownRight/Left, x)` est le miroir vertical exact de
  `slopeSurfaceHeight(ConcaveUpRight/Left, x)` pour quelques points (bords + centre).
- Aller-retour JSON (chargement puis écriture) pour les quatre nouveaux types.
- Physique de bout en bout : un personnage suit `ConcaveUpRight`/`ConcaveUpLeft` en marchant (sol) ;
  un saut est bloqué par la silhouette de `ConcaveDownRight`/`ConcaveDownLeft` par en dessous, et un
  personnage qui tombe dessus par au-dessus s'y pose normalement (plafond).
- **Suite de régression complète** : tous les tests physique existants (sol, pentes/arrondis
  convexes de sol et de plafond, murs, sauts, blocs poussables) restent verts — trois d'entre eux
  (`ChuteRapideSurUnePenteSansLaTraverser`, `SuitUnePenteAscendanteEnMarchant`,
  `SuitUnePenteDescendanteEnMarchant`) ont dû être ajustés suite aux deux correctifs décrits
  ci-dessous (fenêtre d'échantillonnage élargie pour les deux derniers, aucun changement de
  comportement attendu autre que le calage exact/le moment de stabilisation reproduits).
- **Deux arrondis concaves de sol adjacents** (jointure « pleine » commune, comme un pic/une voûte)
  restent praticables en marchant sans chute jusqu'au fond de la case voisine à la jointure
  (`ArrondisConcavesDeSolAdjacentsSansChuteALaJointure`) ; un arrondi concave de plafond bloque
  toujours un saut visant son bord plein même tout près du bord de sa propre case, sans case solide
  voisine pour compenser (`ConcaveDePlafondBloqueMemePresDuBordDeSaPropreCase`) — régression du
  premier bug de sélection de colonne décrit ci-dessous, reproduite avant correctif sur
  `SlopeDownRight` (`LOT-26`) pour confirmer qu'il ne s'agissait pas d'un défaut de la formule
  concave elle-même.
- **Sauter tout en marchant** (les deux mouvements combinés) vers le bord épais d'un arrondi
  concave de plafond, ou d'une pente linéaire de plafond, bloque toujours le saut
  (`ConcaveDePlafondBloqueMemeEnMarchantPendantLeSaut`, couvre `ConcaveDownRight`/`ConcaveDownLeft`
  **et** `SlopeDownRight`/`SlopeDownLeft`) — régression du second bug (mémoire d'ascension),
  reproduite sur les quatre types pour confirmer un défaut de mécanique générale, pas propre au
  concave.
- **Un saut bloqué près du bord fin** (silhouette quasi vide) d'un arrondi concave de plafond reste
  bloqué durablement, sans téléportation au-dessus du plafond au pas suivant
  (`ConcaveDePlafondBordFinResteBloqueSansTeleportation`) — régression du troisième bug décrit
  ci-dessous, vérifiée à CHAQUE pas du saut (pas seulement au minimum global), le passage au travers
  d'origine ne se manifestant qu'un pas après le blocage.

## Points d'attention
- **Écart de cadrage majeur, constaté après coup-sur-coup lors d'essais manuels en jeu** (rapporté
  par le demandeur : « les blocs concaves ne vont pas jusqu'au bout d'un bloc et si on saute on
  peut passer au travers par le bas ») : `core::resolveSlopeFollow`/
  `core::resolveCeilingSlopeFollow` sélectionnaient la case à consulter uniquement par le **centre**
  de la boîte du personnage (`floor(centerX)`), en ignorant sa **largeur**. Or la boîte peut
  chevaucher la frontière entre deux cases sans que son centre ait encore basculé — et pour toute la
  **dernière fraction** (environ la moitié de la largeur de la boîte) de la largeur d'une case
  arrondie/concave, le centre atterrit déjà dans la case **voisine**. Si cette case voisine est
  vide (silhouette isolée) ou une autre tuile non solide (deux arrondis posés côte à côte, comme un
  arc/une voûte — l'usage même que cet épic vise), aucune silhouette n'y est trouvée : le
  personnage tombe/saute au travers, sans aucun filet (contrairement à une case voisine solide,
  où la collision classique sur grille — `core::isSolid` — masquait silencieusement le même défaut
  depuis `LOT-22`/`LOT-26`). Confirmé en reproduisant le bug sur `SlopeDownRight` (pente linéaire de
  plafond, `LOT-26`) avec le même symptôme exact (saut totalement non bloqué au-delà de `localX ≈
  0,8` avec la boîte réelle du personnage, `0,4` de large) : **le défaut est antérieur à ce lot**,
  simplement jamais exposé faute d'avoir déjà placé deux tuiles non solides directement adjacentes.
  Corrigé en élargissant la sélection de colonne à celles réellement couvertes par la boîte : la
  colonne **centrale** garde exactement son calcul d'origine (aucune régression pour tout appelant
  dont la boîte ne dépasse pas d'une seule colonne pertinente) ; toute colonne **supplémentaire**
  chevauchée par un bord de la boîte est également consultée, avec le `localX` de **ce bord-là**
  (pas du centre) — et parmi toutes les colonnes couvertes, la surface la plus haute (sol) ou la
  silhouette la plus basse (plafond) l'emporte, comme un balayage classique. Deux tests de
  régression pré-existants ont d'abord cassé avec une première version plus large de ce correctif
  (échantillonnage au milieu du segment de chevauchement plutôt qu'au bord) — révélant que la boîte
  1×1 des tests plus anciens (`spawnPlayer`) chevauche en permanence deux colonnes par construction
  (largeur = largeur d'une case) ; la version au bord de boîte, ci-dessus, préserve exactement le
  comportement d'origine pour la colonne centrale et n'a plus cassé aucun test existant.
- **Deuxième écart, plus profond, constaté juste après le précédent** (rapporté par le demandeur :
  « le personnage passe toujours au travers en sautant par dessous le bloc plafond », confirmé
  ensuite sur les pentes linéaires aussi) : le correctif ci-dessus ne considère que la boîte du pas
  **précédent** et celle du pas courant — insuffisant quand **marcher tout en sautant** fait sortir
  la boîte d'une colonne pertinente **avant** que le seuil vertical de blocage n'y soit atteint,
  parfois sur PLUSIEURS pas consécutifs (le seuil peut être manqué de peu, pas après pas, pendant
  toute une montée lente combinée à une marche rapide). Reproduit via un test simulant un saut
  **et** une marche simultanés (pas seulement un saut immobile) : la case qui aurait dû bloquer
  redevenait invisible dès que la boîte la quittait horizontalement, y compris plusieurs pas après
  qu'elle ait cessé d'être couverte. Corrigé en faisant mémoriser par `core::CharacterPhysicsSystem`
  l'étendue horizontale couverte par la boîte depuis le **début de la montée courante** (nouveaux
  champs `Player::ascentSweepMinX`/`ascentSweepMaxX`, remis à l'étendue courante à chaque pas où le
  personnage est au sol ou ne monte pas), plutôt que la seule boîte du pas précédent —
  `core::resolveCeilingSlopeFollow` reçoit désormais cette étendue accumulée en paramètres explicites
  (`sweptMinX`/`sweptMaxX`) plutôt qu'une boîte précédente. Vérifié généralisé aux pentes linéaires
  de plafond (`EX-GP-006`, `LOT-26`), pas seulement aux arrondis concaves de ce lot : le correctif ne
  connaît aucun type de tuile, seulement la géométrie de la boîte.
- **Troisième écart, distinct des deux précédents, constaté juste après leur correction** (rapporté
  par le demandeur : « toujours le problème sur le bord fin du bloc ») : un saut bloqué tout près du
  bord **fin** (silhouette quasi vide) d'un arrondi concave de plafond se retrouvait, au pas SUIVANT
  le blocage, téléporté tout AU-DESSUS du plafond — un passage au travers complet, mais retardé d'un
  pas et donc non détecté par les deux tests de régression précédents (qui ne vérifient que le
  minimum atteint pendant l'ascension, pas l'état juste après le blocage). Cause : près du bord fin,
  le blocage a lieu tout près du sommet de la case (silhouette peu épaisse) — le bord bas du
  personnage reste alors, du fait de sa propre hauteur, encore DANS la même case après le blocage ;
  au pas suivant, `core::resolveSlopeFollow` interprétait ce chevauchement résiduel comme un
  atterrissage sur la face du haut de la tuile de plafond (qui ne devrait porter un personnage que
  tombant dessus PAR AU-DESSUS), et calait donc la position du personnage au sommet de la case —
  au-dessus du plafond. Corrigé en exigeant que le bord bas ait déjà été AU-DESSUS (ou pile au
  sommet) de la case AVANT le pas pour qu'un calage sur face du haut de plafond soit accepté
  (garde-fou restreint aux tuiles `isCeilingSlope`, sans effet sur le calage normal d'un
  atterrissage sur sol plat classique). Diagnostiqué grâce à une nouvelle infrastructure de
  journalisation réutilisable (`Source/Core/Physics/PhysicsLog.h`, macros `PHYSICS_LOG_*`) plutôt
  qu'une instrumentation temporaire ponctuelle — conservée pour faciliter le suivi d'anomalies
  similaires à l'avenir, réservée aux événements rares (jamais au chemin exécuté à chaque pas fixe).
- **Écart constaté en cours de lot** (test) : le test d'intégration `SuitUnArrondiConcaveAscendantEnMarchant`
  utilisait initialement la boîte de test 1×1 (`spawnPlayer`, comme les autres tests de pente/arrondi
  de ce fichier), qui échoue sur ce cas précis — pas un bug de la formule concave elle-même, mais un
  effet de bord déjà connu du moteur de collision (documenté par
  `TransitionPenteSolPlatSansAACoup`) : une boîte large d'une case entière chevauche la colonne
  solide voisine (sol haut, après l'arrondi) exactement au point d'échantillonnage à mi-case
  (x=0,5), avant que le suivi de la courbe concave n'ait eu la chance de s'exprimer — imperceptible
  avec la formule convexe (déjà proche du palier haut à ce stade), mais spectaculaire avec la
  formule concave (censée rester proche du palier bas jusque tard). Corrigé en utilisant la taille
  RÉELLE du personnage (`core::playerSize()`, 0,4×0,8), comme `TransitionPenteSolPlatSansAACoup`.
- **Ne pas confondre les deux formules concave/convexe** : elles partagent les mêmes valeurs aux
  bords (`0`/`1`), seule la courbe entre les deux diffère — un test qui ne vérifie que les bords ne
  distinguerait pas un bug d'inversion de formule (concave collée à la place du convexe ou
  inversement) ; le point **central** (`x=0,5`) est le test qui les discrimine réellement :
  `RoundedUpRight` (convexe) y vaut `1 - sqrt(0,75) ≈ 0,134`, `ConcaveUpRight` (ce lot) y vaut
  `sqrt(0,75) ≈ 0,866` — les deux valeurs sont exactement complémentaires (somme `1`), mais très
  distinctes l'une de l'autre, ce qui rend un bug d'inversion facilement détectable par le test.

## Définition de fait (DoD)
- Formule et physique fonctionnelles et testées ; zéro régression sur la suite de tests physique
  existante ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-GP-007` (modèle et physique).
