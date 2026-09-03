# LOT-41 — Bascule Physique/Texture (`F8`, non remappable) {#lot-41}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (fondations du rendu). Prépare
> [LOT-42](@ref lot-42) → [LOT-55](@ref lot-55).

## Objectif
Introduire la commande qui bascule, en édition **et** en jeu réel, entre le rendu **Physique**
(couleur plate par type de tuile, celui d'aujourd'hui — lecture directe des collisions) et le rendu
**Texture** (habillage complet, construit lot après lot à partir de LOT-42). Sans cette bascule,
rien de ce que les lots suivants ajoutent n'est consultable ni comparable au rendu physique
existant.

## Périmètre

### Inclus
- ***RenderMode { Physique, Texture }*** (namespace `hmi`, nom équivalent possible), propagé depuis
  `hmi::GameViewport` (widget unique derrière édition, essai et jeu réel, `EX-IHM-002`) jusqu'à
  `DraftRenderer::render` et `GameSession::render`/`SpriteRenderer::render`.
- **`F8`** géré **en dur** dans `GameViewport::keyPressEvent`, précédent direct : `Qt::Key_F10`
  (bascule de la grille de repère, déjà non remappable) — **hors** des tables remappables
  (`GameKeyBindings`/`EditorKeyBindings`) : aucune entrée dans leurs enums, leur JSON ou leur UI de
  remappage.
- **Défaut : Texture, dans toutes les configurations de build**, et **persistance** du dernier choix
  entre deux sessions via `QSettings` — le mécanisme est déjà en place pour la langue et la
  disposition des docks (`MainWindow::saveLayout`/`restoreLayout`), il n'y a pas de nouveau
  mécanisme à inventer.
- En mode Texture, **tout** s'affiche avec le repli en damier magenta (LOT-40, TACHE-03) tant
  qu'aucun skin n'existe encore — comportement honnête et attendu, pas un bug, jusqu'à LOT-42.
- Le mode Physique reste **identique** au rendu actuel, dans tous les cas, ce qui est **asserté** via
  le *QuadRecorder* (LOT-40, TACHE-04) et non constaté à l'œil.

### Exclus (hors périmètre de ce lot)
- Aucune donnée de skin, de fond, de décor ou d'objet — ce lot ne fait que câbler la bascule et le
  point de résolution déjà préparé en LOT-40.
- Aucun 3ᵉ mode (ex. superposition physique+texture) : binaire pour ce lot. Le contrôle **fin** par
  calque relève de LOT-51, et il est délibérément distinct.

## Décisions de cadrage
- **`GameViewport` unique** porte la bascule : couvre automatiquement édition, essai et jeu réel
  sans double implémentation (`EX-IHM-002`).
- **Touche fixe, jamais remappable** : cohérent avec `F10` déjà non remappable ; une bascule de
  rendu n'est pas une action de gameplay remappable au même titre que sauter ou courir.
- **Un seul défaut, dans toutes les configurations** — décision **révisée** par rapport au cadrage
  initial, qui prévoyait Physique en Debug et Texture en Release via `core::kDeveloperBuild`. Trois
  raisons : le développeur qui construit le mode Texture ne le verrait jamais par défaut ; deux
  binaires du même commit afficheraient un rendu différent, rendant ambiguë toute capture d'écran,
  vérification visuelle ou remontée de bug ; et couplé au damier magenta généralisé, cela livrerait
  une Release entièrement magenta pendant tout LOT-41. `F8` reste évidemment actif partout.
- **Persistance retenue** — décision également **révisée**. Le cadrage initial l'écartait « par
  simplicité », alors que `QSettings` est déjà câblé et que le coût réel est marginal ; à l'usage,
  reperdre son mode d'affichage à chaque lancement est une friction quotidienne pendant treize lots.
- **Résolution de calque non concernée** : ce lot introduit uniquement le drapeau de mode ; le point
  de résolution « surcharge > skin > damier » n'a encore qu'une seule branche active (damier), les
  suivantes arrivent lot après lot sans changer la forme de ce résolveur.

## Exigences couvertes
- Amendée : `EX-REN-046` (bascule Physique/Texture, `F8`, non remappable, défaut Texture unique et
  persistance du choix).
- Réutilisées : `EX-REN-043` (rendu multi-textures, LOT-40), `EX-NFR-040` (repli), `EX-IHM-002`
  (viewport unique), `EX-NFR-004` (vérification sans GPU), `EX-IHM-011` (persistance des préférences
  d'interface).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-render-mode.md) | *RenderMode* + câblage `GameViewport` → `DraftRenderer`/`GameSession` | `Source/HMI/Game`, `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-touche-f8-persistance.md) | `F8` en dur hors tables de remappage ; défaut Texture + persistance `QSettings` | `Source/HMI/Game`, `Source/HMI/Interface` | ✅ |
| [TACHE-03](tache-03-non-regression-documentation.md) | Non-régression du mode Physique (assertions *QuadRecorder*) + documentation | `Source/Test`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. `F8` bascule le rendu en édition, en essai et en jeu réel, sans passer par les tables de
   remappage.
2. Le mode Physique produit **la même liste de primitives** qu'avant ce lot, assertée sans GPU.
3. Le défaut au lancement est Texture dans **toutes** les configurations de build ; le dernier choix
   est restitué au lancement suivant.
4. Aucune entrée `F8` n'apparaît dans les enums, le JSON ou l'UI de remappage.
5. Build `/W4 /WX`, Doxygen et lint verts ; bascule vérifiée visuellement dans les trois contextes
   (édition, essai, jeu réel).

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (calques, résolveur de texture, *QuadRecorder*). Ne modifie pas
`Core`. Prérequis de [LOT-42](@ref lot-42) à [LOT-55](@ref lot-55).

## Navigation des tâches
- @subpage lot-41-tache-01-render-mode
- @subpage lot-41-tache-02-touche-f8-persistance
- @subpage lot-41-tache-03-non-regression-documentation
