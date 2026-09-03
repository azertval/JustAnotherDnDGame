# TACHE-01 — Route multi-points dans le modele, le format et le brouillon {#lot-67-tache-01-route-multi-points}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
`core::MovingPlatformConfig` ne portait que `startPosition` et `endPosition`, et sa documentation
actait explicitement l'absence de chemin multi-points. Un parcours en L ou un circuit ferme n'etait
donc pas exprimable — ni dans le fichier, ni dans le brouillon d'edition.

`core::LevelDraft::setPlatformConfig` existait deja, annulable et teste, mais **sans aucun
appelant** dans `Source/HMI` : sa granularite (remplacer la configuration entiere) convenait mal au
geste d'edition a venir, qui manipule un point a la fois.

## Travail a realiser
- Remplacer `endPosition` par `waypoints` (les points **apres** le depart) et ajouter
  `PlatformPathMode` (`PingPong` par defaut, `Loop`). Supprimer `endPosition` plutot que le
  deprecier : le compilateur recense alors tous les appelants d'un coup.
- **Lecture** (`LevelLoader`) : accepter `waypoints`, et a defaut retomber sur `endX`/`endY`
  converti en point unique. Valider **chaque** point contre les bornes (`OutOfBounds`), pas
  seulement le premier. Un `mode` inconnu retombe silencieusement sur l'aller-retour, comme l'axe
  d'un danger mobile.
- **Ecriture** (`LevelWriter`) : emettre `waypoints`, omettre `mode` a sa valeur par defaut, et ne
  plus jamais produire `endX`/`endY`.
- **Brouillon** : elargir `setPlatformConfig`, et ajouter les mutateurs granulaires que le geste
  d'edition consommera (`addPlatformWaypoint`, `insertPlatformWaypoint`, `movePlatformWaypoint`,
  `removePlatformWaypoint`, `setPlatformMode`/`setPlatformSpeed`/`setPlatformPhase`). Tous passent
  par un point d'entree commun qui empile **un seul** `pushUndo()`, y compris quand il cree la
  configuration au passage.
- Au redimensionnement, une route dont **un** point sort du niveau fait partir la configuration
  entiere : amputer la route donnerait un parcours silencieusement different.

## Fichiers impactes
`Source/Core/Levels/Level.h`, `LevelLoader.cpp`, `LevelWriter.{h,cpp}`, `LevelDraft.{h,cpp}`.

## Tests (obligatoires)
- `test_level_loader.cpp` : route multi-points lue avec son mode ; repli `endX`/`endY` ; mode
  inconnu tolere ; point de parcours hors bornes rejete (`OutOfBounds`) **au-dela du premier**.
- `test_level_writer.cpp` : aller-retour route + mode ; mode par defaut omis ; plateforme sans route
  n'ecrivant aucun champ de route ; ancien format reecrit en `waypoints`.
- `test_level_draft.cpp` : construction d'un parcours point par point, chaque geste annulable en un
  pas ; rang hors de la route sans effet ; mode, vitesse et dephasage independants.

## Points d'attention
La suppression de `endPosition` est une **rupture assumee** : cinq sites de test la referencent,
aucun code de production. Les faire remonter par le compilateur vaut mieux qu'une depreciation
silencieuse qui laisserait les deux champs coexister.

## Definition de fait (DoD)
Le format lit les deux ecritures, n'en produit qu'une, et tous les niveaux livres se chargent sans
erreur. `ctest` a 100 %.

## Exigences
`EX-GP-054`, `EX-LVL-008`, `EX-GP-026`, `EX-NFR-040`, `EX-EDIT-005`.
