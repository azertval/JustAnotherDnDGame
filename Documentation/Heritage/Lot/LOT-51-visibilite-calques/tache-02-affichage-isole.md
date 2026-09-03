# TACHE-02 — Affichage isolé sans repli {#lot-51-tache-02-affichage-isole}

**Lot :** [LOT-51](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Masquer les autres calques ne suffit pas à auditer un calque. La priorité de résolution figée en
LOT-45 — **surcharge > skin > damier** — garantit qu'une case affiche **toujours** quelque chose.
Isoler le calque « objets interactifs » avec cette règle montrerait donc toutes les cases : celles
qui portent une surcharge, celles qui retombent sur le skin de leur type, et celles qui retombent
sur le damier.

C'est exactement l'inverse de ce qu'on cherche. Pour auditer, il faut voir **seulement ce qui est
réellement configuré sur ce calque** — quitte à ne rien voir du tout.

## Travail à réaliser
- **Mode isolé** : quand un seul calque de contenu est visible, la résolution ne retombe **pas** sur
  les niveaux de priorité inférieurs :
  - « objets interactifs seuls » → uniquement les cases portant une surcharge ; les autres restent
    vides. Ni skin, ni damier.
  - « skin des tuiles seul » → uniquement les types ayant un skin dans le jeu courant ; les types non
    skinnés restent vides. C'est le diagnostic principal du programme : **quels types restent à
    habiller**.
  - « fond seul », « décors seuls » → uniquement ce qui est configuré.
- **Un seul résolveur, deux règles d'affichage** : réutiliser le résolveur de priorité de LOT-45 avec
  un indicateur « composer » ou « isoler », plutôt que d'en écrire un second. Deux résolveurs
  divergeraient.
- **Distinction claire des deux notions** : « masqué » (le calque n'est pas affiché) et « vide »
  (le calque est affiché mais rien n'y est configuré) doivent être distinguables à l'écran — un
  quadrillage discret sur le calque isolé, par exemple, plutôt qu'un écran noir ambigu.

## Fichiers impactés
- `Source/HMI/Graphics/TileAppearance.{h,cpp}` (extension de `resolveTileAppearance`, le résolveur
  unique — pas `TileVisuals.{h,cpp}`, qui ne porte que la correspondance type de tuile → région
  d'atlas et ne connaît pas la priorité surcharge/skin/damier).
- `Source/HMI/Graphics/ComposedScene.cpp` (seul appelant de production, ajusté au retour désormais
  optionnel).
- `Source/Test/Unit/HMI/Graphics/test_texture_resolution.cpp` (étendu).

## Tests (obligatoires)
- « Objets interactifs seuls » : une case avec surcharge est affichée ; une case sans surcharge dont
  le type a un skin n'affiche **rien** ; une case sans surcharge ni skin n'affiche **rien** (pas de
  damier).
- « Skin des tuiles seul » : un type skinné est affiché ; un type non skinné n'affiche rien.
- Le résolveur en mode **composer** garde exactement le comportement de LOT-45 — les tests de
  priorité existants passent inchangés.
- Sans GPU.

## Points d'attention
- **Ne pas casser la composition en ajoutant l'isolement.** Les tests de priorité de LOT-45 sont le
  filet : ils doivent passer sans modification.
- Le mode isolé n'a de sens que pour les calques **de contenu**. Isoler le calque d'aides d'édition
  ou celui de l'interface n'apporte rien : les exclure ou l'assumer explicitement.
- Ce mode n'est **pas** un mode de rendu du jeu : `GameSession` ne doit jamais l'utiliser, même
  accidentellement via un paramètre par défaut.

## Définition de fait (DoD)
- L'affichage isolé ne montre que ce qui est configuré sur le calque, sans repli ; « masqué » et
  « vide » sont distinguables ; le résolveur reste unique et la composition inchangée ; tests sans
  GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-044` (visibilité et isolement par calque) ; réutilise `EX-EDIT-042` (skins),
`EX-EDIT-043` (surcharges, priorité inversée ici), `EX-REN-044` (fond), `EX-DEC-002` (couches de
décor), `EX-NFR-004` (vérification sans GPU).
