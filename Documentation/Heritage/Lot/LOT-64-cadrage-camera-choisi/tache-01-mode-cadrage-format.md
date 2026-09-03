# TACHE-01 — Mode de cadrage dans le format de niveau {#lot-64-tache-01-mode-cadrage-format}

**Lot :** [LOT-64](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
Le cadrage n'existe nulle part dans le modèle de niveau. Il est **déduit** à l'exécution, dans
`HMI`, d'une comparaison entre les dimensions du niveau et deux constantes de compilation
(`hmi::RoomGrid::ROOM_WIDTH_TILES` = 24, `ROOM_HEIGHT_TILES` = 14). Le designer n'a aucun moyen de
l'exprimer, et le moteur aucun moyen de le lire.

Cette tâche en fait une **donnée**, avec la contrainte qui prime sur toutes les autres : un niveau
existant, qui ne déclare rien, doit se comporter **exactement** comme aujourd'hui.

## Travail à réaliser
- **Champ de cadrage** dans le modèle de niveau (`core::Level`) : mode parmi *niveau entier*,
  *par salle*, *suivi*, plus les paramètres propres au mode retenu (taille de salle pour *par
  salle*).
- **Sérialisation** dans `core::LevelLoader` et `core::LevelWriter`, avec incrément de la **version
  de format** (`EX-LVL-005`).
- **Repli explicite** : champ absent → le mode qui **reproduit** la règle actuelle, c'est-à-dire
  *niveau entier* si le niveau tient dans une salle, *par salle* sinon. Le repli est calculé à un
  seul endroit, nommé, et testé — pas dispersé dans les appelants.
- **Validation** (`EX-LVL-004`) : mode inconnu, taille de salle nulle ou supérieure au niveau,
  paramètre incohérent avec le mode → erreur **exploitable**, nommant le champ fautif.
- **Taille de salle par niveau** : les constantes de `hmi::RoomGrid` deviennent des **valeurs par
  défaut**, pas la seule vérité. `hmi::RoomGrid` reçoit la taille au lieu de la connaître.
- **Le cadrage vit dans `Core`** parce que c'est une donnée de niveau ; son **application** reste
  dans `HMI` — même partage que le reste du format.

## Fichiers impactés
- `Source/Core/Levels/Level.h`, `LevelLoader.cpp`, `LevelWriter.cpp`, `LevelDraft.{h,cpp}`.
- `Source/Core/Levels/CameraFraming.{h,cpp}` (nouveau) — énumération, paramètres, règle de repli.
- `Source/HMI/Graphics/RoomGrid.{h,cpp}` — taille reçue plutôt que constante.
- `Source/Test/Unit/Core/Levels/test_level_loader.cpp`, `test_level_writer.cpp`,
  `test_level_draft.cpp`, `Source/Test/Unit/HMI/Graphics/test_room_grid.cpp` (étendus).
- `Source/Test/Unit/Core/Levels/test_camera_framing.cpp` (nouveau).

## Tests (obligatoires)
- **Compatibilité — le test central** : chacun des quinze niveaux livrés, chargé **sans** champ de
  cadrage, donne le mode et les paramètres qui reproduisent son comportement actuel.
- Aller-retour de sérialisation : un niveau portant chacun des trois modes s'écrit, se relit, et
  donne exactement le même modèle.
- Validation : mode inconnu, taille de salle nulle, taille supérieure au niveau, paramètre étranger
  au mode — chacun refusé avec une erreur nommant le champ.
- Une taille de salle personnalisée est bien celle qu'utilise `hmi::RoomGrid` ; la partition reste
  correcte aux bords.
- Tests `Core` purs, sans GPU.

## Points d'attention
- **La règle de repli est le cœur de la tâche.** L'écrire à un seul endroit, nommé, et la tester
  contre les niveaux réellement livrés : c'est la seule façon de garantir qu'aucun tableau ne
  change de comportement.
- **Version de format** : `EX-LVL-005` existe pour ça, et l'éditeur doit continuer de lire un
  fichier d'une version antérieure.
- `core::LevelDraft` (modèle d'édition) doit porter le champ, sinon le round-trip de l'éditeur
  l'efface silencieusement au premier enregistrement — le défaut classique des nouveaux champs.
- Rendre la taille de salle variable touche `hmi::RoomGrid`, utilisé par la caméra **et** par le
  repère visuel de salles dans l'éditeur : les deux doivent recevoir la même valeur.
- Ne pas introduire de dépendance de `Core` vers `HMI` : `hmi::RoomGrid` reste dans `HMI` et reçoit
  la donnée, `Core` ne l'appelle pas.

## Définition de fait (DoD)
- Le mode de cadrage est une donnée du niveau, sérialisée, versionnée et validée ; la règle de repli
  reproduit à l'identique le comportement des niveaux existants, prouvé sur les quinze tableaux
  livrés ; la taille de salle est réglable ; tests `Core` purs ; `/W4 /WX` propre.

## Exigences
`EX-LVL-006` (le niveau porte son mode de cadrage) ; réutilise `EX-LVL-004` (validation),
`EX-LVL-005` (version de format), `EX-REN-015` (cadrage par salle), `EX-NFR-040` (erreur
récupérable), `EX-NFR-010` (`Core` testable sans GPU), `EX-NFR-020` (tests unitaires).
