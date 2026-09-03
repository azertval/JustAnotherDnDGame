# LOT-40 — Fondations : registre de textures, calques, culling, testabilité {#lot-40}

> Statut : **fait**. Prérequis : [LOT-39](@ref lot-39) (chargement de textures depuis
> fichiers). Premier lot du programme d'habillage `LOT-40` → `LOT-55` ; prérequis de tous les
> suivants.

## Objectif
Lever les verrous qui bloquent l'ensemble du programme, **sans changer un seul pixel affiché
aujourd'hui**. Quatre verrous, tous structurels :

1. `hmi::SpriteBatch::begin(projection, texture)` ne lie qu'**une seule** texture par lot, et
   `hmi::SpriteRenderer`/`hmi::DraftRenderer` pilotent chacun une unique `hmi::TextureAtlas` de bout
   en bout : aucune couche à venir n'est dessinable tant qu'un niveau ne peut afficher qu'une
   texture à la fois.
2. Il n'existe **aucun ordonnancement de calques nommé** : le joueur est un `layer = 100` magique
   (`GameSession.cpp`), les tuiles un `0` (`LevelScene.cpp`, `DraftRenderer.cpp`). Chaque lot
   suivant inventerait le sien.
3. Le rendu n'est **pas vérifiable** autrement qu'à l'œil, alors que le programme va empiler six
   couches dont l'ordre et la priorité de résolution sont exactement le genre de règle qui casse
   silencieusement.
4. Le rendu soumet **tout le niveau** à chaque image, sans culling, avec un plafond de
   `MAXIMUM_QUADS = 2048` par lot — alors que le programme va multiplier le volume de primitives sur
   des niveaux de grande taille (`LOT-16`).

## Périmètre

### Inclus
- ***TextureCache*** : registre de textures chargées à la demande par **nom logique**, construit
  directement sur *TextureLoader*/`hmi::AssetPaths` (LOT-39) — décodage, upload D3D11 et résolution
  de chemin sont déjà génériques, seule la mise en cache par nom est nouvelle. RAII (`ComPtr`),
  repli sans exception si un asset est absent/illisible (`EX-NFR-040`). **`invalidate(name)` et
  `invalidateAll()` font partie de l'API dès l'origine** : le rechargement à chaud (LOT-43) et
  l'aperçu live de l'atelier (LOT-54) en dépendent, et les greffer après coup obligerait à modifier
  une API déjà consommée par quatre lots.
- **Contrat d'asset et validation** (`EX-REN-007`) : chaque famille d'asset déclare ses dimensions
  attendues ; le cache **valide** à l'entrée et refuse un asset non conforme en journalisant le nom
  du fichier et la dimension attendue. Sans cela, un PNG de travers ne produit pas une erreur mais
  des artefacts de filtrage silencieux (`POINT`, sans mipmap).
- ***RenderLayer*** : jeu de valeurs **unique**, aligné sur `EX-REN-014` et `EX-DEC-002`, réservé
  maintenant même si une seule valeur est utilisée avant LOT-42 :
  `Background, Decor, Shadow, Tile, Object, Player, Foreground, UI, EditorOverlay`. Le `layer = 100`
  du joueur et le `0` des tuiles rejoignent l'enum — plus aucune valeur magique.
- **Rendu multi-textures** : `SpriteRenderer`/`DraftRenderer` regroupent leurs quads par
  `(RenderLayer, ID3D11ShaderResourceView*)` et émettent un `SpriteBatch::begin/end` par groupe
  contigu, **dans l'ordre des calques** — `SpriteBatch` lui-même n'est pas modifié (même décision
  que LOT-39 pour `TextureAtlas` : interface conservée).
- **Texture de repli en damier magenta**, sœur de `hmi::buildProceduralAtlasImage`, créée une fois,
  utilisée par tout lot ayant besoin d'indiquer « texture attendue mais absente », avec
  `GRAPHICS_LOG_WARNING`.
- ***QuadRecorder*** (`EX-NFR-004`) : couche d'inspection capturant la liste des `SpriteQuad`/
  `LineQuad` soumis pour une scène donnée, sans GPU. Rend assertables l'ordre des calques, le
  regroupement par texture, et — pour les lots suivants — la priorité de résolution, la règle de
  raccords, l'isolement d'un calque, la présence des ombres.
- **Culling par salle** (`EX-NFR-005`) : n'émettre que les primitives intersectant le cadrage de la
  caméra, en réutilisant `hmi::RoomGrid::roomBounds` (LOT-32) — aucune nouvelle notion de spatialité.

### Exclus (hors périmètre de ce lot)
- Aucune nouvelle couche visible (fond, décor, skin, objets) — ce lot ne change **aucun** pixel
  affiché aujourd'hui.
