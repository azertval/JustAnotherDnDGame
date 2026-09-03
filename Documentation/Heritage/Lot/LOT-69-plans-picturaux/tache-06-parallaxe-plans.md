# TACHE-06 — Parallaxe portée par le plan {#lot-69-tache-06-parallaxe-plans}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/{Graphics,Game}` ·
**Statut :** fait

## Contexte
`Source/HMI/Graphics/Parallax.h` fige trois constantes — `0.5`, `1.0`, `1.15` — indexées par la
couche de décor, et dépend donc de `Core/Levels/Decor.h`. La formule elle-même est bonne et le reste :
`rendu = centre + (position − centre) × facteur`, où `centre` est celui de la **salle courante** et
non l'origine du niveau. C'est ce qui empêche le décor de **sauter** à chaque bascule de salle
(`EX-REN-015`) : le décalage se replace au moment exact où toute l'image change déjà.

Un décalage **absolu** en espace niveau a été envisagé au cadrage, au motif qu'un plan continu n'a
pas de repère ponctuel. Il a été écarté : la formule relative reste correcte pour un plan, et la
remplacer rouvrirait précisément le défaut qu'elle corrige.

## Travail à réaliser
- Retirer `PARALLAX_FACTOR_*`, `parallaxFactor(DecorLayer)` et l'inclusion de `Decor.h`.
- Généraliser `parallaxRenderPosition` / `parallaxModelPosition` à un facteur **par axe**
  (`core::Vector2`).
- Ajouter `clampPlaneOffset(offset, planeBounds, cameraBounds)` — fonction **pure**. Une translation
  découvre le bord d'un plan qui couvre exactement le niveau : sans bornage, une bande vide apparaît.
  L'alternative « que l'artiste peigne des marges » est un piège, la marge nécessaire dépendant du
  facteur *et* de la taille de salle — personne ne la calculera juste.
- Ajouter `planeParallaxActive(CameraFramingMode, bool levelFlag)` — **table pure** portant la
  décision, plutôt qu'une condition disséminée dans `GameSession` :

  | Mode | Parallaxe | Motif |
  |---|---|---|
  | `Follow` | **active** | Seul mode où la caméra défile : le seul où la parallaxe se lit |
  | `PerRoom` | **active** | Formule relative à la salle, sans saut (`EX-REN-015`) |
  | `WholeLevel` | **neutralisée** | La caméra ne bouge pas : l'offset serait une constante, donc un simple désalignement du plan par rapport aux tuiles |

- Supprimer `applyDecorParallax` de `GameSession` au profit de cette table.
- **Neutraliser la parallaxe en mode création** : sinon la correspondance curseur ↔ pixel dépendrait
  de la position de caméra et le 1:1 promis serait faux.

## Fichiers impactés
`Source/HMI/Graphics/Parallax.{h,cpp}`, `Source/HMI/Graphics/PlaneVisuals.cpp`,
`Source/HMI/Game/GameSession.cpp`, `Source/HMI/Game/GameViewport.cpp`.

## Tests (obligatoires)
- `test_parallax.cpp` réécrit : facteur `1.0` → position **inchangée** ; `parallaxModelPosition`
  inverse exact de `parallaxRenderPosition` ; **clamp** — un plan à facteur `0.5`, caméra en bord de
  niveau, couvre toujours le cadrage ; `roundToScreenPixel` appliqué **après** le clamp ; table
  `planeParallaxActive` **exhaustive** sur les trois modes × deux valeurs du drapeau.

## Points d'attention
La réactivation en mode **suivi** inverse une décision du `LOT-64`. C'est un changement de
comportement **visible** : il s'assume dans l'epic et le `CHANGELOG`, il ne se glisse pas dans un
refactor. Le motif est net — le `LOT-64` avait coupé la parallaxe parce qu'un **décor** est un objet
collé au contenu du niveau et paraissait « suivre » la caméra ; un plan est un fond, l'argument
tombe.

Le drapeau est **par niveau**, mais la neutralisation en `WholeLevel` est une règle du moteur. Un
level designer cochera « parallaxe » sur un niveau `WholeLevel` et ne verra rien : l'interface doit
**griser la case dans ce mode et l'expliquer** (`TACHE-08`), sinon c'est un défaut perçu.

L'arrondi au pixel écran reste indispensable : un décalage fractionnaire rend le pixel art flou ou
tremblant.

## Definition de fait (DoD)
Plus aucune constante de parallaxe en dur, facteur porté par le plan, plan toujours couvrant, table
de comportement testée exhaustivement. `ctest` à 100 %.

## Exigences
`EX-DEC-043`, `EX-REN-015`, `EX-REN-016`, `EX-ARCH-012`, `EX-ARCH-022`, `EX-NFR-010`.
