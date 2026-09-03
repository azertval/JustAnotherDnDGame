# TACHE-01 — Plan « lointain » : générateur, contenu, format des deux niveaux {#lot-70-tache-01-plan-lointain}

**Lot :** [LOT-70](epic.md) · **Emplacement :** `scripts`, `Source/Elements/Levels` ·
**Statut :** fait

## Contexte
`scripts/generate_demo_plans.py` peint aujourd'hui, pour chaque tableau, un fond (densité 8) et —
seulement s'il portait des décors de premier plan — un devant (densité native). `paint_backdrop`
contient déjà, en interne, une boucle qui peint **deux** crénelages (un lointain plus clair, un
proche plus sombre) pour donner une illusion de profondeur — mais c'est une illusion **peinte**,
sur un seul plan, à un seul facteur de parallaxe : les deux crénelages bougent ensemble. Ce que ce
lot ajoute, c'est un **vrai** troisième plan, avec son propre facteur, plus lent.

## Travail à réaliser
- Factoriser la boucle de crête déchiquetée de `paint_backdrop` en une fonction `jagged_ridge`
  réutilisable (mêmes paramètres que l'usage actuel : ligne de crête, remplissage, teinte de
  sommet, rugosité, densité).
- Nouveau motif procédural `star()` : un petit éclat (croix de quelques pixels), pour le thème
  nocturne uniquement.
- Nouvelle fonction `paint_far_backdrop(width_units, height_units, pixels_per_unit, theme, seed)` :
  - un voile de ciel **plus transparent** que celui du fond (la profondeur se lit aussi à
    l'estompage atmosphérique) ;
  - une **unique** crête déchiquetée (pas deux), positionnée **au-dessus** de l'horizon du plan
    fond correspondant, dans une teinte dérivée du ciel du thème (mélange crête/ciel, cohérent avec
    le calcul déjà fait pour le crénelage « distant » de `paint_backdrop`) ;
  - pour le thème `test_night.png` (`demo-mouvement`) : semis d'étoiles au-dessus de la crête
    (`drift`, motif `star`) ;
  - pour le thème `test_industrial.png` (`demo-final`) : silhouettes de piliers lointains posés sur
    la crête (`stand_on`, motif `pillar`, densité 4).
- Étendre `build_level`/`build_all` : un plan `<stem>-lointain.png` est produit **uniquement** pour
  `demo-mouvement` et `demo-final` — une liste explicite (`FAR_BACKDROP_LEVELS`), pas une règle
  déduite du thème, pour que la restriction reste visible à la lecture plutôt qu'accidentelle.
- Mettre à jour `demo-mouvement.json` et `demo-final.json` : ajouter le plan lointain **en tête**
  de `planes` (densité 4, `parallaxX` inférieur à celui du plan fond existant — 0.2 contre 0.6 pour
  `demo-mouvement`, 0.15 contre 0.5 pour `demo-final`, l'écart plus large sur `demo-final`
  compensant ses salles plus grandes). Le plan fond et le plan devant existants ne changent pas.
- Régénérer les PNG (`python scripts/generate_demo_plans.py`) ; les vingt autres tableaux ne doivent
  produire **aucune** différence d'octet.

## Fichiers impactés
`scripts/generate_demo_plans.py`, `Source/Elements/Levels/demo-mouvement.json`,
`Source/Elements/Levels/demo-final.json`, `Source/Elements/Levels/Plans/demo-mouvement-lointain.png`
(nouveau), `Source/Elements/Levels/Plans/demo-final-lointain.png` (nouveau).

## Tests (obligatoires)
- `python scripts/generate_demo_plans.py --check` : vert, y compris sur les vingt tableaux non
  touchés (aucun octet de différence).
- `python scripts/check_demo_sequence.py` : la séquence des 22 niveaux reste identique.
- Chargement des deux niveaux modifiés inchangé au niveau gameplay : les tests système existants
  (`test_parcours_complet.cpp`, `test_couverture_mecaniques.cpp`) passent sans retouche, la
  modification étant purement visuelle (`EX-ARCH-012`).
- `ctest` du budget de rendu (`test_render_budget.cpp`) : le plan supplémentaire reste sous le
  plafond de mémoire de texture et de primitives des deux niveaux concernés.

## Points d'attention
Le plan lointain n'est visible **que** dans la bande de ciel du plan fond (semi-transparente
au-dessus de son horizon) : peindre du contenu sous cette ligne serait du travail perdu, invisible à
l'écran quelle que soit sa qualité. C'est la contrainte qui fixe la position de la crête du plan
lointain, pas un choix esthétique libre.

`jagged_ridge` doit produire des résultats **identiques** à l'ancien code inline de
`paint_backdrop` une fois appelée avec les mêmes paramètres — sinon les vingt-deux PNG existants
changeraient tous, ce que `--check` détecterait immédiatement au premier lancement après le
refactor.

## Definition de fait (DoD)
Les deux niveaux ciblés déclarent trois plans aux facteurs croissants, les PNG sont générés et
reproductibles, les vingt autres tableaux sont bit-à-bit inchangés, `ctest` à 100 %.

## Exigences
`EX-DEC-040`, `EX-DEC-041`, `EX-DEC-043`, `EX-LVL-009`.
