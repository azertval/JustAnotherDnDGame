# LOT-54 — Atelier pixel art intégré {#lot-54}

> Statut : **fait** (deux points d'entrée/fonctionnalités secondaires non livrés, cf. amendements
> de TACHE-05 et TACHE-08). Prérequis : [LOT-42](@ref lot-42) (convention `Assets/Skins/`, planches
> à raccords), [LOT-43](@ref lot-43) (bibliothèque et rechargement à chaud),
> [LOT-56](@ref lot-56) (jetons de design, style maîtrisé, actions et barre d'outils, netteté à
> toute échelle) et [LOT-57](@ref lot-57) (barre d'état structurée, regroupement des panneaux,
> déduplication des commandes).

## Objectif
Permettre de créer et modifier les fichiers d'assets de texture (skins, planches, fonds, objets,
décors, images d'animation) **sans quitter l'application**, avec un canevas pixel art — et en voyant
immédiatement le résultat dans le niveau.

Ce lot était initialement cadré comme un premier jet minimal, avant que [LOT-56](@ref lot-56) et
[LOT-57](@ref lot-57) n'existent. Il s'exécute désormais **derrière eux** et bâtit sur ce qu'ils
livrent : le canevas n'a plus à inventer son habillage, ses commandes ni son affichage d'état, et
le périmètre récupère en fonctions d'édition ce qu'il n'a plus à dépenser en plomberie d'interface.

## Périmètre

### Inclus
- **Canevas** `QWidget` + `QImage` + `QPainter` : peindre et effacer au pixel près (grille alignée),
  **pot de peinture** (remplissage par zone contiguë), pipette, zoom, annuler/refaire **local** —
  propre au canevas, totalement indépendant de l'historique de `LevelDraft`.
- **Commandes exposées comme actions** (`EX-IHM-055`, LOT-56) : les outils du canevas forment un
  groupe exclusif dans une **barre d'outils**, avec des icônes dessinées par code selon le patron de
  `hmi::ThemeIcons`. Aucun bouton radio empilé.
- **Annuler et Refaire uniques, à cible contextuelle** : une seule paire d'actions, dont la cible
  est le contexte d'édition actif — le niveau ou le canevas. Un état, un contrôle (`EX-IHM-062`).
- **État permanent dans la barre d'état** : l'asset ouvert, son état modifié, l'outil de canevas
  actif, le pixel survolé, le zoom et la couleur courante s'affichent via le modèle de décision pur
  livré en LOT-57 (`EX-IHM-060`), **étendu** d'un contexte d'édition d'asset — pas doublé.
- **Netteté et habillage issus du socle** : le canevas se dimensionne selon l'échelle d'affichage
  réelle (`EX-IHM-053`) et tire toutes ses couleurs des jetons (`EX-IHM-051`) ; aucune constante de
  style locale.
- **Outils de région** : sélection rectangulaire, déplacement de la région sélectionnée, symétrie
  horizontale et verticale, rotation par quart de tour, copier et coller.
- **Palettes persistées** : palette de projet enregistrée sur disque, extraction de la palette d'un
  asset ouvert, et mode « contraindre à la palette » qui interdit de poser une couleur hors palette.
- **Historique visuel** : un panneau listant les opérations nommées de la session d'édition, avec
  retour à un point antérieur.
- **Ouvrir, créer, enregistrer** : ouvrir un asset existant depuis la bibliothèque (LOT-43) ou depuis
  le panneau Textures recentré (LOT-57), ou en créer un à une taille choisie parmi celles admises par
  le contrat d'asset (LOT-40) ; enregistrer via un encodeur d'image **symétrique** de
  `hmi::decodeImageFile` (LOT-39).
- **Aperçu live** : pendant l'édition, le niveau affiché reflète les modifications, en s'appuyant sur
  `TextureCache::invalidate` (API prévue dès LOT-40). C'est ce qui distingue un atelier intégré d'un
  éditeur d'images lancé à côté.
