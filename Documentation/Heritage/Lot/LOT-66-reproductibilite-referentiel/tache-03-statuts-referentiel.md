# TACHE-03 — Statuts des spécifications et exigences orphelines {#lot-66-tache-03-statuts-referentiel}

**Lot :** [LOT-66](epic.md) · **Emplacement :** `Documentation/Specification` ·
**Statut :** fait

## Contexte
Huit fichiers de spécification sur dix portent la mention « Statut : **brouillon** » —
`architecture.md`, `gameplay.md`, `vision.md`, `rendu-technique.md`, `controles.md`, `niveaux.md`,
`interface-ihm.md`, `exigences-non-fonctionnelles.md`. Certains n'ont pas bougé depuis vingt lots et
décrivent un système entièrement livré.

Les deux exceptions montrent la voie : `decors.md` annonce « partiellement livré » en détaillant ce
qui l'est, et `editeur-niveaux.md` « édition de tuiles de base et robustesse/confort d'édition
validés et livrés ». Le modèle existe ; il n'a simplement jamais été appliqué aux autres.

S'ajoutent douze exigences déclarées et jamais référencées. Six auront été traitées par les lots du
programme `0.1.0` (`EX-NFR-003` par [LOT-58](@ref lot-58), `EX-VIS-002` à `EX-VIS-005` par les
tableaux de démonstration et [LOT-59](@ref lot-59)). Restent celles qui ne seront jamais référencées
parce que ce n'est pas leur nature.

## Travail à réaliser
- **Un statut par fichier**, reflétant son état réel, sur le modèle de `decors.md` : ce qui est
  livré, ce qui reste ouvert. Pas de statut global, pas de « validé » là où `vision.md` conserve des
  propositions marquées ⚠️.
- **Relever les mentions ⚠️ restantes** dans tous les fichiers, et pour chacune : levée par un lot
  du programme, ou explicitement reportée. Aucune ne doit subsister sans que son sort soit dit.
- **Requalifier les exigences orphelines**, sans fabriquer de références artificielles :
  - `EX-ARCH-001`, `EX-ARCH-060`, `EX-ARCH-070`, `EX-NFR-032` — **invariants transverses**, respectés
    par tout lot sans avoir à être cités. Le référentiel doit le dire, dans une phrase de
    traçabilité, comme `exigences-non-fonctionnelles.md` le fait déjà pour ses exigences
    transverses.
  - `EX-VIS-002` à `EX-VIS-007` — **objectifs de vision**, réalisés par des lots qui référencent les
    exigences détaillées correspondantes. Ajouter, sous chacun, le renvoi vers les exigences qui le
    concrétisent.
  - `EX-DEC-031` (paramètres de conversion photo → pixel art) — vrai reste **post-MVP**, à marquer
    comme tel, sans le traiter.
- **Vérifier la cohérence** des renvois entre `vision.md` et les fichiers détaillés : plusieurs
  affirmations de périmètre auront été modifiées par les lots du programme (progression persistée,
  plateformes mobiles, audio).

## Fichiers impactés
- `Documentation/Specification/*.md` (tous).
- `Documentation/Specification/specifications.md` (page d'index), si elle résume les statuts.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — aucune orpheline **inexpliquée** : chacune est référencée,
  ou qualifiée d'invariant, ou marquée post-MVP. Consigner la liste résiduelle et sa justification.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml` — les renvois
  ajoutés ne cassent aucun lien.
- Relecture croisée : aucune mention ⚠️ ne subsiste sans que son sort soit explicitement dit.

## Points d'attention
- **Ne pas fabriquer de références pour faire taire le lint.** Citer `EX-ARCH-060` dans un lot au
  hasard rendrait le compteur propre et le référentiel faux. Le lint mesure l'absence de référence
  **orpheline** ; qu'un invariant ne soit cité nulle part est normal, et c'est cela qu'il faut
  documenter.
- **Ne pas réécrire le fond.** Ce lot ajuste des statuts et des mentions devenues fausses. Une
  refonte des spécifications juste avant un tag est le meilleur moyen de publier un référentiel
  incohérent.
- Un statut doit être **daté ou versionné** pour rester utile : « livré en `0.1.0` » vaut mieux que
  « livré ».
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Le lint d'exigences produit des centaines de faux positifs si un worktree traîne sous `.claude/` :
  filtrer avant de conclure à un doublon.

## Définition de fait (DoD)
- Chaque fichier de spécification porte un statut reflétant son état réel ; toute mention ⚠️ a un
  sort explicite ; les exigences orphelines résiduelles sont qualifiées et justifiées ; les renvois
  de `vision.md` concordent avec ce qui est livré ; lint et documentation verts.

## Exigences
Réutilise `EX-ARCH-001` (sens des dépendances), `EX-NFR-032` (cible Windows), `EX-VIS-002` à
`EX-VIS-007` (objectifs, requalifiés), `EX-DEC-031` (post-MVP, marqué), `EX-NFR-012` (conventions).
