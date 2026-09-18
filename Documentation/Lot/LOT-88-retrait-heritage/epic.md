# LOT-88 — Retrait de l'héritage : code, assets et spécifications {#lot-88}

> Statut : **livré le 18 septembre 2026** (ouvert le même jour), sur la branche
> `lot-88-retrait-heritage`. Vérification automatisée : construction `/W4 /WX` sans avertissement,
> `ctest` à **768/768**, `lint_exigences.py`, `lint_lots.py`, les contrôles Python de la CI
> et Doxygen verts. Reste
> la vérification manuelle : ouvrir une carte dans l'éditeur, l'essayer (**P**), y marcher,
> franchir un portail, revenir à l'édition par **Échap** ; lancer le jeu et parcourir Martpart.
> Prérequis : [LOT-67](@ref lot-67) (la notion de niveau discret retirée), [LOT-11](@ref lot-11)
> (l'éditeur retargé), [LOT-09](@ref lot-09) (l'exploration dans le jeu, sur laquelle l'essai de
> l'éditeur se rebranche).
> Absorbe : le `LOT-69` (retrait de l'atelier pixel art).
> Exigences : **120 retirées**, ancres conservées ; `EX-EDIT-055` refondue.

## Objectif

Le dépôt est né d'un jeu de plateforme en vue de côté. Le [LOT-01](@ref lot-01) en avait purgé les
niveaux et le solveur par apprentissage ; il restait le reste : un runtime de jeu que l'essai de
l'éditeur était seul à faire tourner, les mécanismes (interrupteurs, portes, clés, plaques,
plateformes mobiles), la tuile de sortie, le cadrage de caméra, les plans et la parallaxe,
l'habillage par skins et raccords automatiques, un atelier de dessin au pixel, des bruitages, des
assets et leurs scripts générateurs — et la moitié des spécifications, qui décrivait ce jeu-là.

Ce lot retire tout ce qui ne sert plus le RPG : code, assets, spécifications, guides, manuel,
README. Le dépôt ne garde **aucun lien** avec le moteur d'origine : aucune mention de son nom, aucun
renvoi à son programme de lots, aucun fichier archivé.

## Décisions de l'auteur (18 septembre 2026)

| Question | Décision |
|---|---|
| L'essai immédiat de l'éditeur faisait tourner l'ancien runtime | **Le rebrancher sur le jeu** : l'essai joue l'exploration du jeu (`core::ExplorationSession`) sur le brouillon, dessinée par le rendu du lieu (`hmi::WorldSceneRenderer`). L'ancien runtime part. |
| Mécanismes de puzzle | **Tout retirer** : types de tuiles, liaisons, contrôleur, visuels, format. |
| Exigences qui décrivent le jeu d'origine | **Registre compact** : le texte part, l'ancre reste avec un motif d'une ligne ; aucun numéro réattribué. |
| Habillage de l'éditeur | **Tout l'habillage mort** : skins, raccords, pièce par case de l'ancien outil, fond, plans, ombres, bascule `F8`, cadrage et zones de caméra, atelier pixel art, bibliothèque d'assets. L'éditeur peint types, couches et entités en couleurs procédurales. |

## Ce qui a été fait

### 1. L'essai de l'éditeur joue le jeu

- `hmi::WorldPlay` (`Source/HMI/Game/WorldPlay.h`), sans Qt : la mise en scène de l'exploration —
  une session, ses figurines, l'instantané de scène — partagée par `hmi::WorldModel` (le jeu) et
  l'essai de l'éditeur. Le jeu et l'éditeur ne peuvent plus diverger sur ce qu'est « marcher sur
  une carte ».
- `EditorViewport` dessine le brouillon (`DraftRenderer`) et, pendant l'essai, la scène du jeu.
  Le brouillon est servi sous son propre identifiant de carte ; les autres cartes se lisent sur
  disque, si bien qu'un portail mène vraiment ailleurs.
- Supprimés : `GameSession` et ses modes, la physique de plateforme (`Core/Physics`), les systèmes
  de mouvement, d'animation et de particules, les mécanismes, l'issue de niveau, la caméra de
  suivi, les zones de caméra, les plans et la parallaxe, les ombres, les skins et raccords, les
  polices bitmap, l'incrustation de diagnostic `F8`/`F9` de l'ancien runtime, les déclencheurs de
  sons, les liaisons clavier et manette du jeu d'origine, et les panneaux d'éditeur qui les
  servaient (liens, textures, plans, atelier pixel art, bibliothèque, espaces de travail, remappage).

### 2. Le modèle de carte ne porte plus que le RPG

- `core::TileType` garde douze types : vide, plein, entrée, herbe, terre, sable, eau, eau profonde,
  mur, falaise, pont, escalier.
- `core::Level` et `core::LevelData` perdent la sortie, les mécanismes, le fond, le jeu de skins,
  le cadrage et les plans ; `CameraFraming` et `Plane` sont supprimés. Le chargeur n'exige plus
  qu'une entrée.
- Les trois cartes livrées (Colisée, Martpart, Arenarea) perdent leur tuile de sortie et leur
  cadrage ; les ateliers du [LOT-09](@ref lot-09) et du [LOT-96](@ref lot-96) cessent de les
  écrire.

### 3. Assets, scripts et restes de code

- Supprimés : les fonds, objets, sprites du joueur, skins, palettes et l'atlas en image de
  l'ancien jeu (`Source/Elements/Assets/`), les bruitages (`Source/Elements/Audio/`), et leurs
  scripts générateurs (`scripts/generate_test_*.py`, `scripts/motifs/`).
- L'atlas de tuiles est désormais **procédural seul**, sans rangées de sprite du joueur ; la
  secousse de caméra et les calques de plan disparaissent ; l'option `--export-atlas` part.
- Traductions : les clés mortes partent, les messages de l'essai s'ajoutent.

### 4. Spécifications, guides, manuel

- `ia.md` et `decors.md` sont retirées ; leurs 41 ancres (`EX-IA-*`, `EX-DEC-*`) vivent dans
  [`exigences-retirees.md`](@ref spec-retirees).
- `gameplay.md`, `controles.md`, `rendu-technique.md`, `niveaux.md`, `editeur-niveaux.md`,
  `interface-ihm.md`, `architecture.md`, `exigences-non-fonctionnelles.md` sont réécrites sur le
  jeu réel ; chacune liste ses exigences retirées en fin de page.
- `vision.md` perd la section « Origine » ; `contenu.md` dit treize régions.
- `Documentation/Heritage/` (l'archive des guides, lots et specs du jeu d'origine) est supprimée,
  et avec elle la notation `LOT-H-NN` : `lint_lots.py` refuse désormais toute occurrence dans une
  spécification.
- `lint_exigences.py` reconnaît la marque *(retirée…)* : une exigence retirée n'a pas à être
  référencée, et **aucun code ne peut la citer**.
- Guides, manuel et README décrivent le jeu et l'éditeur tels qu'ils sont ; les guides de la
  physique et de l'atelier pixel art, et la page « Regarder l'IA jouer » du manuel, sont supprimés.
- Le `CHANGELOG` perd les versions du jeu d'origine.

## Écarts par rapport au texte de la feuille de route

- **Élargi.** Le lot était « une passe de rédaction, aucune ligne de code ». L'auteur l'a élargi
  au code, aux assets et à toute mention du projet d'origine : une spécification qui retire une
  mécanique encore compilée ne dit pas la vérité non plus.
- **Registre compact au lieu du texte d'origine intact.** Le texte des exigences retirées part ;
  l'ancre et un motif d'une ligne restent.
- **`Documentation/Heritage/` supprimé**, et non enrichi de `ia.md` et `decors.md`.
- **Le `LOT-69` est absorbé** : l'atelier pixel art est parti avec le reste de l'habillage mort.
  La question qu'il laissait ouverte — la portée identité de l'éditeur charge encore les polices
  pixel — reste posée : les polices appartiennent à la charte, elles ne sont pas de l'héritage.

## Exigences qui perdent leur porteur

Certaines exigences toujours vraies n'étaient citées que par l'archive supprimée. Elles restent en
vigueur ; voici ce qui les tient aujourd'hui.

| Exigence | Ce qui la tient |
|---|---|
| `EX-IHM-001`, `EX-IHM-041` | Deux exécutables, une technologie d'UI chacun : Qt Quick pour le jeu, Qt Widgets pour l'éditeur ([LOT-86](@ref lot-86)) |
| `EX-IHM-002` | La scène embarquée dans un élément Qt : `WorldViewportItem`, `ArenaViewportItem`, et le `QRhiWidget` de l'éditeur |
| `EX-REN-030` | Le menu principal en Qt Quick (`MainMenu.qml`) |
| `EX-GP-014` | Le déplacement de `core::ExplorationSession`, résolu axe par axe contre les tuiles solides |
| `EX-NFR-001` | Le compteur de diagnostic des options ; la cadence se constate, elle ne se vérifie pas en CI |
| `EX-EDIT-020`, `EX-EDIT-021`, `EX-EDIT-022` | `LevelEditor.exe`, livré à côté du jeu ; les cartes dans `Source/Elements/Levels/` ; le guide [Créer et partager une carte](@ref manuel-partager-niveau) |

## Ce qui reste

- Les tags `v0.0.1` à `v0.1.3` et `archive/platformer-v0.1.3` du dépôt distant désignent des
  versions du jeu d'origine, et les trois premiers entrent en collision avec les jalons de la
  feuille de route. Les supprimer est une décision de l'auteur, hors de ce lot.

## Vérification

- Construction et tests : `scripts/build.ps1 -Preset ninja -Test`.
- Lints : `py -3.13 scripts/lint_exigences.py`, `py -3.13 scripts/lint_lots.py`,
  `py -3.13 scripts/check_translations.py`, `py -3.13 scripts/build_docs.py`.

## Texte d'origine (feuille de route, second audit)

> **Ajouté au second audit.** Les spécifications sont relues comme des entrants, et la moitié
> d'entre elles décrit **un autre jeu**. `gameplay.md` est « livré » avec le saut, la gravité, les
> pentes, les blocs poussables et 39 exigences de plateforme actives ; `controles.md` mappe Sauter
> et Dash ; `rendu-technique.md` exige un écran de fin de niveau et des bruitages de saut ;
> `niveaux.md` décrit le format par ses tuiles de pente ; `ia.md` déclare « livré » un solveur par
> apprentissage purgé au `LOT-01`. Le `LOT-67` n'a retiré que la notion de niveau discret ; le
> `LOT-77` a écrit la moitié RPG ; personne ne portait le retrait de l'autre moitié.
>
> *Acceptation* — aucune exigence active ne décrit une mécanique retirée du moteur ;
> `lint_exigences.py` et Doxygen verts ; aucune ancre supprimée.
