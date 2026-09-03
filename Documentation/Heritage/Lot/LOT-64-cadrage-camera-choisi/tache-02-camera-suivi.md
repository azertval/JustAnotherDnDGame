# TACHE-02 — Caméra de suivi {#lot-64-tache-02-camera-suivi}

**Lot :** [LOT-64](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
C'est le mode qui **manque au moteur**, et la tâche substantielle du lot. `hmi::Camera2D` sait
placer un centre et un zoom ; tout le reste — décider **où** est ce centre — est aujourd'hui
répondu de deux façons seulement : le centre du niveau (`LOT-16`), ou le centre de la salle
courante (`LOT-32`). Aucune ne suit le personnage.

`EX-REN-015` écarte d'ailleurs explicitement le suivi (« sans jamais suivre le personnage en
continu ») — non parce qu'il serait mauvais, mais parce que le `LOT-32` traitait le cas des grands
niveaux à salles. Ce lot en fait l'un des trois modes, choisi par le niveau.

La famille de défauts d'une caméra de suivi est connue : caméra qui tremble parce qu'elle colle
exactement au personnage, qui oscille à chaque changement de direction, qui déborde des limites et
montre le vide hors grille, ou qui rend le rendu flou parce que son centre n'est pas aligné sur la
grille de pixels.

## Travail à réaliser
- **Zone morte** : le personnage se déplace librement dans un rectangle central ; la caméra ne bouge
  que lorsqu'il en sort. C'est ce qui supprime le tremblement permanent.
- **Anticipation** dans le sens du déplacement : le centre visé est décalé devant le personnage, de
  sorte qu'on voie où l'on va. Le décalage s'inverse **progressivement** au changement de sens,
  jamais d'un coup.
- **Lissage** vers le centre visé, avec un temps de réponse constant nommé.
- **Bornage aux limites du niveau** : la caméra ne montre jamais hors de la grille. Si le niveau est
  plus étroit que le cadrage sur un axe, elle est **centrée** sur cet axe plutôt que bornée — le cas
  limite qui produit sinon un cadrage collé à un bord.
- **Alignement pixel** : le centre retenu est aligné sur la grille de pixels à l'échelle de rendu,
  sinon les textures sont échantillonnées entre deux texels et tout le pixel art devient flou. Le
  zoom reste **entier** (`EX-ARCH-022`).
- **Fonction pure** : à partir de (position du personnage, sens, cadrage, limites du niveau, état
  précédent de la caméra), produire le centre suivant. Testable sans GPU (`EX-NFR-004`), comme
  `hmi::RoomGrid` et `hmi::Parallax` l'ont déjà été.
- **Aucun effet sur la simulation** (`EX-ARCH-012`) : la caméra lit, elle n'écrit rien.

## Fichiers impactés
- `Source/HMI/Graphics/FollowCamera.{h,cpp}` (nouveau) — logique pure.
- `Source/HMI/Graphics/Camera2D.{h,cpp}` — alignement pixel du centre, si absent.
- `Source/HMI/Game/GameViewport.cpp` — sélection du mode selon le niveau.
- `Source/Test/Unit/HMI/Graphics/test_follow_camera.cpp` (nouveau), `Source/Test/CMakeLists.txt`
  (compiler `FollowCamera.cpp` dans `UnitTests`, comme `RoomGrid.cpp`).

## Tests (obligatoires)
- **Zone morte** : un déplacement à l'intérieur de la zone ne déplace **pas** la caméra ; en sortir
  la déplace.
- **Bornage** : sur un niveau plus grand que le cadrage, la caméra ne montre jamais au-delà des
  limites, sur les quatre bords.
- **Niveau plus étroit que le cadrage** sur un axe → caméra **centrée** sur cet axe, pas collée à un
  bord.
- **Anticipation** : le décalage s'inverse progressivement au changement de sens, sans discontinuité
  — le défaut qui donne le mal de mer.
- **Alignement pixel** : le centre retenu est toujours aligné sur la grille de pixels à l'échelle de
  rendu.
- **Déterminisme** : mêmes entrées, même trajectoire de caméra.
- Tests **purs**, sans GPU.

## Points d'attention
- **Le lissage doit être cadencé sur le pas de simulation**, pas sur la fréquence de rendu : depuis
  le `LOT-33`, les deux sont découplées, et une caméra lissée par image se comporte différemment à
  60 et à 144 Hz.
- **L'alignement pixel est ce qu'on oublie**, et il ruine visuellement tout le travail d'habillage
  des seize lots précédents. À vérifier à l'œil autant que par test.
- La caméra suit la position **interpolée** pour l'affichage (`hmi::PreviousPosition`), pas la
  position simulée brute, sinon le personnage tremble par rapport au décor.
- `EX-REN-015` affirme aujourd'hui que la caméra ne suit **jamais** le personnage en continu : cette
  formulation devra être nuancée à la `TACHE-04`, la contrainte devenant propre au mode *par salle*.
- Ne pas rendre les paramètres (taille de zone morte, anticipation, lissage) éditables : constantes
  nommées, hors périmètre du lot.

## Définition de fait (DoD)
- Un mode de caméra suivi existe, avec zone morte, anticipation progressive, lissage cadencé sur le
  pas fixe, bornage aux limites (centrage sur l'axe trop étroit) et alignement pixel ; la logique est
  pure et testée sans GPU ; aucun effet sur la simulation ; `/W4 /WX` propre.

## Exigences
`EX-REN-016` (modes de cadrage, dont la caméra de suivi bornée) ; réutilise `EX-REN-013` (caméra 2D),
`EX-REN-015` (mode par salle), `EX-ARCH-022` (zoom entier, *nearest*), `EX-ARCH-021` (pixels par
unité), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-REN-021` (pas fixe), `EX-NFR-004`
(vérification sans GPU).
