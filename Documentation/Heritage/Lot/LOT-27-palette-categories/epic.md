# LOT-27 — Palette de l'éditeur organisée par catégories {#lot-27}

> Statut : **terminé**. La palette de tuiles de l'éditeur passe d'une **liste plate** de 19 types
> à un **accordéon à trois niveaux** (`EX-EDIT-018`) : catégories repliables, sous-groupes
> repliables imbriqués pour les familles à plusieurs formes/tailles (Pente, Arrondi, Bloc
> poussable), puis les variantes elles-mêmes.

## Objectif
Depuis `LOT-26`, la palette affiche 19 lignes à plat : huit types simples (Vide, Solide, Danger,
Entrée, Sortie, Interrupteur, Plaque, Porte) noyés au milieu de huit variantes de pente/arrondi et
trois tailles de bloc poussable, dont les libellés ne se distinguent que par une orientation ou une
fraction. Repérer le bon type impose un défilement visuel de la liste entière à chaque clic. Ce lot
réorganise la palette par **catégories** (regroupement par intention d'usage) avec un second niveau
de repli pour les familles à formes multiples, sans jamais recouvrir le canevas d'édition (esprit
du panneau latéral fixe posé par `LOT-15`).

Un mockup HTML/CSS interactif a été construit et itéré avec le demandeur **avant** toute
implémentation (position du dépliage, choix des quatre catégories, retrait des textes de
description, ajout du troisième niveau d'accordéon) — l'implémentation reproduit fidèlement la
structure validée sur ce mockup, adaptée aux contraintes réelles (police bitmap, panneau de largeur
fixe, pas de rendu de survol/animation).

## Périmètre

### Inclus
- Réorganisation de `hmi::TilePalette` en accordéon à trois niveaux :
  1. Deux entrées **autonomes** toujours visibles (Vide, Piège — ex-Danger, renommé) et trois
     **catégories** repliables (Tuile, Interactif, Jalon).
  2. Une catégorie dépliée expose ses tuiles **directes** (ex. Porte, Plaque, Interrupteur pour
     Interactif) et, pour Tuile/Interactif, des **sous-groupes** repliables (Pente, Arrondi pour
     Tuile ; Bloc poussable pour Interactif).
  3. Un sous-groupe déplié expose ses variantes (quatre orientations pour Pente/Arrondi, trois
     tailles pour Bloc poussable).
- Hauteur de panneau **dynamique** : `TilePalette::bottom()` remplace le compte fixe
  (`EditorLayout::PALETTE_TYPE_COUNT`/`TOOLBAR_TOP`, supprimés) ; `ToolBar::relayout(top)` se
  repositionne chaque frame juste sous la palette, quel que soit son état de dépliage courant.
- **Défilement** de la palette (molette au-dessus du panneau latéral) et barre de défilement
  (piste + curseur), sur le modèle de `LevelPicker` (`LOT-15`) : tout déplier en même temps (25
  lignes au maximum) peut dépasser la hauteur de fenêtre disponible, une fois `ToolBar` réservée en
  dessous. Un en-tête replié/déplié reste **toujours visible** après son propre clic (la palette
  défile automatiquement pour le garder à l'écran, comme `LevelPicker` suit sa sélection au
  clavier) — sans quoi déplier une catégorie proche du bas de la liste pourrait faire disparaître
  son propre en-tête (ou un autre plus bas) derrière le bord de l'écran, sans aucun moyen évident
  de le retrouver.
- Renommage d'affichage « Danger » → « Piège » (libellé seulement ; le type `core::TileType::Danger`
  et son identifiant JSON restent inchangés — aucune migration de niveau nécessaire).

### Exclus (hors périmètre de ce lot)
- **Aucun nouveau type de tuile** — pure réorganisation de la palette existante (19 types
  inchangés), aucun impact sur `Core` (gameplay, sérialisation, physique).
- **Palette pilotée par un fichier de configuration externe** — non-objectif déjà acté
  (`editeur-niveaux.md`, section 5) ; la structure des catégories reste codée, cohérente avec une
  liste de types qui change rarement.
- **Recherche/filtre textuel dans la palette** — au-delà du besoin exprimé (regroupement visuel
  suffit à la taille actuelle de la liste, 19 types).
- **Persistance de l'état de dépliage** entre deux sessions d'édition — repart toujours replié à
  l'ouverture, cohérent avec l'absence de préférences utilisateur ailleurs dans l'éditeur.

