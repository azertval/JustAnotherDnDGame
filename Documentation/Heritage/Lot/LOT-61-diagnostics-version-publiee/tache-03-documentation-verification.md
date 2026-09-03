# TACHE-03 — Documentation et vérification {#lot-61-tache-03-documentation-verification}

**Lot :** [LOT-61](epic.md) · **Emplacement :** `Documentation` · **Statut :** non commencé

## Contexte
Un journal que personne ne sait trouver ne sert à rien. Ce lot n'est complet que si un joueur non
développeur peut, sur demande, joindre le bon fichier à un signalement — donc si le **manuel** le
dit, et non seulement le guide du développeur.

## Travail à réaliser
- **Manuel utilisateur** : une courte section dans `Documentation/Manuel/telecharger-et-lancer.md`
  — où se trouve le dossier `Logs/`, ce qu'il contient, quoi joindre en cas de problème, et
  l'assurance qu'aucune donnée ne quitte la machine.
- **Guide du développeur** : compléter `Documentation/Guide/guide-journalisation.md` — le troisième
  sink, la politique de niveau selon la configuration, la rotation et ses deux constantes, le
  contexte de démarrage et pourquoi l'ordre de destruction compte pour le message d'arrêt.
- **Spécifications** : déclarer `EX-NFR-042` en section 5 (Robustesse) de
  `Documentation/Specification/exigences-non-fonctionnelles.md`.
- **`Source/Core/Diagnostics/README.md`** : le dossier annonce ses sinks, il en aura trois.
- **`CHANGELOG.md`**, section *Non publié*.
- **`.gitignore`** : vérifier que `Logs/` produit à l'exécution n'est pas versionnable par accident
  (le motif `*.log` existe déjà — s'assurer qu'il couvre bien l'extension retenue).
- **Vérification manuelle**, au moment prévu par le lot : lancer le zip **Release** sur une machine
  sans outil de développement, jouer quelques minutes, fermer, ouvrir le fichier et vérifier qu'il
  est lisible et contient le contexte.

## Fichiers impactés
- `Documentation/Manuel/telecharger-et-lancer.md`.
- `Documentation/Guide/guide-journalisation.md`.
- `Documentation/Specification/exigences-non-fonctionnelles.md`.
- `Source/Core/Diagnostics/README.md`, `.gitignore`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — `EX-NFR-042` déclarée une fois et référencée.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %, **et** le job Release de [LOT-58](@ref lot-58).
- Essai réel du zip Release : le fichier existe, est lisible, contient version, configuration,
  adaptateur graphique et message d'arrêt.

## Points d'attention
- **La vérification décisive est l'essai du zip**, pas le test unitaire : c'est le seul contexte où
  `NDEBUG`, le déploiement et un dossier d'exécution réel se combinent.
- Le manuel s'adresse à un **non-développeur** : dire où cliquer, pas ce qu'est un sink.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Ne documenter que le livré : si la rotation a été simplifiée, le guide décrit ce qui existe.

## Définition de fait (DoD)
- `EX-NFR-042` est déclarée, le manuel indique où trouver le journal et ce qu'il contient, le guide
  de journalisation décrit le sink fichier et ses pièges, le cahier est régénéré, la CI complète est
  verte, et l'essai du zip Release est fait.

## Exigences
`EX-NFR-042` (déclarée ici) ; réutilise `EX-NFR-012` (conventions), `EX-NFR-020` (tests),
`EX-NFR-022` (CI verte), `EX-NFR-040` (erreur récupérable).
