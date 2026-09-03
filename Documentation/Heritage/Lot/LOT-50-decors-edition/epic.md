# LOT-50 — Décors : placement et manipulation dans l'éditeur {#lot-50}

> Statut : **fait**. Prérequis : [LOT-49](@ref lot-49) (modèle et rendu des décors),
> [LOT-43](@ref lot-43) (bibliothèque d'assets).

## Objectif
Rendre les décors réellement utilisables par le level designer. LOT-49 pose le modèle, le rendu et
un placement minimal ; ce lot livre l'outillage d'édition demandé par `EX-DEC-010` et
`EX-EDIT-040` : placer, sélectionner, déplacer, redimensionner, changer de couche, réordonner et
supprimer.

C'est la différence entre « le format supporte les décors » et « on peut composer un décor ».

## Périmètre

### Inclus
- **Nouvelle valeur `hmi::EditorTool`** (ex. *DecorPlace*) et **geste pur** modelé sur
  `hmi::resolveLinkClick`/`LinkGesture` (LOT-37) : la machine à états du geste est une fonction
  testable sans Qt ni GPU, l'UI n'en est que le déclencheur.
- **Opérations** (`EX-DEC-010`) : placer depuis la bibliothèque, sélectionner par clic, déplacer par
  glisser, redimensionner par poignées, pivoter, changer de couche, **réordonner à l'intérieur d'une
  couche**, supprimer. Toutes passent par des mutateurs de `LevelDraft` et bénéficient donc de
  l'undo/redo existant.
- **Aimantation optionnelle** sur la grille, **désactivable** — un décor est libre par construction
  (`EX-DEC-001`), mais l'aimantation reste utile pour aligner une rangée de colonnes.
- **Sélection visuelle** : mise en évidence du décor sélectionné et de ses poignées, sur le calque
  d'aides d'édition (*EditorOverlay*), jamais en jeu.
- **Section « Décors » du panneau « Textures »** (LOT-42) : liste des décors du niveau courant,
  regroupés par couche, avec sélection croisée avec le canevas.

### Exclus (hors périmètre de ce lot)
- Manipulation par le joueur en cours de partie (`EX-DEC-020`/`EX-DEC-021`) : hors programme.
- Groupes de décors, motifs répétés, bibliothèque de compositions réutilisables.
- Conversion photo → pixel art (`EX-EDIT-041`) : hors programme.
- Alignement automatique entre décors, guides magnétiques.

## Décisions de cadrage
- **Geste pur, UI mince** : même parti que `LinkGesture` (LOT-37). La logique de sélection, de
  déplacement et de redimensionnement est une fonction déterministe sur des données, testable sans
  fenêtre — l'éditeur Qt ne fait que router les événements.
- **Réordonnancement explicite à l'intérieur d'une couche** : trois couches ne suffisent pas à
  ordonner dix décors qui se recouvrent. L'ordre dans le vecteur annexe fait foi ; l'exposer est
  moins coûteux que d'ajouter une profondeur continue.
- **Aimantation désactivable, jamais imposée** : imposer la grille annulerait l'intérêt même du
  décor libre.
- **Aucun nouveau mécanisme d'historique** : tout passe par `LevelDraft::State`, comme le reste.

## Exigences couvertes
- Concrétisées : `EX-DEC-010` (placer, déplacer, redimensionner, superposer, supprimer),
  `EX-EDIT-040` (édition de décors dans l'éditeur).
- Réutilisées : `EX-DEC-001`/`EX-DEC-002` (objet libre, couches), `EX-EDIT-010` (réutilisation du
  modèle `Core`), `EX-REN-033` (traduction), `EX-IHM-010` (fenêtre à panneaux).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-mutateurs-decor.md) | Mutateurs de décors sur `LevelDraft` (déplacer, redimensionner, pivoter, réordonner, changer de couche) | `Source/Core/Levels` | ✅ |
| [TACHE-02](tache-02-outil-geste.md) | Outil *DecorPlace* + geste pur de placement, sélection et manipulation | `Source/HMI/Editor` | ✅ |
| [TACHE-03](tache-03-selection-poignees.md) | Rendu de la sélection et des poignées sur le calque d'aides d'édition + aimantation | `Source/HMI/Graphics`, `Source/HMI/Editor` | ✅ |
| [TACHE-04](tache-04-section-decors.md) | Section « Décors » du panneau « Textures », sélection croisée avec le canevas | `Source/HMI/Editor` | ✅ |

## Critères d'acceptation du lot
1. Un décor peut être placé, sélectionné, déplacé, redimensionné, pivoté, changé de couche,
   réordonné et supprimé depuis l'éditeur.
2. Chacune de ces opérations est annulable et rétablissable.
3. L'aimantation sur la grille peut être activée et désactivée, et n'est jamais imposée.
4. Les aides visuelles de sélection n'apparaissent **jamais** en mode jeu ni en mode essai.
5. Le geste de manipulation est testé sans Qt ni GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-49](@ref lot-49) (modèle et rendu) et [LOT-43](@ref lot-43) (bibliothèque). Réutilise
`LinkGesture`/`LinkGeometry` (LOT-37) et l'historique de `LevelDraft` (LOT-14). Complète le
périmètre inspecté par [LOT-51](@ref lot-51).

## Navigation des tâches
- @subpage lot-50-tache-01-mutateurs-decor
- @subpage lot-50-tache-02-outil-geste
- @subpage lot-50-tache-03-selection-poignees
- @subpage lot-50-tache-04-section-decors
