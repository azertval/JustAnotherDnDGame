# LOT-28 — Audio, effets et version `0.2.0` {#lot-28}

> Statut : **à faire**.
> Prérequis : [LOT-27](@ref lot-27).

## Objectif

Donner au jeu son identité sonore et ses retours d'impact, puis clore le programme du *vertical
slice* par une version publiable.

## Périmètre

### Audio

`hmi::AudioEngine` et `SoundCatalog` sont hérités et fonctionnels — ce lot les **alimente**, il ne
les réécrit pas :

- musique de carte et thème de combat, avec bascule à l'entrée et à la sortie de rencontre ;
- bruitages : pas, interaction, ouverture de coffre, jet de dé, coup porté, coup critique, échec,
  fin de combat, navigation de menu.

La règle héritée tient : **le jeu reste pleinement jouable en silence**, sans périphérique audio,
et le volume est réglable et persisté.

### Effets

`ParticleRenderer` et la secousse d'écran sont hérités : particules à l'impact, secousse **sur
critique uniquement** (une secousse à chaque coup rendrait un combat tour par tour épuisant).

### Équilibrage

Passe de réglage sur les données du slice : PV, CA, dégâts, seuils d'expérience, prix. Tout est en
JSON depuis la phase C — aucune recompilation.

### Clôture

- Régénération du **cahier de test** (`scripts/generate_cahier_test.py`).
- Mise en cohérence documentaire globale : specs, guides, manuel.
- Bump `project(VERSION 0.2.0)` dans le `CMakeLists.txt` racine — **seul endroit** où la version
  s'écrit — puis tag `v0.2.0`.

## Note de méthode

La documentation de chaque domaine est mise à jour **par le lot qui le livre**, pas ici. Ce lot ne
fait que la cohérence d'ensemble et la régénération. Un lot fourre-tout de fin de programme n'est
jamais fini : c'est le risque à éviter.

## Exigences couvertes

`EX-REN-*` audio et effets, `EX-NFR-*` (budget de rendu mesuré, patron hérité).

## Critères d'acceptation

- Le jeu est pleinement jouable **sans périphérique audio**.
- Volume réglable et persisté ; bascule musicale exploration ↔ combat sans coupure brutale.
- Secousse d'écran réservée aux critiques.
- Cahier de test régénéré, tous les linters verts, version bumpée et taguée.
