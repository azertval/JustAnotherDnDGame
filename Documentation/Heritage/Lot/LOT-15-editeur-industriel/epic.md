# LOT-15 — Éditeur de niveaux : robustesse et confort d'édition {#lot-15}

> Statut : **terminé**. Ce lot fait évoluer l'éditeur intégré livré en LOT-14 (peinture,
> mécanismes, redimensionnement, undo/redo, enregistrement, essai immédiat) vers un outil plus
> proche d'un éditeur de niveau de production : protection contre la perte de travail, confort
> d'édition sur des niveaux plus grands, découvrabilité des commandes, et réduction de la dette
> technique laissée par LOT-14.

## Objectif
- **Nommer/renommer** un niveau et être **averti avant d'écraser** un fichier existant
  (`EX-EDIT-009`).
- Être **protégé** contre la perte de travail : confirmation avant un redimensionnement
  destructeur, avertissement avant de quitter avec des modifications non enregistrées
  (`EX-EDIT-012`).
- **Naviguer confortablement** dans la grille : déplacer (pan) et zoomer la caméra indépendamment
  du cadrage automatique existant (`EX-EDIT-013`).
- Éditer plus vite qu'à la case : **remplissage rectangulaire** et **sélection avec copier/coller**
  (`EX-EDIT-014`).
- **Découvrir les commandes** sans quitter l'éditeur : barre d'outils, aperçu des raccourcis,
  libellés de palette (`EX-EDIT-015`), et liaisons interrupteur↔porte **lisibles** même
  nombreuses (`EX-EDIT-016`).
- Éliminer deux points de dette technique identifiés en LOT-14 : l'essai immédiat passe par un
  fichier temporaire partagé plutôt qu'un niveau en mémoire, et les messages d'erreur de
  validation sont traduits par recherche de sous-chaînes fragiles.

Comme pour LOT-14, toute la logique nouvelle sans rendu (mutation par lot, requête de perte de
données, saisie de texte) reste dans `Core`/logique pure, testable sans GPU (`EX-NFR-010`) ;
`HMI/Editor` et `HMI/Interface` n'orchestrent que l'interaction et l'affichage.

## Périmètre

### Inclus
- **Entrées bas niveau** : molette (zoom) et texte tapé (saisie de nom), absents de `InputState`
  aujourd'hui.
- **Nommage** : prompt de nom à la création d'un niveau vierge, renommage en cours d'édition,
  avertissement avant d'écraser un fichier existant différent du niveau chargé.
- **Garde-fous de perte de données** : confirmation avant un redimensionnement qui supprimerait
  l'entrée, la sortie ou une liaison ; confirmation avant de quitter l'éditeur avec des
  modifications non enregistrées.
- **Caméra** : pan (glisser bouton droit) et zoom (molette) manuels, réinitialisables au cadrage
  automatique existant.
- **Outils de zone** : remplissage rectangulaire, sélection + copier/coller d'un bloc de tuiles.
- **Découvrabilité** : barre d'outils (sélection d'outil), aide des raccourcis (bascule), libellés
  texte sur la palette, une teinte distincte par interrupteur pour les liaisons.
- **Essai immédiat en mémoire** : le `GameScreen` intégré accepte un `core::Level` déjà construit,
  sans passer par un fichier temporaire partagé.
- **Erreurs de validation structurées** : `LevelLoadResult` porte un code d'erreur énuméré en plus
  du message technique, pour que l'éditeur traduise sans dépendre du texte exact du message.
- **Nettoyage documentaire** : suppression de `Source/Core/Level/README.md` (format abandonné,
  jamais implémenté), mise à jour de `Source/HMI/Editor/README.md` (périmètre réellement livré).

### Exclus (lots ultérieurs ou non retenus)
- **Décors & pipeline photo → pixel art** (`EX-DEC-*`, `EX-EDIT-040/041`) — dépend d'un lot dédié,
  inchangé depuis LOT-14.
- **Clé ↔ porte verrouillée, blocs poussables** dans la palette — dépend de leur implémentation
  côté `Core`/gameplay, toujours absente.
- **Sélection multiple non contiguë** et **historique undo/redo par delta** — l'historique par
  snapshots complets (décision LOT-14) reste adapté à la taille des niveaux du projet ; non
  reconsidéré ici.
