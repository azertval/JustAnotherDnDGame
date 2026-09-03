# TACHE-02 — Vérification finale et documentation du programme {#lot-55-tache-02-verification-documentation}

**Lot :** [LOT-55](epic.md) · **Emplacement :** `Source/Test`, `Documentation` · **Statut :** fait

## Réalisation
- **Documentation** : section « Ombres du plan physique (LOT-55) » ajoutée à
  `Documentation/Guide/guide-rendu.md` ; la section d'orientation « Ce qui vient ensuite »
  (obsolète depuis plusieurs lots — elle ne listait que 8 des 16 lots du programme) est remplacée
  par un état livré complet du programme d'habillage `LOT-40` → `LOT-55` (`LOT-53`, effets et
  particules, explicitement noté non commencé et hors de cette page tant qu'il ne l'est pas).
  `Source/HMI/Graphics/README.md` référence `ShadowRenderer` ; la section « À venir », qui ne
  listait plus que ce lot, est retirée. `Documentation/Manuel/jouer.md` et
  `partager-un-niveau.md` mentionnent les ombres dans la description du mode habillé (`F8`) et de
  l'onglet Calques.
- **Tests** : `Source/Test/Unit/HMI/Graphics/test_shadow_render.cpp` couvre les cas de la
  définition de fait de TACHE-01/TACHE-03 (tuile pleine, tuiles non physiques, mode Physique, les
  douze silhouettes contre `hmi::regionForTile`, blocs réduits, bloc poussable interpolé, porte
  selon l'état courant de la grille de collision). L'ensemble de la suite (`ctest`, 943 cas,
  unitaires + intégration + système) reste vert ; `generate_cahier_test.py --check`,
  `lint_exigences.py` (aucun résultat hors pollution connue du worktree `.claude/`) et
  `check_demo_sequence.py` passent ; build `/W4 /WX` et Doxygen propres.
- **Vérification manuelle** (mode Texture avec/sans fond ni décor, bascule `F8` dans les trois
  contextes, isolement par calque, budget de primitives sur le plus grand niveau de démonstration)
  : laissée à l'essai réel du joueur/designer au moment de la revue, conformément à la pratique du
  projet — pas d'automatisation d'interface ad hoc pendant le développement du lot.

## Contexte
Dernier lot du programme d'habillage. Au-delà de la vérification de ses propres ombres, cette tâche
est l'occasion — et la dernière — de constater que l'ensemble tient : seize lots ont ajouté des
calques, des animations, des décors, du texte et des effets par-dessus un rendu qui n'affichait
qu'une texture et deux couches.

## Travail à réaliser
- **Vérification manuelle des ombres** : mode Texture avec fond, avec décor d'arrière-plan, sans
  aucun des deux ; mode Physique ; sur un niveau à pentes et arrondis.
- **Vérification d'ensemble du programme**, sur un niveau de démonstration complet :
  - les sept calques s'empilent dans l'ordre attendu, premier plan **au-dessus** du personnage ;
  - `F8` bascule proprement entre Physique et Texture, dans les trois contextes ;
  - les visibilités par calque (LOT-51) isolent correctement chaque plan ;
  - les mécanismes s'animent selon leur état, sans modulation d'opacité résiduelle ;
  - le HUD affiche les budgets sur un niveau à budget, rien sur les autres.
- **Budget de primitives** : relever le nombre de primitives par image sur le plus grand niveau de
  démonstration, en mode Texture complet, et vérifier qu'il reste dans les bornes attendues
  (`EX-NFR-005`). C'est la seule mesure de bout en bout de l'effet cumulé du programme.
- **Documentation** :
  - `Documentation/Guide/guide-rendu.md` — section sur les ombres ; **remplacer** la section
    d'orientation « ce qui vient ensuite » par la description de l'état livré ;
  - `Documentation/Manuel/jouer.md` et `partager-un-niveau.md` — état final des commandes ;
  - `Source/HMI/Graphics/README.md` — retirer la section « À venir », désormais réalisée.

## Fichiers impactés
- `Source/Test/Systeme/` (scénario de vérification d'ensemble, si automatisable).
- `Documentation/Guide/guide-rendu.md`, `guide-editeur.md`,
  `Documentation/Manuel/jouer.md`, `partager-un-niveau.md`.
- `Source/HMI/Graphics/README.md`, `Source/HMI/Editor/README.md`,
  `Source/Elements/Assets/README.md`.

## Tests (obligatoires)
- Test de composition d'ensemble : sur une scène couvrant tous les calques, l'ordre des primitives
  est celui de `EX-REN-014`, asserté via le *QuadRecorder*.
- Dénombrement de primitives sur le plus grand niveau de démonstration, en mode Texture complet.
- L'ensemble de la suite de tests reste verte, y compris les tests de franchissabilité.

## Points d'attention
- **Les sections « À venir » doivent disparaître.** Une documentation qui annonce encore comme futur
  ce qui est livré est pire qu'une documentation absente : elle induit en erreur le prochain
  développeur.
- Le guide de rendu décrit encore, en plusieurs endroits, des limites levées par le programme (une
  seule texture par lot, deux valeurs de couche, pas de texte en scène). Les relire toutes, pas
  seulement celles de ce lot.
- Si le dénombrement de primitives révèle un dépassement, c'est un constat à **remonter**, pas à
  corriger dans ce lot : l'optimisation par atlas dynamique a été explicitement écartée en LOT-40,
  sous réserve d'un profilage réel. Ce serait ce profilage.

## Définition de fait (DoD)
- Les ombres sont vérifiées dans tous les cas de fond ; l'ensemble du programme est constaté
  cohérent sur un niveau complet ; le budget de primitives est mesuré et documenté ; guides, manuel
  et README ne contiennent plus de section « À venir » réalisée ; Doxygen et lint verts.

## Exigences
`EX-REN-045` (ombres) ; réutilise `EX-REN-014` (ordonnancement des calques), `EX-NFR-005` (budget de
primitives), `EX-NFR-004` (vérification sans GPU), `EX-NFR-021` (test système), `EX-NFR-012`
(conventions et documentation).