- Aucun changement de `Level`/`LevelDraft` (`Core` non touché).
- Aucune UI éditeur.
- Optimisation par atlas dynamique/packing : hors périmètre. Le culling traite la vraie cause du
  volume ; plusieurs passes `begin/end` par image restent largement assez rapides à l'échelle d'une
  salle. À revisiter seulement si un profilage futur montre un problème réel.
- Éviction ou budget mémoire du cache : les assets restent bornés ; `invalidate` sert au
  rechargement, pas à une politique de libération.

## Décisions de cadrage
- **Plusieurs passes `begin/end` plutôt qu'un atlas partagé dynamique** : simplicité, coût des
  changements d'état D3D11 négligeable à l'échelle d'une salle, `SpriteBatch` reste inchangé.
- **Tri stable, calque prioritaire** : à l'intérieur d'un calque, le regroupement par texture ne
  doit **jamais** réordonner deux sprites de calques différents. L'ordre est
  *RenderLayer* → texture → `Sprite::layer` (tri fin existant), jamais l'inverse.
- ***RenderLayer* complet dès maintenant**, y compris les valeurs `Decor`, `Player`, `Foreground` et
  `UI` qui ne servent qu'à partir de LOT-48/49/52 : c'est précisément l'anticipation qui évite qu'un
  lot suivant réinvente un ordonnancement concurrent. Le calque `Foreground` est **au-dessus** de
  `Player` par construction — c'est le contrat de lecture « ce qui est devant n'est pas physique ».
- **La testabilité est une fonctionnalité du lot, pas un bonus** : un critère d'acceptation formulé
  en « pixel-identique » ou « ordre correct » est invérifiable à l'œil. Le *QuadRecorder* le rend
  assertable, ici et dans tous les lots suivants (`EX-NFR-004`).
- **Culling maintenant, pas plus tard** : introduit tant que le rendu est encore simple et que la
  non-régression est facile à établir ; l'ajouter après six couches serait un chantier de débogage
  visuel.

## Exigences couvertes
- Nouvelles : `EX-REN-043` (rendu multi-textures par calques), `EX-REN-007` (contrat d'asset et
  validation), `EX-NFR-004` (rendu vérifiable sans GPU), `EX-NFR-005` (culling et budget).
- Amendée : `EX-REN-014` (ordonnancement de calques explicite, premier plan au-dessus du
  personnage).
- Réutilisées : `EX-NFR-040` (repli sans plantage), `EX-NFR-010` (logique testable sans GPU),
  `EX-ARCH-022` (filtrage *nearest*, inchangé), `EX-REN-015` (cadrage par salle, base du culling).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-texture-cache.md) | *TextureCache* — registre par nom logique, invalidation, contrat d'asset | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-rendu-multicouche.md) | *RenderLayer* + regroupement des quads par `(calque, texture)` | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-damier-magenta.md) | Texture de repli en damier magenta + point de résolution unique | `Source/HMI/Graphics`, `Source/Test` | ✅ |
| [TACHE-04](tache-04-quad-recorder.md) | *QuadRecorder* — capture des primitives soumises, tests sans GPU | `Source/HMI/Graphics`, `Source/Test` | ✅ |
| [TACHE-05](tache-05-culling-salle.md) | Culling par salle visible + observation du nombre de primitives | `Source/HMI/Graphics`, `Source/Test` | ✅ |

## Critères d'acceptation du lot
1. Le rendu (jeu et éditeur) est **inchangé** : la liste des primitives soumises pour une scène
   donnée est identique à celle d'avant le lot, **assertée** par le *QuadRecorder* — hors primitives
   écartées par le culling, qui font l'objet de leur propre test.
2. Une scène de test affiche des sprites provenant d'**au moins deux textures distinctes** en une
   image, dans l'ordre de calque attendu, **assertée sans GPU**.
3. `hmi::SpriteBatch` n'a **aucune** signature publique modifiée.
4. Le regroupement `(calque, texture)`, l'invalidation du cache, la validation de dimensions et le
   culling sont testés **sans GPU**.
5. Aucune valeur de calque magique ne subsiste dans `GameSession`, `LevelScene` ou `DraftRenderer`.
6. Un asset aux dimensions non conformes est refusé avec un message nommant le fichier et l'attendu,
   et affiche le damier — sans interrompre le rendu.
7. Build `/W4 /WX`, Doxygen et lint verts.

## Dépendances
Bâtit sur [LOT-39](@ref lot-39) (*TextureLoader*, `hmi::AssetPaths`,
`hmi::buildProceduralAtlasImage`) et [LOT-32](@ref lot-32) (`hmi::RoomGrid`, base du culling). Ne
modifie pas `Core`. Prérequis de [LOT-41](@ref lot-41) à [LOT-55](@ref lot-55).

## Navigation des tâches
- @subpage lot-40-tache-01-texture-cache
- @subpage lot-40-tache-02-rendu-multicouche
- @subpage lot-40-tache-03-damier-magenta
- @subpage lot-40-tache-04-quad-recorder
- @subpage lot-40-tache-05-culling-salle
