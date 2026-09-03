# TACHE-03 — Intégration éditeur (palette, rendu, liaison) {#lot-31-tache-03-integration-editeur}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics`, `HMI/Interface` · **Statut :** ✅

## Contexte
Rend les sept nouveaux types **peignables** depuis la palette (`EX-EDIT-002`), le danger
directionnel **visuellement fidèle** à sa hitbox réelle, et la liaison déclencheur↔danger commuté
éditable au même geste qu'une porte (`EX-EDIT-003`).

**Deux réductions de périmètre tranchées pendant l'implémentation**, documentées ici plutôt que
laissées implicites :
- **Pas de geste dédié « portée » pour `DangerMover`.** L'idée initiale (second clic après avoir
  peint la tuile, pour définir axe/portée à la souris) n'a pas été implémentée : elle aurait ajouté
  un état d'interaction supplémentaire (`_pendingMoverConfig` ou équivalent) pour un gain marginal —
  `LevelDraft::setMoverConfig` existe déjà (TACHE-01) et couvre le besoin **programmatique** ; sans
  UI dédiée, un danger mobile peint depuis la palette garde l'axe/la portée de conception par défaut
  (horizontal, 2 cases) tant que `setMoverConfig` n'est pas appelé — cohérent avec l'exclusion déjà
  actée par l'epic (pas de widget numérique). Un futur lot pourra ajouter ce geste s'il s'avère
  réellement nécessaire à l'usage.
- **Mobile/Commuté/Temporisé partagent la couleur du danger classique** (même région d'atlas, rouge)
  plutôt que des teintes distinctes. La grille procédurale de `TextureAtlas` (5×5 cases) n'a
  qu'**une seule** case encore libre (`(3,4)`) — les 24 autres portent déjà une couleur ou un masque
  de forme (pentes/arrondis). Une distinction fiable aurait exigé d'agrandir la grille
  (`TILES_PER_SIDE`), une opération plus large et plus risquée (réindexation complète de la palette
  de couleurs procédurale) que ce que ce lot justifie : le **type** de la tuile (visible dans la
  palette au moment de peindre) et son **comportement** restent la source de vérité, la couleur
  n'étant qu'un indice secondaire. Le danger **directionnel**, lui, reste visuellement fidèle
  (bande sur son bord réel) car cela ne nécessite aucune nouvelle case d'atlas — seulement un
  positionnement différent du même quad rouge.

## Travail réalisé
- **`TilePalette.h`/`.cpp`** : la catégorie « Piège » (`Category`), avant `pushStandalone` (une
  seule feuille), devient un **en-tête de catégorie** (comme « Tuile »/« Interactif ») :
  - `Danger` reste une feuille directe (« Classique »).
  - Nouveau sous-groupe « Directionnel » (`Subgroup::Directionnel`) : quatre feuilles (`DangerUp`/
    `Down`/`Left`/`Right` — « Haut »/« Bas »/« Gauche »/« Droite »), même patron que « Pente ».
  - Trois nouvelles feuilles directes : `DangerMover` (« Mobile »), `DangerSwitched` (« Commuté »),
    `DangerBlink` (« Clignotant »).
- **`core::isDangerTileType`** (nouveau, `Core/Levels/DangerGeometry.h`/`.cpp`) : vrai pour les huit
  types de danger — utilisé par le rendu (ci-dessous) pour ne recalculer un quad asymétrique qu'à
  ces types précis.
- **`core::buildLevelScene`** (`Core/Levels/LevelScene.cpp`) : pour une tuile de danger, la `Transform` de
  l'entité (position + échelle) est dérivée de `core::dangerHitbox` plutôt que de la case pleine par
  défaut — bande décalée sur le bord réel pour les quatre directionnels, case pleine inchangée
  sinon. Même principe que `core::tileVisualScale` pour les blocs réduits (`EX-GP-005`), mais
  décalage asymétrique plutôt que centré.
- **`HMI/Interface/EditorScreen.cpp`** :
  - Canevas d'édition : même logique `core::dangerHitbox`/`isDangerTileType` appliquée au dessin
    immédiat (`quadFor`), pour un aperçu fidèle à ce que `LevelScene` produira en jeu.
  - `isLinkTargetTile` (nouveau, à côté de `isTriggerTile`) : vrai pour `Door` **et**
    `DangerSwitched`. `handleLinkClick` généralisé en conséquence (variables renommées
    `targetPosition`/`targetType`, plus `switchPosition`/`doorPosition`) — dispatche vers
    `_draft.linkMechanism`, qui route lui-même vers `_mechanisms` ou `_dangerLinks` selon le type
    réel de la cible (TACHE-01).
  - Rendu des liaisons : les cases d'un `DangerLink` (déclencheur + danger) reçoivent la **même**
    teinte que celles d'un `Mechanism` partageant le même déclencheur (espace de couleurs unifié,
    `LINK_TINTS`) — un déclencheur qui active à la fois une porte et un danger commuté reprend une
    teinte cohérente sur ses trois cases, plutôt que deux échelles de couleur indépendantes
    (`EX-EDIT-016`, plusieurs liaisons simultanées restent distinguables **entre elles**).
