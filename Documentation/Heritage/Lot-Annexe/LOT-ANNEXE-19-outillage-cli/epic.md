# LOT-ANNEXE-19 — Outillage CLI {#lot-annexe-19}

> Statut : **fait**. Regroupe l'ensemble des générations 2 à 4 sous une interface en ligne
> de commande unique. Deuxième lot de la génération 5.

## Objectif
Chaque algorithme d'entraînement (génération 2, 3), le harnais de benchmark et l'export de rejeu
existent, à ce stade du programme, comme des bibliothèques C++ (`Source/AiSolver`) sans point
d'entrée exécutable direct — les utiliser suppose d'écrire un petit programme `main` ad hoc à chaque
fois. Ce lot rassemble ces capacités déjà écrites sous une interface en ligne de commande unique,
sans réimplémenter la moindre logique d'entraînement ou d'évaluation : un exécutable, plusieurs
sous-commandes, chacune un mince habillage autour d'une API déjà cadrée par un lot antérieur.

## Périmètre

### Inclus
- **Exécutable `aisolver-cli`** (`Source/AiSolver/Cli`), sous-commandes :
  - `train --level <chemin> --algo <evo|pg|ac|avance> [--seed <graine>] [--config <fichier>]` :
    construit une `LevelTrainingSession` (`LOT-ANNEXE-11`) ou l'équivalent de génération 3 selon
    `--algo`, journalise via `TrainingStatsRecorder` (`LOT-ANNEXE-09`, chemin résolu par
    `makeTrainingRunPath`, `LOT-ANNEXE-09` TACHE-04).
  - `evaluate --model <chemin> --level <chemin> [--repetitions N]` : construit le `TrainedPolicy`
    adapté (`LOT-ANNEXE-15`) et appelle `BenchmarkRunner::run`, affiche/écrit le résultat.
  - `export-replay --model <chemin> --level <chemin> --output <chemin>` : rejoue le modèle en mode
    déterministe (`decodeArgmax`, `LOT-ANNEXE-07`) et écrit un fichier de rejeu (`aisolver::
    writeReplay`, `LOT-ANNEXE-07`/`17`).
- **Configuration des hyperparamètres** : fichier JSON optionnel ou arguments individuels
  (taille de population, taux de mutation, taux d'apprentissage, `gamma`, etc. selon la
  sous-commande) — toujours journalisée dans les métadonnées produites (CSV, fichier de rejeu),
  pour qu'un run passé reste reproductible à partir de ses seules traces écrites.

### Exclus (hors périmètre de ce lot)
- **Toute nouvelle logique d'entraînement, d'évaluation ou d'export** : ce lot n'est qu'une
  interface — toute règle métier vit déjà dans les lots amont (générations 2 à 4, `LOT-ANNEXE-07`/
  `17`). Un bug de comportement d'entraînement ne se corrige jamais dans `Source/AiSolver/Cli`.
- **Interface graphique** : ligne de commande uniquement, cohérent avec un outil destiné à
  l'expérimentation (entraînements potentiellement longs, lancés en tâche de fond) plutôt qu'à un
  usage interactif.
- **Orchestration multi-runs** (lancer automatiquement un entraînement sur toute la séquence
  `demo-*.json`) : chaque invocation de `train` porte sur un seul niveau (régime d'entraînement
  niveau par niveau, décision transverse) — enchaîner plusieurs invocations reste un geste manuel
  ou scripté par l'utilisateur en dehors de `aisolver-cli` lui-même.
- **Auto-complétion shell, distribution packagée** : hors de portée d'un outil d'expérimentation
  interne, sans utilisateur externe identifié.

