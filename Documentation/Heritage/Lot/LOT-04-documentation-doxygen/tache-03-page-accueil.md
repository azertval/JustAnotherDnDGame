# TACHE-03 — Page d'accueil du projet (mainpage) {#lot-04-tache-03-page-accueil}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/index.md` · **Statut :** fait

## Contexte
Le site a besoin d'une porte d'entrée. `index.md` devient la page principale Doxygen (`USE_MDFILE_AS_MAINPAGE`, cf. TACHE-02) et présente le projet en quelques sections, avec des liens vers les grandes rubriques.

## Travail à réaliser
- Rédiger `Documentation/index.md` avec, a minima :
  - **Pitch** : ce qu'est JustAnotherDnDGame (jeu 2D C++/DirectX, moteur maison from-scratch).
  - **Technologies & architecture en bref** : C++20, Direct3D 11, ECS dans `Core`, séparation `Core`/`HMI`/`Elements`.
  - **État d'avancement** : lots livrés (LOT-01→03) et lot courant.
  - **Navigation** : liens vers les **Spécifications**, les **Lots** et le **Manuel** (via `@subpage` — cf. TACHE-04/05/06), et un renvoi vers la **référence de code** (namespaces/classes).
- Déclarer la page comme racine de la hiérarchie de navigation (`@mainpage` ou titre + `@subpage` vers les rubriques).

## Fichiers impactés
- `Documentation/index.md` (nouveau).

## Vérifications (obligatoires)
- Après génération, `index.html` affiche la page d'accueil rédigée.
- Les liens vers Spécifications / Lots / Manuel / référence de code fonctionnent.
- Aucun avertissement Doxygen sur cette page (référence, ancre).

## Points d'attention
- La page cible les nouveaux venus (contributeurs et non-codeurs) : claire, sans jargon inutile.
- Garder l'accueil **stable** : détails volatils (avancement fin) renvoyés vers les lots plutôt que dupliqués.

## Définition de fait (DoD)
- Page d'accueil décrivant le projet, tête de la navigation, sans avertissement.

## Exigences
`EX-NFR-012`.
