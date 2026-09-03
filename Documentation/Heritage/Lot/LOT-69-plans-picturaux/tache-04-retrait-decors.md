# TACHE-04 — Retrait du système de décors {#lot-69-tache-04-retrait-decors}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/{Core,HMI,Test,Elements}` ·
**Statut :** fait

## Contexte
Le cadrage a tranché un **remplacement net** : maintenir décors et plans en parallèle doublerait le
format, le rendu et l'éditeur pour un système destiné à disparaître. La tâche exécute ce retrait
d'un bloc, une fois le modèle de remplacement en place (`TACHE-03`) et avant que le rendu des plans
n'arrive (`TACHE-05`) — de sorte que les deux systèmes ne coexistent jamais.

## Travail à réaliser
- **Supprimer** : `Source/Core/Levels/Decor.h` ; `Source/HMI/Graphics/DecorVisuals.{h,cpp}` ;
  `Source/HMI/Editor/{DecorGeometry,DecorGesture,DecorListModel,DecorsPanel}.{h,cpp}` ;
  `Source/Elements/UI/DecorsPanel.ui` ; le générateur d'assets de décors dans `scripts/`.
- **Retirer par petits bouts** : `Level::_decors` et son accesseur ; les dix mutateurs de décor de
  `LevelDraft` ; la composition des décors dans la scène ; `EditorTool::Decor` ; `PanelId::Decors` ;
  `AssetFamily::Decor` — **sans remplacement**, les plans n'étant pas des assets réutilisables mais
  des données de niveau ; l'état de survol et la sélection de décor de `DraftRenderer` ; dans
  `GameViewport` et `MainWindow`, l'asset et la couche actifs, le geste, la sélection, les signaux
  et l'action « centrer la caméra sur le décor » ; les entrées correspondantes des catalogues
  d'actions, d'icônes et de statut.
- **Tests supprimés** : `test_decor.cpp`, `test_decor_mutations.cpp`, `test_decor_geometry.cpp`,
  `test_decor_gesture.cpp`, `test_decor_list_model.cpp`, `test_decor_visuals.cpp`.
- **Tests adaptés** : `test_niveau_ecs.cpp`, `test_editor_status.cpp`, `test_panel_focus.cpp`,
  `test_render_budget.cpp`, `test_render_culling.cpp`, `test_parallax.cpp`,
  `test_layer_visibility.cpp`, `test_asset_contract.cpp`, `test_editor_workspace.cpp`,
  `test_couverture_mecaniques.cpp`, `test_parcours_complet.cpp`.
- **Conserver `Source/Elements/Assets/Decors/` jusqu'à `TACHE-10`**, qui s'en sert comme source pour
  générer les plans de `demo-final` avant de le supprimer.
- **Sort du champ `"decors"` dans le format** (déplacé depuis `TACHE-03`, voir l'écart assumé qui y
  est documenté) : supprimer `parseDecors` et, si le champ est présent, **l'ignorer en journalisant
  un avertissement** nommant le fichier — jamais un `LevelValidationError`. Un champ obsolète n'est
  pas une donnée *invalide* (`EX-LVL-004` vise la validité), et rejeter rendrait illisible tout
  niveau personnel antérieur, à rebours de l'invariant `EX-LVL-005`. `LevelWriter` cesse de l'émettre,
  de sorte qu'un simple charger-puis-enregistrer migre un fichier. Incrémenter
  `kLevelFormatVersion`.

## Fichiers impactés
Une quinzaine de fichiers supprimés, autant retouchés, répartis sur `Core/Levels`,
`HMI/{Graphics,Editor,Game,Interface}`, `Elements/UI` et `Test`.

## Tests (obligatoires)
Un seul test nouveau, sur le champ obsolète : un JSON portant `"decors"` **charge sans erreur**,
produit `planes()` vide, et sa réécriture ne contient plus le champ. Pour le reste, la tâche
**retire** : le critère est que la suite restante passe à 100 % et que rien ne référence plus un
symbole de décor. Attendre une **baisse temporaire** du nombre de cas
de test, remontée par `TACHE-03` et les suivantes — et donc un cahier de test à régénérer.

## Points d'attention
C'est la tâche la plus large en **surface** du lot, et la plus mécanique. Elle doit passer **après**
`TACHE-03` (le modèle de remplacement existe) et **avant** `TACHE-05` (jamais deux systèmes en
parallèle).

Supprimer plutôt que déprécier est délibéré : le compilateur recense alors **tous** les appelants
d'un coup, là où une dépréciation laisserait les deux notions cohabiter indéfiniment. Le `LOT-67`
avait fait le même choix en retirant `endPosition`.

La couverture (`COVERAGE_THRESHOLD_PERCENT`) peut bouger : retirer du code testé change le rapport.
Vérifier qu'on reste au-dessus du seuil plutôt que de le découvrir en CI.

## Definition de fait (DoD)
Aucun symbole `Decor` ne subsiste hors des documents de lots livrés et du `CHANGELOG`. `ctest` à
100 %, couverture au-dessus du seuil, cahier de test régénéré.

## Exigences
`EX-DEC-001`, `EX-DEC-002`, `EX-DEC-004`, `EX-DEC-005`, `EX-DEC-010`, `EX-DEC-020`, `EX-DEC-021`
(retirées — voir la section « Exigences retirées » de [`decors.md`](@ref spec-decors)).
