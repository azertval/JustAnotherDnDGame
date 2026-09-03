# LOT-51 — Mode « définition des textures » : visibilité par calque {#lot-51}

> Statut : **fait**. Prérequis : [LOT-41](@ref lot-41) (plomberie de mode),
> [LOT-44](@ref lot-44) (fond), [LOT-45](@ref lot-45) (objets), [LOT-49](@ref lot-49) (décors).

## Objectif
Livrer l'outil d'inspection du mode design : voir, dans l'éditeur, **ce qui a été configuré sur
chaque calque** à un endroit précis.

Ce lot est **délibérément distinct** de la bascule `F8` (LOT-41). `F8` **compose** le rendu final
(surcharge > skin > damier) ; ce mode **décompose**, pour auditer. Il n'est jamais exposé au joueur.

Il arrive tardivement dans le programme à dessein : un outil d'audit des calques n'a de valeur que
lorsque les calques existent réellement — fond (LOT-44), skins (LOT-42), objets (LOT-45), décors sur
trois couches (LOT-49), personnage (LOT-48).

## Périmètre

### Inclus
- **Contrôle de visibilité par calque**, éditeur uniquement : une case à cocher par calque
  pertinent — Fond, Décor d'arrière-plan, Skin des tuiles, Objets interactifs, Personnage, Décor de
  premier plan — chacune activable et désactivable indépendamment.
- **Affichage isolé sans repli** : lorsqu'un seul calque est visible, ce calque n'affiche **que** ce
  qui y est réellement configuré. En particulier, l'affichage « objets interactifs seuls » ne
  retombe **ni** sur le skin du type, **ni** sur le damier magenta : le but est d'auditer ce calque,
  pas le rendu composé. C'est l'inversion délibérée de la priorité de résolution figée en LOT-45.
- **État « Physique seul »** : le rendu en couleurs plates existant, proposé comme une visibilité
  parmi les autres plutôt qu'un renderer séparé.
- Réutilisation du même résolveur que le rendu composé — deux règles d'affichage, un seul résolveur.
- Toute UI de ce mode passe par le catalogue de traduction.

### Exclus (hors périmètre de ce lot)
- Tout ce qui touche au jeu réel ou au mode essai : éditeur uniquement, aucun effet sur
  `GameSession`.
- Modification des données depuis ce mode : c'est une vue, pas un outil d'édition.
- Persistance des cases cochées entre deux sessions.

## Décisions de cadrage
- **Cases de visibilité plutôt qu'un contrôle à trois états** — décision **révisée** par rapport au
  cadrage initial, qui prévoyait un sélecteur « Fond seul / Physique seul / Interactif seul ». À coût
  d'implémentation identique, des cases indépendantes offrent les états isolés **et** toutes leurs
  combinaisons, ce qui est le vrai besoin d'audit (« le décor de premier plan cache-t-il quelque
  chose d'important ? » suppose deux calques visibles, pas un).
- **Le calque « skin des tuiles » est réintégré**, alors que le cadrage initial l'excluait
  explicitement. C'est pourtant le diagnostic le plus utile du programme : quels types de tuiles
  n'ont pas encore de skin dans le jeu courant.
- **Distinct de `F8` par construction**, à rendre évident dans l'UI (libellés « Aperçu » contre
  « Jeu ») pour que les deux ne soient jamais confondus.
- **Aucun repli en mode isolé** : un calque vide doit se voir vide. Afficher un repli reviendrait à
  masquer précisément l'information recherchée.
- **`RenderLayer::Object` prend son sens à ce lot** — décision prise à l'implémentation, non prévue
  au cadrage initial : ce calque était réservé mais resté inutilisé (`LOT-45` garde les objets
  interactifs sur `RenderLayer::Tile`, un override étant une propriété de n'importe quelle case, pas
  d'un calque à part). `hmi::LayerVisibility` réutilise donc son bit comme **axe de résolution** —
  « la surcharge par instance doit-elle s'afficher ? » — distinct du bit `RenderLayer::Tile` — « le
  skin de type doit-il s'afficher ? » — plutôt que d'ajouter une structure parallèle qui aurait
  contredit « indexé par `RenderLayer`, pas par une liste écrite à la main ». Les deux entités
  restent sur `RenderLayer::Tile` pour l'ordre de dessin, inchangé.

## Exigences couvertes
- Amendée : `EX-EDIT-044` (visibilité par calque : isolement et combinaisons).
- Réutilisées : `EX-REN-043` (rendu multi-calques), `EX-REN-044` (fond), `EX-EDIT-042` (skins),
  `EX-EDIT-043` (objets interactifs), `EX-DEC-002` (couches de décor), `EX-REN-046` (distinct de la
  bascule), `EX-REN-033` (traduction), `EX-NFR-004` (vérification sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-visibilite-calques.md) | Jeu de visibilités par calque + câblage `GameViewport`/`DraftRenderer` (éditeur uniquement) | `Source/HMI/Game`, `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-affichage-isole.md) | Affichage isolé sans repli (objets interactifs seuls, skin seul) | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-controles-documentation.md) | Contrôles d'interface traduits + documentation | `Source/HMI/Editor`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Chaque calque peut être affiché ou masqué indépendamment, y compris en combinaisons.
2. L'affichage « objets interactifs seuls » n'affiche **que** les cases portant une surcharge —
   jamais de repli skin ni de damier — asserté via le *QuadRecorder*.
3. L'affichage « skin des tuiles seul » permet d'identifier les types sans skin dans le jeu courant.
4. Aucun effet sur `GameSession`, le mode essai ou le jeu réel.
5. Les libellés distinguent sans ambiguïté ce mode d'aperçu de la bascule `F8`.
6. Build `/W4 /WX`, Doxygen, lint verts ; vérification manuelle des combinaisons principales.

## Dépendances
Bâtit sur [LOT-41](@ref lot-41) (plomberie de mode), [LOT-42](@ref lot-42) (skins),
[LOT-44](@ref lot-44) (fond), [LOT-45](@ref lot-45) (objets), [LOT-48](@ref lot-48) (personnage),
[LOT-49](@ref lot-49) et [LOT-50](@ref lot-50) (décors). Éditeur uniquement.

## Navigation des tâches
- @subpage lot-51-tache-01-visibilite-calques
- @subpage lot-51-tache-02-affichage-isole
- @subpage lot-51-tache-03-controles-documentation
