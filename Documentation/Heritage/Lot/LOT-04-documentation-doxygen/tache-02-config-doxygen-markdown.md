# TACHE-02 — Configuration Doxygen pour le Markdown {#lot-04-tache-02-config-doxygen-markdown}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/Doxyfile` · **Statut :** fait

## Contexte
Le `Doxyfile` actuel n'ingère que le code (`INPUT = ../Source`, `FILE_PATTERNS = *.cpp *.h *.hpp`) : aucun fichier Markdown n'est lu et aucune page principale n'est définie, d'où un site sans accueil. Cette tâche configure Doxygen pour publier **le Markdown documentaire en plus du code**.

## Travail à réaliser
- `INPUT` : ajouter les sources Markdown documentaires — `index.md`, `Specification`, `Lot`, `Manuel` — en conservant `../Source`.
- `FILE_PATTERNS` : ajouter `*.md`.
- `USE_MDFILE_AS_MAINPAGE = index.md` (la page d'accueil, cf. TACHE-03).
- `EXCLUDE` / `EXCLUDE_PATTERNS` : exclure `generated/` (sortie Doxygen) pour éviter toute auto-ingestion.
- Vérifier `MARKDOWN_SUPPORT = YES` (défaut) et, si besoin, `EXTENSION_MAPPING` pour `.md`.
- Laisser la référence de code active (`Source/`) : le site combine prose + API.

## Fichiers impactés
- `Documentation/Doxyfile`.

## Vérifications (obligatoires)
- `doxygen Doxyfile` (dans `Documentation/`) génère sans erreur.
- `generated/html/index.html` affiche la page d'accueil (pas la page vide par défaut).
- Les pages Markdown apparaissent dans la navigation ; la référence de code reste présente.
- `generated/` n'est pas ingéré (pas de pages en double issues de la sortie).

## Points d'attention
- Chemins `INPUT` relatifs au dossier `Documentation/` (répertoire de travail de la CI `docs.yml`).
- La hiérarchie de navigation fine (`@page`/`@subpage`) est traitée en TACHE-03/04/05 ; ici on garantit surtout l'**ingestion** et la **mainpage**.
- Ne pas activer `WARN_AS_ERROR` ici (fait en TACHE-07, une fois toutes les pages propres).

## Définition de fait (DoD)
- Doxygen ingère le Markdown documentaire et le code ; page d'accueil affichée ; build local vert.

## Exigences
`EX-NFR-012`.