- **Mode planche à raccords** : lors de l'édition d'un asset en mode `bitmask16` (LOT-42), afficher
  les repères des seize cases et l'indication de ce que chaque case représente (bord haut, coin
  concave, intérieur…), ainsi qu'un **aperçu de raccord** montrant un assemblage 3×3 construit avec
  la planche en cours. Dessiner une planche cohérente sans repères est une source d'erreurs garantie.
- **Garde-fou d'écrasement** : enregistrer par-dessus un asset référencé par des niveaux ou par un
  jeu de skins demande confirmation, en nommant les références concernées (détection déjà livrée en
  LOT-43).
- Toute UI passe par le catalogue de traduction.

### Exclus (hors périmètre de ce lot)
- **Calques dans le canevas**, dégradés, anticrénelage : sans objet pour du pixel art de tuiles
  16×16, et coûteux — extensions possibles dans un lot ultérieur si le besoin se confirme à l'usage.
  Les transformations, elles, **entrent** dans le périmètre (TACHE-06) : ce sont des fonctions pures
  sur un tampon, exposées comme des actions, donc peu coûteuses une fois le socle du LOT-56 posé.
- Édition d'une animation image par image avec prévisualisation temporelle : le canevas édite une
  spritesheet comme une image ; l'affectation des clips relève de LOT-46/47.
- Conversion photo → pixel art (`EX-EDIT-041`, `EX-DEC-030` à `EX-DEC-032`) : hors programme.
- Toute intégration d'une bibliothèque externe (cf. décision de cadrage).
- **Réorganisation du panneau Textures** et **découpage de *TexturePanel*** : LOT-57 s'en charge ;
  l'atelier s'y branche par un point d'entrée, sans le restructurer une seconde fois.

## Décisions de cadrage
- **L'ordre d'exécution est le vrai levier de ce lot.** Exécuté avant [LOT-56](@ref lot-56) et
  [LOT-57](@ref lot-57), l'atelier aurait livré un sixième panneau au rendu natif garni de boutons
  radio empilés, une seconde paire de `Ctrl+Z` en conflit avec celle du niveau, un quatrième chemin
  de vignettes floues à l'échelle 125 %, et un second modèle d'affichage d'état — soit précisément
  les quatre défauts que ces deux lots corrigent, à retoucher ensuite deux fois. Exécuté derrière,
  son habillage est essentiellement gratuit, et le budget ainsi libéré finance les outils de région,
  les palettes et l'historique visuel. Le numéro `LOT-54` est **conservé** : la règle de
  [`lots.md`](../lots.md) veut qu'un lot planifié garde son identifiant même si son ordre d'exécution
  change.
- **Le fond du canevas et le damier de transparence appartiennent à la portée invariante.** LOT-56
  scinde l'habillage en une part invariante (menu principal, Options, jeu) et une part variable (le
  châssis d'édition, qui suit le réglage clair/sombre du système). Le canevas tombe des deux côtés :
  son châssis — barre d'outils, panneaux, barre d'état — suit le thème, mais sa **surface de
  peinture** n'en dépend pas. Un fond qui change de clarté selon l'heure de la journée fausse la
  perception des couleurs posées, ce qui est rédhibitoire pour l'outil dont c'est justement le sujet.
  C'est la première décision de cadrage que l'existence d'un thème clair rend nécessaire.
- **Annuler et Refaire sont une action unique à cible contextuelle**, pas deux commandes homonymes
  arbitrées par le focus. `hmi::EditorKeyBindings` définit déjà *annuler*, *refaire*, *copier* et
  *coller* comme actions remappables sans jamais les lire ; LOT-57 les branche enfin, et LOT-54 leur
  donne une seconde cible. Le libellé de l'action nomme l'opération qui sera annulée, ce qui lève
  l'ambiguïté à l'écran plutôt que dans la documentation.
