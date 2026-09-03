# TACHE-04 — Pages de spécification navigables (conventions incluses) {#lot-04-tache-04-pages-specification}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/Specification/` · **Statut :** fait

## Contexte
Les spécifications (`vision` … `decors`, plus `conventions` intégré en TACHE-01) doivent former une **rubrique navigable** du site, et non une liste plate. On les structure sous une page « Spécifications » via `@page`/`@subpage`. Point clé de maintenabilité : **l'ordre d'affichage est porté par la liste `@subpage` de l'index** (source unique), et non par les noms de fichiers — insérer/réordonner une spec ne renumérote rien.

## Travail à réaliser
- Faire de `Specification/README.md` la **page d'index** « Spécifications » (ancre `{#specifications}`) : elle liste les documents en `@subpage`, **dans l'ordre voulu**. Cet ordre est désormais l'unique source de vérité.
- Ajouter à **chaque** fichier de spécification un en-tête Doxygen : ancre de page **stable** (`{#spec-vision}`, `{#spec-gameplay}`, … `{#spec-conventions}`) et titre. Les ancres, indépendantes du nom de fichier et de l'ordre, garantissent des liens qui ne cassent jamais.
- Vérifier que `conventions.md` apparaît dans la rubrique (sans numéro).
- Convertir les liens croisés entre specs pour viser les **ancres** (`@ref spec-xxx`) plutôt que des noms de fichiers, afin qu'ils survivent aux renommages/réordonnancements.
- Préserver les renvois `EX-…` (identifiants d'exigences).
- (Optionnel, si retenu) Ajouter une exigence `EX-NFR` « documentation publiée et navigable » dans la spec des exigences non fonctionnelles.

## Convention d'insertion d'une nouvelle spec (à documenter dans l'index)
1. Créer `Documentation/Specification/<nom>.md` avec un en-tête `{#spec-<nom>}` + titre.
2. Ajouter une ligne `@subpage spec-<nom>` **à la position voulue** dans `README.md`.
   → Aucune renumérotation, aucun lien à corriger.

## Fichiers impactés
- `Documentation/Specification/*.md` (en-têtes de page + ancres stables).
- `Documentation/Specification/README.md` (page d'index de la rubrique, source de l'ordre).

## Vérifications (obligatoires)
- La rubrique « Spécifications » liste tous les documents (vision → decors + conventions) **dans l'ordre de l'index** et s'ouvre depuis l'accueil.
- Réordonner deux `@subpage` dans l'index change l'ordre du site **sans** toucher aux fichiers ni aux liens (test de non-régression de la maintenabilité).
- Chaque sous-page a un titre correct et est atteignable via la navigation (`@subpage`).
- Aucun avertissement Doxygen (ancres, liens).

## Points d'attention
- Ne pas dénaturer le contenu des specs : on ajoute **structure et ancres**, pas de réécriture.
- Ancres de page (`{#spec-…}`) **stables et indépendantes de l'ordre** : c'est ce qui rend les liens robustes aux insertions/réordonnancements.

## Définition de fait (DoD)
- Rubrique Spécifications hiérarchique, conventions comprises, navigable et sans avertissement.

## Exigences
`EX-NFR-012`.