- **Édition collaborative en temps réel** — non-objectif de la spec.
- **Palette pilotée par un fichier de configuration externe** — la liste de types gérés par `Core`
  reste petite et change rarement ; ajouter une couche de données externes serait une abstraction
  non justifiée à ce stade.

## Décisions de cadrage
- **Une seule méthode `Core` pour toute mutation par lot** : `LevelDraft::paintRegion(originColumn,
  originRow, block)` sert à la fois au remplissage rectangulaire et au collage — elle réutilise la
  sémantique cellule-par-cellule déjà validée de `paintTile` (déplacement entrée/sortie, nettoyage
  des liaisons), mais ne pousse **qu'un seul** snapshot undo pour toute l'opération
  (`paintTile` devient un appel à `paintRegion` avec un bloc 1×1, `EX-EDIT-010` : aucune règle
  dupliquée).
- **Le copier ne touche pas `Core`** : `EditorScreen` lit directement `LevelDraft::tileMap()` (déjà
  public) pour construire le presse-papiers ; seul le **coller** repasse par `paintRegion`.
- **Confirmation de redimensionnement** : nouvelle requête pure et non mutante
  `LevelDraft::wouldResizeDropContent(width, height) const`, interrogée par `HMI` avant d'appeler
  `resize` — cohérent avec le principe déjà acté en LOT-14 TACHE-03 (« l'avertissement de perte
  revient à l'appelant `HMI` »), qui n'avait alors pas été câblé jusqu'au bout.
- **Essai immédiat en mémoire** : nouveau constructeur `GameScreen(batch, atlas, w, h, core::Level)`
  ; la construction de scène ECS, aujourd'hui dans `loadLevel(path)`, est factorisée en un
  `loadLevel(core::Level)` privé partagé par les deux constructeurs. Supprime le fichier temporaire
  partagé `projectgaming_playtest_level.json` (risque de collision entre instances, aller-retour
  JSON inutile) identifié en LOT-14 TACHE-05.
- **Erreurs structurées sans casser l'existant** : `LevelLoadResult` gagne un champ
  `LevelValidationError` (enum) **en plus** du `error` (message technique) déjà utilisé par les
  tests et les journaux — purement additif, aucune signature existante modifiée.
- **Entrées bas niveau additives** : molette (`WM_MOUSEWHEEL`) et texte tapé (`WM_CHAR`) s'ajoutent
  à `InputState`/`Window` sur le même modèle que l'existant (`InputState` reste indépendant de
  toute fenêtre, testable sans elle, `EX-NFR-010`).
- **Aucun conflit de raccourcis** : le pan/zoom caméra passe par la **souris** (glisser bouton
  droit, molette) — le clic gauche (peinture/palette/liaison) et les flèches (redimensionnement,
  LOT-14) restent inchangés. Le changement d'outil (TACHE-05) réutilise `Key::Tab` (déclarée,
  inutilisée dans l'éditeur) plutôt que d'introduire des touches numériques ; seules cinq touches
  sont réellement nouvelles dans `Key` (`F1`, `F2`, `D0`, `C`, `V`, posées en TACHE-01).
- **Nom de niveau validé au moment de la saisie**, pas seulement à l'enregistrement : une liste
  noire minimale des caractères interdits par le système de fichiers Windows évite un échec
  d'écriture tardif et incompréhensible pour un non-codeur (cf. TACHE-03).
- **Palette et barre d'outils restent des classes de géométrie pure** (comme `TilePalette` en
  LOT-14) : aucune dépendance de rendu, testables sans GPU.
- **Ajustements post-livraison, à l'essai interactif** : la palette/barre d'outils, prévues en
  bandes horizontales empilées, se recouvraient entre elles et parfois avec la grille — remplacées
  par un **panneau latéral** vertical fixe (`EditorLayout.h`, TACHE-06). Une **grille de repère**
  (bascule `F10`) a été ajoutée, absente du périmètre initial. Les bornes de zoom (TACHE-04) ont été
  resserrées (cadrage automatique du niveau ↔ 4 cases visibles) après un premier passage sans lien
  avec la taille du niveau. Détail dans les fiches TACHE-01/04/06 concernées.