- **Développement interne, pas de bibliothèque externe.** La licence du projet (« tous droits
  réservés ») est incompatible avec les éditeurs pixel art matures existants (Aseprite,
  LibreSprite — tous deux de lignée GPL) ; il n'existe pas de bibliothèque Qt/C++ légère et
  **permissive** équivalente pour ce besoin précis. Le périmètre réel repose entièrement sur des
  primitives déjà en place (Qt Widgets depuis LOT-34, *TextureLoader*/`hmi::AssetPaths` depuis
  LOT-39, jetons et actions depuis LOT-56) — cohérent avec la politique du projet de préférer Qt à
  une dépendance tierce.
- **Pot de peinture inclus, explicitement** : le cadrage initial excluait « le remplissage au-delà du
  seau de peinture simple », formulation ambiguë qui laissait ouvert le statut du seau lui-même.
  Tranché : remplissage par zone contiguë inclus.
- **Aperçu live plutôt qu'aller-retour** : sans lui, l'atelier n'apporterait rien de plus qu'un
  éditeur d'images externe, et le programme aurait payé un lot pour une commodité.
- **Huit tâches, avec un ordre de resserrement annoncé.** Le lot dépasse le grain habituel ; si son
  volume doit être réduit, l'ordre de retrait est : **TACHE-07** (palettes), puis **TACHE-06**
  (outils de région), puis le panneau d'historique de la **TACHE-04**. Aucun de ces retraits
  n'invalide le reste, et le socle des tâches 01 à 05 plus 08 correspond au périmètre initialement
  cadré, habillage compris.
- **Amende le non-objectif « pas d'édition d'assets graphiques »** de
  [`editeur-niveaux.md`](../../Specification/editeur-niveaux.md) (section 5) de façon ciblée :
  l'éditeur de niveaux continue d'agencer des tuiles existantes sans générer de sprites procéduraux ;
  seul ce canevas dédié édite les fichiers d'assets eux-mêmes.

## Exigences couvertes
- Amendée : `EX-EDIT-045` (outil de dessin pixel art intégré, avec aperçu du rendu dans le niveau).
- Réutilisées, issues du socle d'habillage : `EX-IHM-050` (thème couvrant l'IHM, en deux portées),
  `EX-IHM-051` (source unique des grandeurs), `EX-IHM-053` (netteté à toute échelle d'affichage),
  `EX-IHM-055` (commandes exposées comme actions), `EX-IHM-060` (état de travail affiché en
  permanence), `EX-IHM-062` (un état ou une commande à un seul endroit).
- Réutilisées : `EX-EDIT-013` (déplacement et zoom), `EX-EDIT-021` (garde-fous contre la perte de
  travail), `EX-EDIT-024`/`EX-EDIT-025` (jeux de skins, planches à raccords), `EX-EDIT-026` (gestion
  des fichiers d'assets), `EX-REN-005` (animations par données), `EX-REN-007` (contrat d'asset),
  `EX-REN-033` (traduction), `EX-REN-041` (décodage image), `EX-ARCH-022` (pixel art net),
  `EX-NFR-010` (testable sans GPU), `EX-NFR-040` (erreur récupérable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-encodage-image.md) | Encodage et enregistrement d'image, symétrique de `decodeImageFile` | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-operations-historique.md) | Opérations pures sur tampon de pixels et historique à opérations nommées | `Source/HMI/Editor` | ✅ |
| [TACHE-03](tache-03-canevas.md) | Canevas : rendu net à toute échelle, couleurs issues des jetons, surface invariante | `Source/HMI/Editor` | ✅ |
| [TACHE-04](tache-04-actions-barre-outils.md) | Outils en actions, barre d'outils, annuler/refaire contextuel, barre d'état, historique visuel | `Source/HMI/Interface`, `Source/HMI/Editor` | ✅ |
| [TACHE-05](tache-05-ouvrir-enregistrer.md) | Ouvrir, créer, enregistrer + garde-fous + point d'entrée depuis la bibliothèque et le panneau Textures | `Source/HMI/Editor` | ✅ (point d'entrée Textures non câblé, cf. amendement) |
| [TACHE-06](tache-06-outils-region.md) | Outils de région : sélection, déplacement, symétries, rotations, copier/coller | `Source/HMI/Editor` | ✅ |
| [TACHE-07](tache-07-palettes.md) | Palettes persistées, extraction depuis un asset, mode contraint à la palette | `Source/HMI/Editor`, `Source/Elements` | ✅ |
| [TACHE-08](tache-08-apercu-live-planche.md) | Aperçu live, mode planche à raccords, aperçu de raccord et d'animation | `Source/HMI/Editor`, `Source/HMI/Graphics` | ✅ (aperçu d'animation non livré, cf. amendement) |

