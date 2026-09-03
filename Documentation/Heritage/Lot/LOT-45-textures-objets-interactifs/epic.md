# LOT-45 — Objet interactif : texture par instance {#lot-45}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41),
> [LOT-42](@ref lot-42), [LOT-44](@ref lot-44) (version du format de niveau).

## Objectif
Permettre d'assigner une texture propre à **une case précise** d'un niveau (ex. « cette porte-là »),
prioritaire sur le skin de son type (LOT-42). C'est la deuxième — et dernière — donnée de niveau
purement visuelle du programme côté grille ; toutes les autres restent globales (`skins.json`) ou
relèvent des décors (LOT-49).

## Périmètre

### Inclus
- **`Core`** : `struct TileTextureOverride { GridPosition position; std::string assetName; };`,
  vecteur annexe sur `Level`/`LevelDraft` — **exactement** le patron déjà utilisé par `Mechanism`/
  `DangerLink`/`DangerMoverConfig`/`DangerBlinkConfig` (`Source/Core/Levels/Level.h`).
- **JSON** : champ par tuile optionnel (ex. `"texture": "door_red.png"`), même précédent que
  `switch.id`/`door.opensWith` dans `LevelLoader.cpp`/`LevelWriter.cpp` — rétrocompatible, dans le
  format versionné installé en LOT-44.
- **Nettoyage** : `TileTextureOverride` rejoint le funnel **déjà existant**
  `LevelDraft::removeLinkedDataAt` (appelé par `paintTileInternal`, donc par `paintTile` **et**
  `paintRegion`) — un override est retiré quand sa case reçoit un autre type de tuile. Participe
  aussi à la troncature silencieuse de `resize` (même principe que les autres données annexes).
  **Ne voyage pas** avec un copier-coller (`paintRegion` utilisé pour le collage n'importe **pas**
  les overrides de la source) — décision explicite, un override reste attaché à sa case d'origine.
- **Undo/redo** : gratuit via `LevelDraft::State` (même mécanisme que les autres champs annexes).
- **Éditeur** : nouvelle valeur `hmi::EditorTool` (ex. *TextureAssign*) + geste pur d'assignation
  modelé sur `hmi::resolveLinkClick`/`LinkGesture` (LOT-37) — clic sur une case → sélecteur parmi
  `Assets/Objects/*.png` (vignettes de LOT-43) → assignation/suppression. Section « Objets » du
  panneau « Textures » (LOT-42) listant les overrides du niveau courant, pas un panneau séparé.
- **Résolution en mode Texture** : **override > skin du jeu courant (LOT-42) > damier magenta
  (LOT-40)**, priorité **assertée** via le *QuadRecorder*.
- Dossier `Assets/Objects/` + commande `POST_BUILD`.

### Exclus (hors périmètre de ce lot)
- **Animation** de l'asset désigné : un override choisit **quel asset** est affiché ; le choix du
  **clip** au sein de cet asset relève de LOT-46/47. Un asset animé assigné ici s'affichera comme
  image fixe jusqu'à LOT-46, sans que le format d'override change ensuite.
- Restriction du type de tuile éligible : **tout** type non-`Empty` peut recevoir un override (usage
  purement visuel, contrairement aux liens de mécanismes qui ont un rôle sémantique) — pas de liste
  blanche façon `isTriggerTile`/`isLinkTargetTile`.
- Réécriture des overrides lors d'un renommage d'asset : LOT-43 avertit, ne migre pas.

## Décisions de cadrage
- **Geste dédié, pas une extension de `LinkGesture`** : la machine à états de `LinkGesture` est
  façonnée pour l'appariement déclencheur/cible, pas l'assignation sur une seule case — un module
  parallèle (*TextureAssignGesture*) évite de coupler deux outils sans rapport.
- **Priorité de résolution figée** : override > skin > damier. Cette hiérarchie est le contrat que
  LOT-51 (visibilité par calque) devra respecter en l'inversant délibérément pour son affichage
  « objets interactifs seuls ».
- **Un seul point de nettoyage** (`removeLinkedDataAt`) : pas de mécanisme parallèle.
- **Séparation asset / clip** (cf. Exclus) : posée maintenant pour que LOT-46 n'ait pas à changer le
  format d'override.

## Exigences couvertes
- Nouvelle : `EX-EDIT-043` (assignation d'une texture par instance, priorité sur le skin).
- Réutilisées : `EX-REN-043`/`EX-REN-046` (rendu, bascule), `EX-EDIT-042` (skin, priorité
  inférieure), `EX-EDIT-010` (réutilisation du modèle `Core`), `EX-NFR-040` (repli), `EX-LVL-005`
  (format versionné), `EX-NFR-004` (vérification sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-override.md) | `TileTextureOverride` sur `Level`/`LevelDraft` + JSON + nettoyage (`removeLinkedDataAt`, `resize`) + undo/redo | `Source/Core/Levels` | ✅ |
| [TACHE-02](tache-02-outil-assignation.md) | Nouvelle valeur *TextureAssign* de `hmi::EditorTool` + geste pur d'assignation | `Source/HMI/Editor` | ✅ |
| [TACHE-03](tache-03-section-objets-priorite.md) | Section « Objets » du panneau « Textures » + résolution de priorité au rendu + dossier `Assets/Objects/` | `Source/HMI/Editor`, `Source/HMI/Graphics`, `Source/HMI/CMakeLists.txt` | ✅ |

## Critères d'acceptation du lot
1. Assigner une texture à une case précise l'affiche avec cette texture, même si son type a un skin
   différent (LOT-42) — priorité assertée sans GPU.
2. Repeindre un autre type de tuile sur cette case retire l'override ; redimensionner hors de sa
   position aussi.
3. Copier-coller une région ne copie **pas** les overrides des cases source.
4. Un niveau existant (sans override) se charge sans changement de comportement.
5. Round-trip JSON, mutateurs `LevelDraft` (dont undo/redo) et geste d'assignation testés sans GPU ;
   build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40), [LOT-41](@ref lot-41), [LOT-42](@ref lot-42) (résolveur de
priorité) et [LOT-44](@ref lot-44) (format versionné, patron de données annexes). Bénéficie de
[LOT-43](@ref lot-43) (vignettes). Réutilise le patron `Mechanism`/`DangerLink` (`Core`) et
`LinkGesture`/`LinkGeometry`/`LinkPanel` (LOT-37, `HMI`). Prépare [LOT-47](@ref lot-47),
[LOT-51](@ref lot-51).

## Navigation des tâches
- @subpage lot-45-tache-01-modele-override
- @subpage lot-45-tache-02-outil-assignation
- @subpage lot-45-tache-03-section-objets-priorite
