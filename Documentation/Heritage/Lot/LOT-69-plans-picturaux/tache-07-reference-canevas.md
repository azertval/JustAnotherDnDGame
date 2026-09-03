# TACHE-07 — Référence peignable et extensions du canevas {#lot-69-tache-07-reference-canevas}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Peindre un plan « à 1:1 sur le niveau complet » demande deux choses que l'atelier du `LOT-54` n'a
pas : voir **le niveau** sous le pinceau, et pouvoir **dézoomer sous 1:1** pour l'embrasser en
entier.

Le choix du support a été pesé au cadrage. Le viewport (chemin GPU) n'apporterait qu'une chose — la
fidélité de la référence — contre la réécriture du pinceau, du remplissage, de la sélection, de
l'historique et du presse-papiers, c'est-à-dire une seconde implémentation divergente de
`EX-EDIT-045`. `hmi::PixelCanvas` apporte tout le reste et ne manque que la référence. C'est donc lui
qu'on étend.

## Travail à réaliser
- `PlaneReference.{h,cpp}` (nouveau), **sans Qt ni GPU** (`EX-NFR-010`) :
  - `buildTileOnionSkin(const core::LevelDraft&, int pixelsPerUnit)` — une couleur plate par type de
    tuile, en **réutilisant la palette du mode Physique**, qui est déjà « la lecture des collisions
    sans distraction ».
  - `flattenPlanes(...)` — alpha-over des autres plans visibles, rééchantillonnés par ratio
    **entier** (16 → 8 = un pixel sur deux ; jamais d'interpolation).
  - `resamplePlane(image, fromPpu, toPpu)` — sert aussi au changement de densité (`TACHE-08`).
- `PixelCanvas` : trois entrées **optionnelles et additives** — `setUnderlay(image, opacity)`,
  `setOverlay(image, opacity)`, `setReferenceGridStep(px)` (grille de tuiles, distincte de la grille
  de pixels existante). Elles sont dessinées avec la même géométrie que le contenu et **jamais**
  incluses dans le tampon édité, ni dans l'historique, ni dans le copier.
- **Zoom rationnel** : `PixelCanvasView::zoom` passe d'un entier `≥ 1` à un couple
  `{numérateur, dénominateur}`, dénominateurs limités à 1/2/4/8 — les pixels restent **carrés**
  (`EX-ARCH-022`). Adapter les conversions vue ↔ image, et masquer la grille de pixels sous 1:1.

## Fichiers impactés
`Source/HMI/Editor/PlaneReference.{h,cpp}` (nouveau), `PixelCanvas.{h,cpp}`,
`PixelCanvasGeometry.{h,cpp}`, `Source/HMI/CMakeLists.txt`.

## Tests (obligatoires)
- `test_plane_reference.cpp` (nouveau) : pelure d'oignon **déterministe** ; aplatissement ignorant un
  plan masqué ; rééchantillonnage 16 → 8 exact ; alpha-over associatif.
- `test_pixel_canvas_geometry.cpp` **étendu** : aller-retour vue ↔ image aux dénominateurs 2 et 4 ;
  pixels carrés à tout zoom ; et surtout **les cas existants (dénominateur 1) passent sans
  retouche**.

## Points d'attention
C'est **la seule modification d'un module déjà livré et testé**, donc le principal risque de
régression du lot. Le zoom rationnel doit être une **extension compatible** : `zoom` conservé comme
numérateur, dénominateur 1 par défaut, pour que l'atelier pixel art ne bouge pas d'un pixel. Le
critère n'est pas « les tests passent » mais « les tests passent **sans être modifiés** ».

La référence n'est **pas** un aperçu : ni raccords automatiques, ni skins, ni objets animés. C'est un
repère géométrique. Le dire dans la documentation utilisateur, sinon l'attente sera déçue.

Le 1:1 est exact **par construction** : l'image du plan *est* le niveau à sa densité, donc zoom 1 =
un pixel image = un pixel écran = un pixel du jeu.

## Definition de fait (DoD)
Le canevas affiche une référence réglable, descend sous 1:1 en gardant des pixels carrés, et les
tests hérités du `LOT-54` passent inchangés. `ctest` à 100 %.

## Exigences
`EX-EDIT-046`, `EX-EDIT-045`, `EX-DEC-045`, `EX-DEC-041`, `EX-ARCH-022`, `EX-NFR-010`.
