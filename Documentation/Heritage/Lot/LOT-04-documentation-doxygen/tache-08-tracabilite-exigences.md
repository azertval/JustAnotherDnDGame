# TACHE-08 — Traçabilité des exigences (IDs stables, ancres Doxygen, lint CI) {#lot-04-tache-08-tracabilite-exigences}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/Specification/`, CI · **Statut :** fait

## Contexte
Les identifiants d'exigences (`EX-VIS-001`, `EX-ARCH-011`, …) sont référencés à la fois dans les **lots** (epics + tâches) et dans le **code** (commentaires Doxygen de `Core`). Les renuméroter casserait ces références. Un ID d'exigence doit donc être un **identifiant permanent**, pas un marqueur de position. Cette tâche grave cette règle, sécurise les références et outille l'allocation. **Aucun ID existant n'est renuméroté** (y compris `EX-VIS`, laissé tel quel) : à la création d'une exigence, l'utilisateur choisit un numéro libre et le **lint CI garantit l'unicité** (un doublon fait échouer la CI).

## Règle à formaliser (dans conventions.md)
- Un `EX-…` est **immuable** : alloué une fois, **jamais renuméroté**, **jamais réutilisé**.
- Allocation au **prochain numéro libre**.
- Le numéro **n'encode pas l'ordre** : l'ordre de lecture est celui du document, indépendant de l'ID.
- Insérer une exigence = écrire son paragraphe où il doit être + lui donner le prochain ID libre (aucune renumérotation).

## Travail à réaliser
- **Documenter la règle** ci-dessus dans `Documentation/Specification/conventions.md` (nouvelle section « Identifiants d'exigences »), avec la procédure d'insertion.
- **Ancres Doxygen** : déclarer chaque exigence par `\anchor EX-XXX-NNN` à l'endroit où elle est définie (la spec est la source de vérité), et convertir les renvois **intra-documentation** (specs ↔ specs, lots → exigences) en `@ref EX-XXX-NNN` (liens cliquables, vérifiés par `WARN_AS_ERROR` en TACHE-07).
- **Lint CI** (script portable, ex. Python) couvrant **tout le dépôt** (specs, lots, code, workflows) :
  - **doublons** : un même `EX-…` déclaré (`\anchor`) deux fois → erreur.
  - **références orphelines** : un `EX-…` cité (y compris en texte brut dans un commentaire de code) sans `\anchor` correspondant → erreur.
  - **prochain libre** : mode utilitaire affichant le prochain numéro disponible par catégorie (aide à l'allocation).
- Brancher le lint dans la CI (job dédié, bloquant en PR).

## Fichiers impactés
- `Documentation/Specification/conventions.md` (règle + procédure).
- `Documentation/Specification/*.md` (déclaration des `\anchor`, renvois `@ref`).
- Renvois dans les lots (`Documentation/Lot/**`) convertis en `@ref` le cas échéant.
- Script de lint (ex. `scripts/lint_exigences.py`) et son intégration CI (`.github/workflows/`).

## Vérifications (obligatoires)
- Chaque `EX-…` a **exactement une** déclaration `\anchor` ; `@ref` vers un ID inexistant fait échouer la génération Doxygen.
- Le lint échoue sur un ID **en double** et sur une **référence orpheline** (testé avec un cas fabriqué), et réussit sur l'état sain du dépôt.
- Le mode « prochain libre » renvoie un numéro non déjà utilisé pour la catégorie demandée.

## Points d'attention
- **Ne renuméroter aucun ID existant** : l'unicité à la création repose sur la vigilance de l'auteur, **garantie par le lint CI** (échec sur doublon). Le mode « prochain libre » aide à choisir un numéro sûr.
- Le lint doit distinguer une **déclaration** (`\anchor`) d'une **référence** (`@ref` ou texte) pour compter correctement doublons vs orphelins.
- Coordination avec TACHE-07 : `WARN_AS_ERROR` transforme un `@ref` cassé en échec de build ; le lint couvre en plus les références en **texte brut** du code.

## Définition de fait (DoD)
- Règle d'immuabilité documentée ; exigences ancrées et référençables par `@ref` ; lint CI opérationnel (doublons, orphelins, prochain libre) et bloquant.

## Exigences
`EX-NFR-012`, `EX-NFR-013` (esprit « zéro avertissement / références vérifiées »).
