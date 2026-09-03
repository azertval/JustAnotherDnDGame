# Lots annexes — IA de résolution autonome {#lots-annexe}

Programme annexe au découpage principal ([Lots](@ref lots)) : une **IA maison**, développée
**from scratch** (aucun framework d'apprentissage automatique — pas de LibTorch, pas de
Python/PyTorch), capable de terminer **de façon autonome** n'importe quel niveau du jeu, entraînée
niveau par niveau, puis rejouée en jeu sous forme d'une **séquence d'actions déterministe
pré-calculée** (aucune inférence en temps réel, aucune nouvelle dépendance dans `Core`/`HMI`).

Comme pour les lots principaux, chaque lot annexe est un sous-dossier `LOT-ANNEXE-NN-nom/`
contenant un `epic.md` (objectif, périmètre, critères d'acceptation) et des fichiers `tache-NN.md`
(une unité de travail chacun). Les lots annexes référencent les
[spécifications](@ref specifications) via les identifiants `EX-IA-…`, alloués séquentiellement
comme toute autre catégorie (`scripts/lint_exigences.py --next`) — sans plage réservée par
génération, la lisibilité par génération vient de cette page, pas de la numérotation.

**Numérotation propre** : `LOT-ANNEXE-NN` est un identifiant indépendant de `LOT-01`…`LOT-55`,
stable une fois attribué (même règle que les lots principaux : jamais réordonné).

**Régime d'entraînement** : chaque lot des générations 2 et 3 entraîne un modèle sur **un seul
niveau à la fois** — pas d'entraînement joint multi-niveaux. La génération 4 mesure ce que ces
modèles niveau-par-niveau donnent en dehors de leur niveau d'origine, à titre de mesure de
référence, pas comme objectif de généralisation.

**Pour apprendre les notions avant d'implémenter** : @ref guide-annexe est un tutoriel autonome
(aucun accès Internet nécessaire), avec les sources citées à la fin de chaque chapitre. Chaque
`epic.md` ci-dessous a sa propre section « Notions abordées » renvoyant au(x) chapitre(s)
concerné(s).

## Génération 0 — Fondations numériques
*Bibliothèque de calcul pure, sans dépendance au jeu, testable en isolation.*
- @subpage lot-annexe-01
- @subpage lot-annexe-02
- @subpage lot-annexe-03
- @subpage lot-annexe-04

## Génération 1 — Pont avec le jeu et observabilité
*Rendre le jeu observable/pilotable et mesurable, sur la base de `Core` sans le modifier.*
- @subpage lot-annexe-05
- @subpage lot-annexe-06
- @subpage lot-annexe-07
- @subpage lot-annexe-08
- @subpage lot-annexe-09

## Génération 2 — Premier agent fonctionnel (évolutionniste)
*Chemin le plus court vers un agent qui termine réellement un niveau — sert de ligne de base.*
- @subpage lot-annexe-10
- @subpage lot-annexe-11

## Génération 3 — Apprentissage par gradient
*Exigence ferme : un vrai modèle appris par rétropropagation, pas seulement une recherche aveugle.*
- @subpage lot-annexe-12
- @subpage lot-annexe-13
- @subpage lot-annexe-14

## Génération 4 — Évaluation et robustesse
- @subpage lot-annexe-15
- @subpage lot-annexe-16

## Génération 5 — Intégration jeu et outillage
- @subpage lot-annexe-17
- @subpage lot-annexe-18
- @subpage lot-annexe-19
- @subpage lot-annexe-20
- @subpage lot-annexe-21
- @subpage lot-annexe-22
