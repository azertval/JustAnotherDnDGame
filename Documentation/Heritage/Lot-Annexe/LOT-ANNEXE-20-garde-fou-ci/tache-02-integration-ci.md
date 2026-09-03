# TACHE-02 — Intégration à la CI existante {#lot-annexe-20-tache-02-integration-ci}

**Lot :** [LOT-ANNEXE-20](epic.md) · **Emplacement :** `.github/workflows` · **Statut :** fait

## Contexte
`scripts/check_ai_replays.py` (TACHE-01) existe et fonctionne en local ; cette tâche le fait
exécuter automatiquement sur chaque Pull Request, à côté des vérifications déjà en place
(`lint_exigences.py`, `check_demo_sequence.py`, `generate_cahier_test.py --check`).

## Travail à réaliser
- **`.github/workflows/ci.yml`** : nouvelle étape, positionnée à côté des étapes existantes
  `lint_exigences.py`/`check_demo_sequence.py` (même job, même style d'invocation — `python
  scripts/check_ai_replays.py`, échec de l'étape si code de sortie non nul).
- Mise à jour du `README.md` (section « Vérifications locales », qui liste déjà les scripts
  équivalents) : ajout de la ligne `python scripts/check_ai_replays.py` à la liste des contrôles
  reproductibles localement.

## Fichiers impactés
- `.github/workflows/ci.yml` — modifié (nouvelle étape).
- `README.md` — modifié (section « Vérifications locales »).

## Tests (obligatoires)
- **Exécution locale identique à la CI** : la commande ajoutée à `ci.yml` est exactement celle
  documentée dans `README.md` et testée manuellement en TACHE-01 — aucune divergence d'invocation
  (arguments, répertoire de travail) entre local et CI.
- **Échec de Pull Request simulé** : sur une branche de test introduisant volontairement un rejeu
  périmé (modification d'un niveau référencé par un rejeu publié), le job CI échoue à cette étape
  précisément, avec le message produit par TACHE-01 visible dans les logs — vérifié une fois en
  conditions réelles avant fusion de ce lot, pas un test automatisé au sens `ctest` (validation de
  configuration CI, pas de code C++).

## Points d'attention
- **La nouvelle étape ne doit pas dépendre d'une étape de build C++ préalable** (cohérent avec
  TACHE-01 : le script ne lit que des fichiers texte) — elle peut s'exécuter tôt dans le job,
  parallèlement ou avant les étapes de compilation, pour échouer vite si un rejeu est périmé.
- **Aucune modification des étapes existantes** (`lint_exigences.py`, `check_demo_sequence.py`,
  build, tests) — ce lot ajoute une étape, il n'en réordonne ni n'en modifie aucune autre.

## Définition de fait (DoD)
- Étape CI ajoutée et vérifiée en conditions réelles (échec provoqué puis corrigé sur une branche
  de test) ; `README.md` à jour ; `EX-IA-021` déclarée dans l'`epic.md` du lot.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-021` (nouvelle, du même lot).
