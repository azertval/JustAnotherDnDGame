# TACHE-05 — Outils de zone : remplissage rectangulaire, sélection, copier/coller {#lot-15-tache-05-outils-rectangle-selection}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `Core/Levels`, `HMI/Editor`, `HMI/Interface` · **Statut :** à faire

## Contexte
Peindre une grande zone ou dupliquer un agencement (une salle, un motif de danger) case par case
est long et source d'erreurs. Cette tâche introduit deux outils complémentaires au pinceau
existant, sans dupliquer la sémantique déjà portée par `LevelDraft::paintTile` (unicité
entrée/sortie, nettoyage des liaisons).

## Travail à réaliser
- **`LevelDraft::paintRegion(int originColumn, int originRow, const std::vector<std::vector<TileType>>&
  block) `** (`Core`) : applique `block` (lignes × colonnes) à partir de `(originColumn,
  originRow)`, découpé aux bornes de la grille, en repassant par la même logique cellule-par-cellule
  que `paintTile` (déplacement entrée/sortie, nettoyage des liaisons) — mais en ne poussant
  **qu'un seul** snapshot undo pour tout le bloc. `paintTile` est refactorée en un appel interne à
  cette même logique (bloc 1×1), sans changement de comportement observable (non-régression sur les
  tests LOT-14 existants).
- **`EditorTool`** (enum, `EditorScreen`) : `Pinceau` (comportement actuel, par défaut),
  `Rectangle`, `Selection`.
- **Outil Rectangle** : glisser définit un rectangle (case de départ, case courante), prévisualisé
  en surbrillance pendant le glisser ; au relâchement, `paintRegion` applique le type sélectionné de
  la palette à toute la zone.
- **Outil Sélection + copier/coller** : glisser définit une sélection rectangulaire (même
  prévisualisation) ; `Ctrl+C` (`Key::C`, TACHE-01) lit directement `LevelDraft::tileMap()` (déjà
  public, aucun ajout `Core` nécessaire côté copie) pour construire un presse-papiers local à
  `EditorScreen` (`std::vector<std::vector<TileType>>`) ; `Ctrl+V` (`Key::V`, TACHE-01) colle ce
  bloc via `paintRegion`, ancré au coin supérieur gauche de la case survolée, découpé aux bornes de
  la grille.
- Changement d'outil via la barre d'outils (clic, TACHE-06) et/ou `Tab` (déjà déclaré, inutilisé
  dans l'éditeur) qui fait défiler les trois outils dans l'ordre Pinceau → Rectangle → Sélection →
  Pinceau.
- **Orthogonalité avec l'existant** : le type de tuile sélectionné dans la palette continue de
  s'appliquer aux outils Pinceau et Rectangle (l'outil Sélection l'ignore : il agit sur les tuiles
  déjà présentes, pas sur un type à poser). La liaison de mécanismes (Maj+clic, LOT-14) reste
  disponible **quel que soit l'outil actif** — c'est un geste distinct (touche `Maj` maintenue),
  prioritaire sur le comportement du glisser normal de l'outil courant, à l'identique de la
  priorité déjà existante de la palette sur la grille.

## Fichiers impactés
- `Source/Core/Levels/LevelDraft.h`/`.cpp` (`paintRegion`, refactor interne de `paintTile`).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (état outil, rectangle en cours, presse-papiers,
  prévisualisation).
- `Source/Test/Unit/Core/Levels/test_level_draft.cpp` (nouveaux cas), test système étendu.

## Tests (obligatoires)
- `paintRegion` sur un bloc homogène équivaut à peindre chaque case individuellement (même grille
  résultante), mais avec **un seul** `undo()` nécessaire pour tout annuler.
- `paintRegion` en bordure de grille découpe silencieusement le bloc sans dépasser les bornes, sans
  erreur.
- `paintRegion` qui inclut une nouvelle position d'entrée/sortie déplace l'ancienne (même sémantique
  que `paintTile`) ; qui recouvre un `Switch`/`Door` existant en retire la liaison (idem).
- Copier une zone puis coller ailleurs reproduit exactement les types de tuiles copiés ; coller
  près d'un bord découpe la partie hors grille.
- Non-régression : tous les tests `paintTile` existants (LOT-14) passent inchangés après le
  refactor interne.
- `Tab` fait défiler les trois outils dans l'ordre attendu et revient au premier après le dernier.
- Maj+clic sur un `Switch`/`Door` produit une liaison quel que soit l'outil actif (Pinceau,
  Rectangle ou Sélection).
- Changer d'outil pendant un glisser en cours n'applique aucune mutation au `LevelDraft`.

## Points d'attention
- Le presse-papiers reste **local à `EditorScreen`** (pas de `Core`), car il ne représente qu'un
  découpage temporaire de l'écran, pas un concept persistant du modèle de niveau.
- Copier/coller n'a pas de sémantique spéciale pour l'entrée/la sortie/les mécanismes au-delà de
  celle déjà portée par `paintRegion`/`paintTile` (déplacement d'unicité, nettoyage de liaison) —
  ne pas ajouter de règle supplémentaire non demandée par la spec.
- La prévisualisation (rectangle en cours de glisser) est un rendu **par-dessus** la grille, sans
  toucher au `LevelDraft` avant relâchement — cohérent avec le principe « un geste = une mutation
  undoable ».
- Changer d'outil (`Tab`, ou clic sur la barre d'outils) pendant un glisser en cours **annule** le
  glisser (aucune mutation, aucun ajout au presse-papiers) plutôt que de l'appliquer avec le
  nouvel outil — évite un résultat surprenant en cas de changement d'avis à mi-geste.

## Définition de fait (DoD)
- Remplissage rectangulaire et sélection/copier-coller opérationnels et testés (`ctest` vert) ;
  build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-014`, `EX-EDIT-010` (aucune règle de niveau dupliquée).
