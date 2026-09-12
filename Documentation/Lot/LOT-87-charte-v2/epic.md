# LOT-87 — Charte v2 et intégration des maquettes {#lot-87}

> Statut : **en cours**.
> Prérequis : `LOT-86`, `LOT-39`.

## Objectif

Adopter les dix maquettes du pack UI comme charte visuelle v2, et transcrire les écrans du jeu sur
cette base : panneaux sombres et parchemin, `Cinzel` en titres, `IM Fell English` en corps,
illustrations peintes. Les assets (cadres 9-patch, plaques, boutons, fonds) sont produits à part, à
1080p, depuis un cahier des assets — jamais découpés des maquettes elles-mêmes, qui restent des
références de cotes et de composition.

## Pourquoi les maquettes remplacent la charte parchemin pixel

Les maquettes du pack contredisent la direction retenue par les `LOT-66` et `LOT-76` (parchemin de
Tanares, sans illustration peinte). Elles sont malgré tout la matière la plus riche dont dispose le
projet pour les dix écrans qu'elles couvrent : la décision, prise le 11 septembre 2026, est de les
adopter comme charte v2 plutôt que de continuer à les ignorer. Le prix assumé : deux directions
graphiques auront coexisté dans l'historique du dépôt, la seconde remplaçant entièrement la
première.

## Ce que Claude ne produit pas

Aucun cadre doré, aucun fond peint n'est dessiné par un modèle Claude. La phase 2 sépare deux
métiers : Claude écrit le cahier des assets (liste, dimensions, marges 9-patch, description,
prompt de génération par pièce) et vérifie/enregistre ce qui revient ; la production des images
passe par un générateur d'images ou un illustrateur, à partir de ce cahier.

## Programme, en six phases

1. **Socle vert** — dépôt propre (branches mortes archivées et retirées, débris de travail retirés,
   maquettes déplacées dans ce dossier), lot inscrit à la feuille de route.
2. **Le module de conception** — Qt Design Studio 4.8.2 ouvre chaque `*Form.ui.qml` en mode
   conception, le jeu démarre : trois modules (`Jadg.Ui`, `Jadg.App`, `Jadg.Runtime`), des
   doublures QML pour les types C++, un garde-fou de compatibilité réécrit en contrat plutôt qu'en
   structure CMake.
3. **Charte v2 et matière première** — la charte écrite dans cet epic, les jetons de `Tokens.qml`
   étendus, les polices `Cinzel` et `IM Fell English` déposées, le cahier des assets, le manifeste
   étendu aux images produites, et les briques QML (`PanelFrame`, `OrnateButton`, `StatMedallion`,
   etc.) qui les consomment.
4. **Les écrans, un par Pull Request** — menu principal, options, crédits, fiche de personnage,
   inventaire et équipement, carte du monde, équipe de mercenaires, compétences et sorts ; pause,
   dialogue, marchand et journal restylés sans redessin.
5. **Le cadre du HUD** — `CombatHudForm` et `GameViewForm` réécrits avec le cadre de la maquette,
   branchement différé aux lots qui fourniront les données réelles.
6. **Retrait** — le dossier `JustAnotherDnDGame_UI_ASSET_PACK/` et l'ancienne charte (`LOT-86`)
   supprimés une fois qu'aucun écran ne les cite plus.

## Exigences couvertes

À écrire en phase 2 : la charte v2 remplace la description de `interface-ihm.md` et des exigences
`EX-IHM-070` et voisines, aujourd'hui écrites pour le parchemin pixel.

## Où en est le lot

**Phase 0 en cours.**

- Branches mortes (dix-neuf) archivées sous des tags `archive/<nom>` et retirées localement ; les
  branches distantes correspondantes restent à retirer (nécessite une confirmation explicite, hors
  de portée de l'automatisation).
- `main` avancé sur `origin/main`, les trois worktrees obsolètes purgés.
- `lot/LOT-87-charte-v2` ouverte depuis `main` ; `scripts/check_qml_designer_compat.py` et
  `Source/Ui/DesignStudio/Main.ui.qml` récupérés depuis `archive/fix/qt-designer-6.8.7-compatibility`
  pour la phase 1.
- Les dix maquettes déplacées dans `references/`.
- Ce dossier créé : comme tout lot démarré, le `LOT-87` quitte `roadmap-0.1.0.md` pour cet epic,
  et `lots.md` porte désormais son `@subpage lot-87`.

## Critères d'acceptation de la phase 0

- `git branch -a` ne liste plus que `main`, `gh-pages` et `lot/LOT-87-charte-v2`, hors branches
  distantes en attente de suppression manuelle.
- La branche construit et démarre sur les quatorze écrans via `scripts/build.ps1 -Test` (`ctest` :
  1033/1033 le 12 septembre 2026).
- `check_ui_assets.py`, `check_ui_layers.py`, `lint_lots.py`, `lint_exigences.py` verts.
- Doxygen sans erreur.
