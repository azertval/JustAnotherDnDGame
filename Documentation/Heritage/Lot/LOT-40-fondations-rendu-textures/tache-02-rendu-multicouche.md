# TACHE-02 — *RenderLayer* + regroupement des quads par (calque, texture) {#lot-40-tache-02-rendu-multicouche}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`hmi::SpriteRenderer::render` trie déjà ses sprites par `core::Sprite::layer` (entier simple) avant
de les soumettre en **un seul** `SpriteBatch::begin/end`, puisqu'il n'existe aujourd'hui qu'une seule
texture (l'atlas). Cette tâche introduit un ordonnancement de calques **nommé** et fait émettre
**plusieurs** passes `begin/end`, une par groupe contigu de même texture, sans changer une seule
ligne de `SpriteBatch`.

## Travail à réaliser
- ***RenderLayer*** (`Source/HMI/Graphics/RenderLayer.h`) : `enum class RenderLayer { Background,
  Decor, Shadow, Tile, Object, Player, Foreground, UI, EditorOverlay };` — ordre de déclaration =
  ordre de dessin, aligné sur `EX-REN-014` et `EX-DEC-002`. Seules les valeurs *Tile* et *Player*
  sont utilisées par ce lot (comportement actuel) ; les autres sont réservées pour LOT-44 à LOT-55.
  Le calque *Foreground* est **au-dessus** de *Player* par construction : c'est le contrat de
  lecture « ce qui est devant le personnage n'est pas physique ».
- **Suppression des valeurs de calque magiques** : le `layer = 100` du joueur (`GameSession.cpp`) et
  le `layer = 0` des tuiles (`LevelScene.cpp`, `DraftRenderer.cpp`) sont remplacés par les valeurs
  nommées correspondantes. `core::Sprite::layer` **reste** un entier et conserve son rôle de tri
  **fin à l'intérieur** d'un calque — `Core` continue d'ignorer l'existence de *RenderLayer*, qui
  est une notion de présentation (`EX-NFR-011`).
- **`SpriteRenderer`** : la structure interne `LayeredQuad` gagne un *RenderLayer* (en plus de
  `core::Sprite::layer`, qui reste le tri **fin** à l'intérieur d'un calque) et une référence de
  texture. Le tri devient : *RenderLayer* (ordre fixe), puis texture (regroupement, ordre stable),
  puis `Sprite::layer` (comme aujourd'hui) — jamais l'inverse, pour ne pas régresser l'ordre visuel
  actuel. `render()` émet un `begin/end` par groupe contigu de même texture, dans cet ordre.
- **`DraftRenderer`** : même traitement pour la scène de l'éditeur (`rebuild()`).
- Toute entité/tuile existante reste implicitement sur la valeur *Tile* de *RenderLayer* avec la texture de l'atlas
  — **aucun changement de comportement** tant qu'aucun appelant ne pousse un sprite sur un autre
  calque/texture (ce qui n'arrive qu'à partir de LOT-42+).

## Fichiers impactés
- `Source/HMI/Graphics/RenderLayer.h` (nouveau).
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`
  (regroupement, plusieurs passes).

## Tests (obligatoires)
- Logique de regroupement/tri (ordre de calque > texture > `Sprite::layer`) testable sans GPU avec
  des textures/handles factices (pointeurs distincts, pas de vraies ressources D3D11).
- **Non-régression** : la liste des primitives soumises pour une scène de référence est identique
  avant/après, **assertée** via le *QuadRecorder* de la TACHE-04 (`EX-NFR-004`) — et non constatée à
  l'œil. Une vérification visuelle complémentaire reste utile, mais elle ne tient pas lieu de test.

## Points d'attention
- Le tri doit rester **stable** (comme aujourd'hui, `std::stable_sort`) : à texture et calque égaux,
  l'ordre de soumission d'origine est préservé.
- `SpriteBatch::begin/end` : contrat public strictement inchangé (aucune signature modifiée).

## Définition de fait (DoD)
- Rendu multi-textures fonctionnel, liste de primitives identique au rendu actuel tant qu'une seule
  texture est utilisée ; plus aucune valeur de calque magique dans le code ; `/W4 /WX` propre ;
  logique de regroupement testée sans GPU.

## Exigences
`EX-REN-043` (rendu multi-textures par calques) ; amende `EX-REN-014` (ordonnancement explicite) ;
réutilise `EX-NFR-004` (vérification sans GPU), `EX-NFR-011` (pas de dépendance `Core → HMI`).
