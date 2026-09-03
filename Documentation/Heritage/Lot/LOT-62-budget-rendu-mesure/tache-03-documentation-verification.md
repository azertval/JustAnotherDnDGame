# TACHE-03 — Documentation, mesure de référence et vérification {#lot-62-tache-03-documentation-verification}

**Lot :** [LOT-62](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Ce lot produit des **chiffres**, et un chiffre sans date ni contexte devient inexploitable en
quelques mois. Le `LOT-55` en a fait l'expérience : sa vérification finale était « la seule mesure
de bout en bout de l'effet cumulé du programme » d'habillage, elle a été faite à la main, et rien
n'en subsiste d'utilisable.

L'autre travail est de corriger deux exigences qui promettent une vérification qu'elles n'ont
jamais eue.

## Travail à réaliser
- **Consigner les mesures de référence** : par niveau livré et par mode de rendu, primitives
  composées et soumises, fraction écartée par le culling, à la **date** du lot et avec la
  configuration de mesure. Emplacement : `Documentation/Guide/guide-rendu.md`, qui décrit déjà le
  pipeline de composition et le culling.
- **Compléter `EX-NFR-005` et `EX-NFR-001`** dans
  `Documentation/Specification/exigences-non-fonctionnelles.md` : indiquer **par quoi** elles sont
  vérifiées (test de budget pour l'une, affichage de diagnostic pour l'autre) et ce qui, dans
  `EX-NFR-001`, reste hors de portée d'un contrôle automatique.
- **Guide du développeur** : section « budget de rendu » — plafonds, où ils vivent, comment les
  mettre à jour légitimement (un lot de contenu qui ajoute un calque) et pourquoi les ajuster pour
  faire passer un test est exactement ce qu'il ne faut pas faire.
- **Manuel utilisateur** : mentionner l'affichage de diagnostic s'il est activable par une touche —
  le manuel documente déjà `F8`, la symétrie est attendue.
- **Cahier de test** régénéré ; chaque nouveau `TEST()` porte son bloc `\castest{}` écrit en même
  temps que lui.
- **Consigner tout dépassement constaté** sans le corriger : ce lot mesure, il n'optimise pas.

## Fichiers impactés
- `Documentation/Guide/guide-rendu.md`.
- `Documentation/Specification/exigences-non-fonctionnelles.md`.
- `Documentation/Manuel/jouer.md`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — aucune nouvelle exigence n'est créée par ce lot ; vérifier
  qu'aucune référence orpheline n'a été introduite.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %.
- **Vérification manuelle** : activer l'affichage de diagnostic sur `demo-salles` (le plus lourd) et
  constater la cadence réelle sur la machine de développement — c'est la mesure que `EX-NFR-001`
  demande et que rien n'automatisera.

## Points d'attention
- **Dater les mesures.** Un tableau de chiffres sans date est un piège pour la personne qui le
  relira après trois lots de contenu.
- **Ne pas transformer un dépassement en correction ici.** S'il y en a un, il est consigné et
  devient un sujet à part entière ; le mélanger à la mise en place de la mesure rendrait les deux
  suspects.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Le guide de rendu est long : insérer la section au bon endroit plutôt que l'ajouter en fin de
  fichier.

## Définition de fait (DoD)
- Les mesures de référence sont consignées, datées et contextualisées ; `EX-NFR-001` et `EX-NFR-005`
  indiquent leur moyen de vérification et ses limites ; le guide explique comment faire évoluer un
  plafond légitimement ; le cahier est régénéré ; la CI complète est verte ; la mesure manuelle de
  cadence est faite.

## Exigences
Réutilise `EX-NFR-001` (60 images par seconde), `EX-NFR-005` (primitives bornées et observables),
`EX-NFR-004` (vérification sans GPU), `EX-NFR-012` (conventions), `EX-NFR-022` (CI verte).