## Décisions de cadrage
- **Chaque sous-commande est un habillage mince, jamais un second point d'entrée vers une logique
  dupliquée.** `train`/`evaluate`/`export-replay` construisent et appellent directement les types
  déjà cadrés (`LevelTrainingSession`, `BenchmarkRunner`, `writeReplay`) — aucune règle de décision
  (critère d'arrêt, calcul de fitness, format de sortie) n'est réimplémentée dans `Cli`.
- **Toute configuration effectivement utilisée pour un run est journalisée**, jamais implicite :
  un `train` lancé sans `--config` utilise des valeurs par défaut documentées, mais ces valeurs
  concrètes (pas seulement leur absence) apparaissent dans les métadonnées du CSV/rejeu produit —
  condition de reproductibilité stricte d'un run passé, y compris pour un lancement futur qui
  cherche à le reproduire sans se souvenir des valeurs par défaut de l'époque.
- **`aisolver-cli` ne lance jamais deux runs concurrents dans le même processus** : un run
  d'entraînement bloque jusqu'à son critère d'arrêt (résolution ou plafond de générations,
  `LOT-ANNEXE-11`) — cohérent avec la boucle mono-thread déjà actée pour tout le programme
  (`LOT-ANNEXE-01`).
- **Le format de configuration (fichier JSON optionnel) reste propre à `AiSolver`**, sans lien avec
  le format de niveau ou de rejeu — un fichier de configuration décrit des hyperparamètres
  d'entraînement, jamais un niveau ni une séquence d'actions, pour ne pas mélanger des
  préoccupations distinctes dans un même schéma.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot est un habillage en ligne de commande
des capacités déjà cadrées par les lots amont — voir leurs propres sections « Notions abordées »
respectives pour les sources pertinentes selon la sous-commande utilisée.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-020`](@ref EX-IA-020).
- Réutilisées (inchangées) : les exigences de tous les lots exposés — non renumérotées ici.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-sous-commandes.md) | Sous-commandes `train`/`evaluate`/`export-replay` | `Source/AiSolver/Cli` | ✅ |
| [TACHE-02](tache-02-configuration-tracabilite.md) | Configuration des hyperparamètres et traçabilité | `Source/AiSolver/Cli` | ✅ |

## Critères d'acceptation du lot
1. `aisolver-cli train --level <chemin> --algo evo` lance un entraînement évolutionniste complet
   sur le niveau donné et produit un CSV (`LOT-ANNEXE-09`) et, en cas de résolution, un fichier de
   rejeu (`LOT-ANNEXE-11`/`07`).
2. `aisolver-cli evaluate` produit un `BenchmarkResult`/rapport identique à celui obtenu par un
   appel direct à `BenchmarkRunner::run` (`LOT-ANNEXE-15`) dans les mêmes conditions.
3. `aisolver-cli export-replay` produit un fichier de rejeu valide (`aisolver::validateReplay`
   renvoie `std::nullopt`) pour tout modèle entraîné fourni en entrée.
4. Deux invocations de `train` avec les mêmes arguments et la même graine produisent des résultats
   strictement identiques (reproductibilité, héritée des lots amont, vérifiée ici de bout en bout à
   travers la CLI).
5. La configuration effective d'un run (valeurs par défaut incluses) apparaît dans les métadonnées
   du CSV et/ou du fichier de rejeu produits.
6. Aucune logique d'entraînement, d'évaluation ou d'export n'est dupliquée dans `Source/AiSolver/
   Cli` (vérifiable par revue : chaque sous-commande délègue à un type déjà cadré par un lot amont).
7. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Expose, sans les dupliquer, les capacités de [LOT-ANNEXE-10](@ref lot-annexe-10)/[11](@ref
lot-annexe-11) (génération 2), [LOT-ANNEXE-12](@ref lot-annexe-12) à [14](@ref lot-annexe-14)
(génération 3), [LOT-ANNEXE-15](@ref lot-annexe-15) (harnais de benchmark), [LOT-ANNEXE-09](@ref
lot-annexe-09) (`TrainingStatsRecorder`) et [LOT-ANNEXE-07](@ref lot-annexe-07)/[17](@ref
lot-annexe-17) (format de rejeu).

## Navigation des tâches
- @subpage lot-annexe-19-tache-01-sous-commandes
- @subpage lot-annexe-19-tache-02-configuration-tracabilite