- **Niveau de démonstration** : `Source/Elements/Levels/demo-dangers-avances.json` (nouveau),
  intégré à la séquence jouée (`Source/HMI/main.cpp`, juste après `demo-bloc-reduit.json`, avant
  `demo-final.json`). Les quatre variantes sont posées sur des **alcôves surélevées optionnelles**
  au-dessus d'un couloir principal ininterrompu — comme les douze niveaux de démo existants, aucun
  n'exige de scénario de mort pour être franchi ; ce niveau ne fait pas exception.

## Fichiers impactés
- `Source/Core/Levels/DangerGeometry.h`/`.cpp` (`isDangerTileType`).
- `Source/Core/Levels/LevelScene.cpp` (géométrie asymétrique pour les tuiles de danger).
- `Source/HMI/Editor/TilePalette.h`/`.cpp` (catégorie Piège restructurée).
- `Source/HMI/Graphics/TileVisuals.cpp` (région d'atlas des sept types).
- `Source/HMI/Interface/EditorScreen.cpp` (`isLinkTargetTile`, `handleLinkClick` généralisé, rendu
  du canevas et des liaisons de danger commuté).
- `Source/Elements/Levels/demo-dangers-avances.json` (nouveau), `Source/Elements/Levels/README.md`
  (décompte de la séquence, 14 fichiers).
- `Source/HMI/main.cpp`, `Source/Test/Systeme/test_parcours_complet.cpp` (insertion dans la
  séquence, aux deux endroits — vérifié par `scripts/check_demo_sequence.py`).
- Tests : `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp` (catégorie Piège restructurée,
  sous-groupe Directionnel), `Source/Test/Systeme/test_parcours_edition.cpp` (nouveau test —
  peinture/liaison/configuration des sept types, round-trip disque).

## Tests (réalisés)
- Chaque nouveau type est sélectionnable depuis la palette et peint la bonne valeur de `TileType`
  (`CategoriePiegeDeplieeExposeSesFeuilles`, `SousGroupeDirectionnelDeplieExposeSesBords`).
- Une liaison déclencheur → `DangerSwitched` se crée/se défait comme une liaison déclencheur →
  `Door` : couvert au niveau `Core` par `test_level_draft.cpp` (TACHE-01, `LierUnDangerCommute`/
  `DelierUnDangerCommute`) — `EditorScreen::handleLinkClick` n'a pas de fichier de test unitaire
  dédié (aucun écran `HMI/Interface` n'en a : ils dépendent de Direct3D/`InputState`, vérifiés
  visuellement par convention du projet), mais délègue tout son comportement observable à
  `LevelDraft`, déjà testé.
- `test_tile_palette.cpp` : la restructuration de la catégorie « Piège » ne casse aucun test
  existant (`EtatInitialCinqEntreesVisibles`, `FenetreReduiteLimiteLesEntreesVisibles` toujours
  verts sans modification — le nombre d'entrées collapsed est inchangé).
- `test_parcours_edition.cpp` : peinture des sept types + liaison + configuration explicite
  (mobile, temporisé) survivent à un enregistrement/rechargement disque complet.
- `scripts/check_demo_sequence.py` retourne un code de sortie `0` (14 niveaux, `Source/HMI/main.cpp`
  et `test_parcours_complet.cpp` identiques).

## Définition de fait (DoD)
- Les sept types sont éditables de bout en bout (peinture, liaison, enregistrement, rechargement,
  essai immédiat) sans édition manuelle de JSON — à l'exception de l'axe/la portée d'un danger
  mobile personnalisés (valeurs par défaut sans geste dédié, cf. réduction de périmètre ci-dessus).
- Couverture des **trois niveaux de test** du projet pour l'ensemble du lot : Unit (TACHE-01/02),
  Integration (TACHE-02), Système (cette tâche — `demo-dangers-avances` jouée bout en bout dans la
  séquence réelle via `test_parcours_complet.cpp`, et round-trip d'édition via
  `test_parcours_edition.cpp`). Build `/W4 /WX` et suite complète (453 Unit + 81 Integration +
  3 Système) vertes, sans régression.

## Exigences
`EX-EDIT-001`, `EX-EDIT-002`, `EX-EDIT-003`, `EX-EDIT-016`, `EX-GP-050`, `EX-GP-051`, `EX-GP-052`,
`EX-GP-053`.
