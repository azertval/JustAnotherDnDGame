# LOT-44 — Fond de niveau et versionnement du format {#lot-44}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40), [LOT-41](@ref lot-41),
> [LOT-42](@ref lot-42) (panneau « Textures », jeux de skins).

## Objectif
Permettre d'associer une image de **fond** à un niveau donné, dessinée en dessous de tout le reste
en mode Texture. Premier lot du programme à toucher `Core` et le format de niveau JSON — il porte
donc aussi les deux décisions de format qui en découlent :

- le **numéro de version** du fichier de niveau (`EX-LVL-005`), avant que trois lots successifs n'y
  ajoutent chacun leur champ sans traçabilité ;
- la **désignation du jeu de skins** utilisé par le niveau (`EX-EDIT-024`), dont le format a été posé
  en LOT-42 mais qui ne pouvait pas être rattaché à un niveau tant que `Core` n'était pas touché.

## Périmètre

### Inclus
- **`Core`** : sur `core::Level`/`core::LevelDraft`, deux champs optionnels
  (`std::optional<std::string>`) — l'asset de fond et le nom du jeu de skins — avec accesseurs et
  mutateurs, ajoutés à `LevelDraft::State` pour l'undo/redo. Même patron que les champs optionnels
  existants (`jumpBudget`/`dashBudget`). Ce sont des **chaînes**, jamais des handles de texture :
  `Core` reste totalement ignorant du rendu.
- **Version de format** (`EX-LVL-005`) : champ racine `"version"` écrit par `LevelWriter`, lu par
  `LevelLoader`. Un fichier **sans** version est lu comme la version initiale, **sans erreur ni
  avertissement** — la rétrocompatibilité des quinze niveaux de démonstration est un invariant, pas
  un objectif souhaitable.
- **JSON** : champs racine optionnels (ex. `"background": "forest.png"`, `"skinSet": "foret"`),
  lus/écrits selon le patron déjà utilisé pour les champs racine optionnels — rétrocompatibles.
- **Résolution `HMI`** : `Assets/Backgrounds/*.png`, balayage de dossier, sélection dans la section
  « Fond » du panneau « Textures » (LOT-42), avec vignettes (LOT-43).
- **Rendu** : calque *Background* de *RenderLayer* (LOT-40), en mode Texture uniquement, **étiré**
  sur les bornes du niveau, avec une règle de **ratio d'aspect** explicitement tranchée (cf.
  décisions de cadrage).
- Dossier `Assets/Backgrounds/` + commande `POST_BUILD`.

### Exclus (hors périmètre de ce lot)
- Fond par salle (`RoomGrid`) : le fond reste en **espace niveau**, simplement cadré par la caméra
  de la salle courante comme le reste du contenu — aucune donnée par salle.
- Fond animé, tuilage, défilement parallaxe : la profondeur relève des **décors** (LOT-49), qui ont
  un modèle adapté (objets libres sur trois couches avec facteur de défilement). Un fond est une
  image de dernier plan, pas un système de couches.
- Ombres portées sur le fond (LOT-55).
- Migration automatique d'un fichier de niveau d'une version à l'autre : il n'y a qu'une version.

## Décisions de cadrage
- **Asymétrie manquant/introuvable** : un niveau **sans** fond configuré est un état normal (fond
  neutre, **pas** de damier magenta) ; un fond **référencé mais introuvable** déclenche le damier +
  avertissement (`EX-NFR-040`). Cette distinction doit être explicite dans l'implémentation, ce ne
  sont pas les mêmes situations.
- **Ratio d'aspect : préservé, avec recadrage par le centre** plutôt qu'étirement libre. Un fond
  16:9 posé sur un niveau presque carré, étiré, serait visiblement déformé, et l'auteur n'a aucun
  moyen de le prévoir depuis l'éditeur. Le recadrage garantit que l'image reste juste, au prix de
  bords perdus — compromis assumé, à documenter dans le contrat d'asset (LOT-40).
- **Version de format ici, pas plus tard** : ce lot est le premier à modifier le schéma JSON ;
  LOT-45 (texture par case) et LOT-49 (décors) suivront. Introduire la version au premier
  changement coûte un champ, l'introduire au troisième coûte une migration.
- **Frontière `Core`** : `background` et `skinSet` sont des **chaînes**. `Core` ne connaît ni
  fichier image, ni dossier d'assets, ni mode de rendu.
- **Culling** : le fond est étiré sur le niveau entier ; son test de visibilité doit porter sur sa
  boîte englobante réelle, pas sur son point d'ancrage (cf. LOT-40, TACHE-05).

## Exigences couvertes
- Nouvelles : `EX-REN-044` (fond de niveau optionnel), `EX-LVL-005` (version du format de niveau).
- Complétée : `EX-EDIT-024` (désignation du jeu de skins par un niveau).
- Réutilisées : `EX-REN-043` (rendu multi-textures), `EX-REN-046` (bascule), `EX-NFR-040` (repli),
  `EX-REN-007` (contrat d'asset), `EX-LVL-003`/`EX-LVL-004` (format et validation).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-version-format.md) | Version de format + champs de fond et de jeu de skins sur `Level`/`LevelDraft`, JSON rétrocompatible, undo/redo | `Source/Core/Levels` | ✅ |
| [TACHE-02](tache-02-rendu-fond.md) | Rendu du fond (calque *Background*, ratio préservé, mode Texture) + repli damier si introuvable | `Source/HMI/Graphics`, `Source/HMI/Game` | ✅ |
| [TACHE-03](tache-03-section-fond.md) | Section « Fond » et sélecteur de jeu de skins du panneau « Textures » + dossier + `POST_BUILD` | `Source/HMI/Editor`, `Source/HMI/CMakeLists.txt` | ✅ |

## Critères d'acceptation du lot
1. Un niveau sans fond configuré s'affiche exactement comme avant ce lot (mode Physique **et**
   Texture), asserté via le *QuadRecorder*.
2. Un fond assigné et valide s'affiche en dessous de tout le contenu, en mode Texture uniquement,
   sans déformation de son ratio.
3. Un fond référencé mais introuvable affiche le damier magenta + avertissement de log.
4. Un niveau existant (sans les nouveaux champs, sans numéro de version) se charge sans erreur ni
   avertissement — rétrocompatible, vérifié sur les quinze niveaux de démonstration.
5. Un niveau désignant un jeu de skins utilise bien ce jeu ; un niveau n'en désignant aucun utilise
   le jeu par défaut.
6. Round-trip JSON (version comprise) et undo/redo testés sans GPU ; build `/W4 /WX`, Doxygen, lint
   verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40), [LOT-41](@ref lot-41), et le panneau « Textures » de
[LOT-42](@ref lot-42) ; bénéficie de [LOT-43](@ref lot-43) (vignettes). **Première extension de
`core::Level`/`LevelDraft` du programme** — le patron de champ optionnel + version qu'elle installe
est réutilisé tel quel par [LOT-45](@ref lot-45) et [LOT-49](@ref lot-49). Prépare
[LOT-51](@ref lot-51), [LOT-55](@ref lot-55).

## Navigation des tâches
- @subpage lot-44-tache-01-modele-version-format
- @subpage lot-44-tache-02-rendu-fond
- @subpage lot-44-tache-03-section-fond