## Critères d'acceptation du lot
1. Créer, peindre et enregistrer un nouvel asset PNG depuis le canevas, puis le retrouver assignable
   dans le panneau « Textures » sans redémarrer.
2. Modifier un asset existant et l'enregistrer préserve ses dimensions.
3. Le niveau affiché reflète les modifications en cours d'édition.
4. L'édition d'une planche à raccords affiche les repères des seize cases, leur rôle, et un aperçu
   d'assemblage 3×3 construit avec la planche en cours.
5. Enregistrer par-dessus un asset référencé demande confirmation en nommant les références.
6. L'annuler/refaire du canevas est fonctionnel et **indépendant** de celui de `LevelDraft` :
   annuler un coup de pinceau n'annule jamais une action d'édition de niveau, et l'action affiche le
   nom de l'opération qu'elle annulera.
7. Le changement d'outil du canevas se fait depuis une barre d'outils à icônes ; aucun bouton radio
   empilé n'est ajouté par ce lot.
8. Le canevas et ses repères restent nets à 100 %, 125 % et 150 % d'échelle d'affichage, sans
   lissage du pixel art.
9. La surface de peinture — fond et damier de transparence — a exactement la même apparence en thème
   clair et en thème sombre, tandis que le châssis autour d'elle suit le thème.
10. Une région sélectionnée peut être déplacée, retournée et pivotée ; deux symétries successives sur
    le même axe restituent l'image d'origine.
11. La palette de projet est retrouvée après un redémarrage, et le mode contraint n'admet aucune
    couleur absente de la palette.
12. Pendant l'édition d'un asset, la barre d'état montre en permanence l'asset ouvert, son état
    modifié, l'outil actif, le pixel survolé et le zoom.
13. Le panneau d'historique liste les opérations nommées et permet de revenir à un point antérieur.
14. Build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-42](@ref lot-42) (convention de dossiers, planches à raccords) et
[LOT-43](@ref lot-43) (bibliothèque, détection de références, rechargement à chaud). Réutilise
*TextureLoader*/`hmi::AssetPaths` (LOT-39) et `TextureCache::invalidate` ([LOT-40](@ref lot-40)).
Dépend désormais de [LOT-56](@ref lot-56) — jetons, palette applicative, actions, icônes dessinées
par code, dimensionnement à l'échelle d'affichage — et de [LOT-57](@ref lot-57) — modèle pur de la
barre d'état, table de mise en avant des panneaux, actions d'éditeur remappables enfin branchées, et
panneau Textures recentré qui accueille son point d'entrée.

## Navigation des tâches
- @subpage lot-54-tache-01-encodage-image
- @subpage lot-54-tache-02-operations-historique
- @subpage lot-54-tache-03-canevas
- @subpage lot-54-tache-04-actions-barre-outils
- @subpage lot-54-tache-05-ouvrir-enregistrer
- @subpage lot-54-tache-06-outils-region
- @subpage lot-54-tache-07-palettes
- @subpage lot-54-tache-08-apercu-live-planche