## Exigences couvertes
- Nouvelles : `EX-EDIT-009`, `EX-EDIT-012` à `EX-EDIT-016`.
- Réutilisées (approfondies sans changer leur sens) : `EX-EDIT-003` (précisée), `EX-EDIT-005`
  (redimensionnement), `EX-EDIT-007`/`008` (validation, essai immédiat), `EX-EDIT-010`/`011`
  (réutilisation/round-trip), `EX-NFR-010` (testabilité sans GPU), `EX-NFR-040` (erreurs
  récupérables).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-entrees-molette-texte.md) | Entrées bas niveau : molette et texte tapé | `HMI/Input`, `HMI/Platform` | ✅ |
| [TACHE-02](tache-02-garde-fous-perte-donnees.md) | Garde-fous : redimensionnement destructeur, quitter sans enregistrer | `Core/Levels`, `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-nommage-renommage.md) | Nommage, renommage, avertissement d'écrasement | `HMI/Editor`, `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-camera-pan-zoom.md) | Caméra : pan et zoom manuels | `HMI/Interface` | ✅ |
| [TACHE-05](tache-05-outils-rectangle-selection.md) | Outils de zone : remplissage rectangulaire, sélection, copier/coller | `Core/Levels`, `HMI/Editor`, `HMI/Interface` | ✅ |
| [TACHE-06](tache-06-decouvrabilite.md) | Découvrabilité : barre d'outils, aide, libellés, liaisons lisibles | `HMI/Editor`, `HMI/Interface` | ✅ |
| [TACHE-07](tache-07-essai-memoire-erreurs-structurees.md) | Essai immédiat en mémoire, erreurs de validation structurées | `Core/Levels`, `HMI/Interface` | ✅ |
| [TACHE-08](tache-08-nettoyage-documentation.md) | Nettoyage documentaire (READMEs, spec, CHANGELOG) | `Documentation`, `Source/Core/Level`, `Source/HMI/Editor` | ✅ |

## Critères d'acceptation du lot
1. Créer un niveau demande un **nom** ; le renommer est possible en cours d'édition ; enregistrer
   sous un nom qui écraserait un **autre** fichier existant est **confirmé** explicitement.
2. Un redimensionnement qui supprimerait l'entrée, la sortie ou une liaison est **signalé** avant
   application et peut être **refusé** ; quitter l'éditeur avec des modifications non enregistrées
   est également confirmé.
3. La vue peut être **déplacée** et **zoomée** indépendamment du cadrage automatique, et
   réinitialisée en un geste.
4. Un rectangle de tuiles peut être **rempli** en un geste et une zone peut être **copiée-collée**,
   chaque opération ne créant **qu'une seule** entrée d'annuler/refaire.
5. Toutes les commandes de l'éditeur sont **découvrables à l'écran** (barre d'outils, aide des
   raccourcis, libellés de palette) ; plusieurs liaisons interrupteur↔porte simultanées restent
   **distinguables** les unes des autres.
6. L'essai immédiat ne crée **plus aucun fichier temporaire** ; les messages d'erreur affichés à
   l'utilisateur ne dépendent plus d'une recherche de sous-chaîne dans un texte technique.
7. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `CHANGELOG.md` à jour, lint des exigences vert.

## Dépendances
- Réutilise l'éditeur intégré et le modèle mutable (LOT-14 : `LevelDraft`, `LevelWriter`,
  `EditorScreen`, `TilePalette`, `LevelPicker`), le rendu 2D et la caméra (LOT-05), les
  entrées/`InputState` (LOT-02/LOT-06), le `GameScreen`/`LevelSequence` (LOT-09).

## Navigation des tâches
- @subpage lot-15-tache-01-entrees-molette-texte
- @subpage lot-15-tache-02-garde-fous-perte-donnees
- @subpage lot-15-tache-03-nommage-renommage
- @subpage lot-15-tache-04-camera-pan-zoom
- @subpage lot-15-tache-05-outils-rectangle-selection
- @subpage lot-15-tache-06-decouvrabilite
- @subpage lot-15-tache-07-essai-memoire-erreurs-structurees
- @subpage lot-15-tache-08-nettoyage-documentation