## Décisions de cadrage
- **Mockup avant code** : trois itérations avec le demandeur (position du panneau déplié — flottant
  au centre puis accordéon juste sous la catégorie cliquée ; choix des quatre catégories exactes
  Tuile/Interactif/Piège/Jalon plutôt qu'une liste plus fine ; retrait des textes de description par
  variante, jugés superflus une fois l'icône et le nom en place) avant qu'une seule ligne de
  l'implémentation réelle ne soit écrite — décision explicite du demandeur (« avant de te lancer
  dans l'implémentation travaillons sur un mockup »).
- **Troisième niveau d'accordéon retenu après un premier jet à deux niveaux** : la première version
  du mockup dépliait Pente/Arrondi/Bloc poussable directement (liste à plat) dès que leur catégorie
  parente était ouverte. Demande explicite d'un niveau de repli supplémentaire pour ces
  sous-groupes, afin qu'ouvrir « Tuile » ou « Interactif » ne révèle pas d'emblée 4 à 8 lignes
  supplémentaires.
- **Une entrée d'en-tête (catégorie ou sous-groupe) porte un type de tuile représentatif**, utilisé
  uniquement pour son icône (ex. l'en-tête « Tuile » affiche l'icône de `Solid`) — pas une nouvelle
  voie de rendu dans `EditorScreen` : `Entry` garde exactement la même forme qu'avant ce lot
  (`type`/`x`/`y`/`width`/`height`/`label`), `EditorScreen::renderPalette` est **inchangé**. Un
  effet de bord accepté : si le type représentatif d'un en-tête replié est la sélection courante,
  l'en-tête reçoit la même surbrillance qu'une feuille sélectionnée — indicateur utile (« la
  sélection active se trouve dans cette catégorie »), pas un bug.
- **Dépliage/repliage signalé par un préfixe textuel** (`> `/`v `) plutôt qu'une icône de chevron
  dédiée — la police bitmap ne gère que de l'ASCII imprimable en chasse fixe
  (`HMI/Graphics/BitmapFont`), un glyphe de chevron aurait demandé une police ou un atlas dédié
  pour un gain cosmétique mineur.
- **`TilePalette` reste une logique pure** (aucune dépendance de rendu), dans l'esprit déjà établi
  par `ToolBar` : seule la géométrie écran et l'état de dépliage/sélection lui appartiennent,
  testable sans GPU (`EX-NFR-010`).
- **Gap découvert après la livraison initiale de TACHE-01** (revue : « rajouter un scroll barre
  dans le menu, n'est pas complet ») : sans défilement, un dépliage cumulé dépassant la hauteur de
  fenêtre rendait les dernières entrées **définitivement inaccessibles** à la souris (aucune ligne
  ne dépasse `entries()`, donc rien à cliquer en dehors de la fenêtre) — corrigé en ajoutant le
  défilement (molette + barre) décrit ci-dessus, complété par le suivi automatique d'un en-tête
  qui vient d'être replié/déplié (`TilePalette::followRow`), sans quoi le correctif aurait
  simplement déplacé le problème (déplier une catégorie aurait pu faire disparaître son propre
  en-tête plutôt que la solution).

## Exigences couvertes
- `EX-EDIT-018` — nouvelle exigence, implémentée.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-palette-accordeon.md) | Palette en accordéon à trois niveaux | `HMI/Editor` | ✅ |
| [TACHE-02](tache-02-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. À l'ouverture de l'éditeur, seules cinq entrées sont visibles : Vide, Tuile, Interactif, Piège,
   Jalon — toutes catégories repliées.
2. Déplier une catégorie révèle ses tuiles directes et, le cas échéant, ses sous-groupes (encore
   repliés) ; déplier un sous-groupe révèle ses variantes. Un second clic sur un en-tête déplié le
   replie, masquant à nouveau tout son contenu (y compris un sous-groupe resté déplié).
3. Cliquer une entrée-feuille (autonome ou nichée dans une catégorie/un sous-groupe déplié)
   sélectionne son type de tuile, comme avant ce lot.
4. La barre d'outils reste **juste sous** la palette quel que soit son état de dépliage courant,
   sans jamais se superposer à elle ni laisser un vide incohérent.
5. Aucune régression fonctionnelle : les 19 types restent tous accessibles et sélectionnables, le
   canevas d'édition n'est jamais recouvert (`EX-EDIT-015`).
6. Un dépliage cumulé dépassant la hauteur de fenêtre reste entièrement accessible : une barre de
   défilement apparaît, la molette au-dessus du panneau fait défiler la palette (sans zoomer la
   caméra), et déplier/replier un en-tête ne le fait jamais disparaître de la fenêtre visible.
7. Logique nouvelle couverte par des tests. Build `/W4 /WX` sans avertissement, Doxygen et lint des
   exigences verts.

## Dépendances
- Étend `hmi::TilePalette`/`hmi::ToolBar`/`hmi::EditorLayout` (`LOT-14`/`LOT-15`) — pure
  réorganisation de l'existant, aucune dépendance nouvelle sur `Core`.

## Navigation des tâches
- @subpage lot-27-tache-01-palette-accordeon
- @subpage lot-27-tache-02-documentation-verification
