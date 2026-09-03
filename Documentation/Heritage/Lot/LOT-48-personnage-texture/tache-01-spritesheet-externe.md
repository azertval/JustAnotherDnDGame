# TACHE-01 — Spritesheet externe et ancrage image/hitbox {#lot-48-tache-01-spritesheet-externe}

**Lot :** [LOT-48](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le personnage est dessiné par *hmi::playerPixel* (`ProceduralAtlas.cpp`) : une silhouette humanoïde
composée de rectangles de couleur — cheveux, peau, chemise, pantalon, chaussures — générée en C++.
Changer son apparence impose de recompiler.

Un piège documenté attend toute modification : la silhouette est **pré-compressée verticalement**
dans le générateur, parce que `Transform::scale` vaut `core::playerSize()` (0,4 × 0,8 unité) et
étire donc l'image. C'est la contrainte « la région du personnage doit rester carrée », héritée d'une
régression corrigée en `LOT-17`.

## Travail à réaliser
- **Chargement** d'une spritesheet depuis `Assets/Player/`, via le *TextureCache* (LOT-40), décrite
  par son `nom-asset.anim.json` (LOT-46).
- **Repli sur l'atlas procédural** si le fichier est absent ou invalide, exactement comme
  `TextureAtlas` le fait déjà pour `atlas.png` (`EX-REN-042`). Le jeu doit rester jouable et lisible
  sans aucun asset.
- **Découplage image / hitbox** — le cœur de la tâche :
  - la spritesheet déclare sa **taille d'image** (indépendante de la hitbox) ;
  - un **point d'ancrage** (par exemple, le centre bas de l'image aligné sur le centre bas de la
    hitbox) relie les deux ;
  - `core::playerSize()` (`Source/Core/Physics/PlayerSpawn.h`) reste **la seule** source de vérité de
    la boîte de collision, inchangée.
  Une image plus grande que la hitbox (cape, cheveux, arme, effet de dash) devient possible sans
  toucher au gameplay.
- **Calcul du quad du personnage** à partir de la taille d'image, de l'ancrage et de l'échelle : une
  fonction **pure**, testable.

## Fichiers impactés
- `Source/HMI/Graphics/PlayerSprite.{h,cpp}` (nouveau) — chargement, ancrage, calcul du quad.
- `Source/HMI/Graphics/TextureAtlas.{h,cpp}`, `ProceduralAtlas.{h,cpp}` (repli conservé).
- `Source/HMI/Game/GameSession.{h,cpp}` (`refreshPlayerSprite`).
- `Source/Elements/Assets/Player/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.
- `Source/Test/Unit/HMI/Graphics/test_player_sprite.cpp` (nouveau).

## Tests (obligatoires)
- **Ancrage** : pour plusieurs tailles d'image (égale, plus grande, plus large que la hitbox), le
  quad calculé place l'image au bon endroit relativement à la boîte de collision. Fonction pure.
- **Une image plus grande ne déplace pas la hitbox** : la position simulée est inchangée.
- Absence de spritesheet → repli procédural, sans exception, avec la même apparence qu'aujourd'hui.
- Spritesheet aux dimensions incohérentes avec sa description → refus par le contrat d'asset, repli.

## Points d'attention
- **Le piège du double étirement.** Aujourd'hui, la région carrée est étirée par `Transform::scale`,
  et le générateur pré-compresse pour compenser. Le nouveau calcul doit rendre cette compensation
  inutile — et la retirer du générateur si c'est fait, sinon les deux se cumuleront.
- **Aucun test de gameplay ne doit changer.** Si un test de franchissabilité échoue, c'est que la
  hitbox a bougé : c'est un défaut, pas un ajustement.
- Documenter l'ancrage dans `Source/Elements/Assets/README.md` : c'est ce que l'auteur doit
  comprendre pour dessiner un personnage aligné.

## Définition de fait (DoD)
- La spritesheet externe remplace l'atlas procédural quand elle est présente, avec repli sinon ; la
  taille d'image est indépendante de la hitbox, qui est inchangée ; l'ancrage est testé et
  documenté ; `/W4 /WX` propre.

## Exigences
`EX-REN-009` (personnage depuis une spritesheet externe) ; réutilise `EX-REN-042` (assets
externalisés avec repli), `EX-REN-007` (contrat d'asset), `EX-ARCH-012` (rendu sans effet sur la
simulation), `EX-GP-018` (gabarit et ressenti du personnage), `EX-NFR-040` (repli).
