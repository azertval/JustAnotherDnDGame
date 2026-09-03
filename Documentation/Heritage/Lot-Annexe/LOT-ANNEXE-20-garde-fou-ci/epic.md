# LOT-ANNEXE-20 — Garde-fou CI {#lot-annexe-20}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-17](@ref lot-annexe-17) (validation de rejeu,
> empreinte FNV-1a) et [LOT-ANNEXE-18](@ref lot-annexe-18) (rejeux réellement versionnés/joués en
> jeu). [LOT-ANNEXE-21](@ref lot-annexe-21) a depuis été ajouté après celui-ci.

## Objectif
`LOT-ANNEXE-17` garantit qu'un rejeu **chargé par le jeu** est refusé s'il ne correspond plus à son
niveau d'origine — mais cette vérification ne s'exécute qu'à l'exécution du jeu, jamais en intégration
continue. Un niveau modifié par un commit ultérieur (rééquilibrage, correction) peut ainsi rendre un
rejeu versionné silencieusement obsolète pendant des semaines, sans qu'aucune Pull Request ne le
signale — jusqu'à ce qu'un joueur ou un testeur tente de le lancer et le voie échouer. Ce lot ferme
cette fenêtre, sur le modèle exact de `scripts/check_demo_sequence.py` (déjà existant, qui compare
deux endroits du code source pour détecter une désynchronisation de séquence de niveaux) : un script
Python autonome, exécuté en CI à chaque Pull Request, qui détecte cette péremption **avant** qu'elle
n'atteigne `main`.

## Périmètre

### Inclus
- **`scripts/check_ai_replays.py`** : énumère les fichiers de rejeu versionnés (dossier de rejeux
  publiés introduit par `LOT-ANNEXE-18`, TACHE-02), pour chacun recalcule l'empreinte FNV-1a
  (réimplémentation Python pure, **même algorithme** que `aisolver::computeLevelFingerprint`,
  `LOT-ANNEXE-17`) du fichier de niveau qu'il référence, et la compare à celle enregistrée dans le
  rejeu — échec du script (code de sortie non nul, message explicite par rejeu périmé) en cas de
  divergence ou de niveau introuvable.
- **Intégration à `ci.yml`** : nouvelle étape appelant ce script sur chaque Pull Request vers
  `main`, au même niveau que les étapes existantes (`lint_exigences.py`, `check_demo_sequence.py`).
- **Vérification locale** : le script s'exécute depuis la racine du dépôt, sans argument, comme
  tous les scripts de `scripts/` déjà en place (`python scripts/check_ai_replays.py`).

### Exclus (hors périmètre de ce lot)
- **Réparation automatique d'un rejeu périmé** (ré-export automatique déclenché par la CI) : un
  rejeu périmé est signalé, jamais régénéré automatiquement — la CI n'a pas accès à un entraînement
  complet (potentiellement long) dans le temps d'une vérification de Pull Request ; la remédiation
  reste un geste manuel (`aisolver-cli export-replay`, `LOT-ANNEXE-19`).
- **Vérification de la logique d'entraînement ou de l'exactitude d'un rejeu au-delà de sa
  correspondance avec le niveau référencé** (ex. rejouer réellement le rejeu en CI pour vérifier
  qu'il aboutit toujours à `Won`) : coûteux (nécessiterait de builder et exécuter `AiSolver`/`HMI`
  en CI pour chaque rejeu, à chaque Pull Request) et redondant avec le test système dédié
  (`LOT-ANNEXE-18`, TACHE-03, qui couvre déjà au moins un rejeu de référence de bout en bout) — ce
  garde-fou se limite à la vérification d'empreinte, bon marché et suffisante pour détecter le cas
  visé (niveau modifié après export).
- **Extension du script à d'autres artefacts générés** (modèles entraînés, CSV de statistiques) :
  ces fichiers vivent sous `/TrainingRuns/`, non versionné (`.gitignore`) — aucun artefact
  d'entraînement brut n'est jamais commité, seul un rejeu explicitement publié (`LOT-ANNEXE-18`,
  dossier de rejeux publiés, versionné) a besoin de ce garde-fou.

## Décisions de cadrage
- **Réimplémentation Python pure de FNV-1a, pas d'appel à un binaire C++ compilé.** Cohérent avec
  le style déjà établi de `scripts/` (Python autonome, aucune dépendance de build C++ pour les
  vérifications de CI rapides) — `LOT-ANNEXE-17` documente déjà explicitement la convention
  d'octets (UTF-8 brut, aucune normalisation) précisément pour que cette réimplémentation reste
  possible sans ambiguïté ; ce lot en est la mise en pratique directe.
- **Le script suit le patron exact de `scripts/check_demo_sequence.py`** : fonction `check(root)`
  retournant un code de sortie, liste des problèmes imprimée un par ligne, `main()` minimal — pour
  qu'un contributeur déjà familier d'un script existant du dépôt comprenne immédiatement celui-ci.
- **Une divergence d'empreinte est un échec bloquant de Pull Request**, jamais un avertissement
  silencieux : cohérent avec le traitement des autres garde-fous du dépôt (`lint_exigences.py`,
  `check_demo_sequence.py`), qui font tous échouer la CI plutôt que d'émettre un simple message.
- **Le script ne dépend d'aucune bibliothèque tierce Python** (implémentation manuelle de FNV-1a,
  lecture de fichiers JSON via la bibliothèque standard `json`) — cohérent avec l'esprit général
  « aucune nouvelle dépendance » du programme, étendu ici à l'outillage CI lui-même.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot est un script de garde-fou d'ingénierie
logicielle (CI), réutilisant l'algorithme de hachage déjà motivé dans
[LOT-ANNEXE-17](@ref lot-annexe-17).

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-021`](@ref EX-IA-021).
- Réutilisées (inchangées) : `EX-IA-018` (empreinte de niveau, algorithme FNV-1a, convention
  d'octets), principe déjà établi par `scripts/check_demo_sequence.py` (non normalisé sous un
  identifiant `EX-…`, garde-fou de convention plutôt qu'exigence produit).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-check-ai-replays.md) | `scripts/check_ai_replays.py` | `scripts` | ✅ |
| [TACHE-02](tache-02-integration-ci.md) | Intégration à la CI existante | `.github/workflows` | ✅ |

## Critères d'acceptation du lot
1. `python scripts/check_ai_replays.py`, exécuté depuis la racine du dépôt sur un ensemble de
   rejeux valides, se termine avec un code de sortie `0` et un message de succès (sur le modèle de
   `check_demo_sequence.py`).
2. Le même script, sur un rejeu dont le fichier de niveau référencé a été modifié après l'export
   (contenu différent), se termine avec un code de sortie non nul et un message identifiant
   précisément le rejeu périmé.
3. Le même script, sur un rejeu référençant un niveau introuvable, se termine avec un code de
   sortie non nul et un message explicite distinct du cas précédent.
4. L'empreinte calculée par ce script, sur un même fichier de niveau, est **strictement identique**
   à celle calculée par `aisolver::computeLevelFingerprint` (`LOT-ANNEXE-17`) — vérifié par un cas
   de test partagé (même fichier de niveau, même valeur attendue documentée dans les deux
   implémentations).
5. La CI (`ci.yml`) échoue si ce script échoue, exactement comme pour `lint_exigences.py` et
   `check_demo_sequence.py` déjà en place.

## Dépendances
Réutilise l'algorithme et la convention d'octets de [LOT-ANNEXE-17](@ref lot-annexe-17) (empreinte
FNV-1a). Suppose l'existence de rejeux réellement versionnés par [LOT-ANNEXE-18](@ref
lot-annexe-18) (dossier de rejeux publiés) pour avoir un objet à vérifier. Aucun lot ultérieur n'en
dépend (`LOT-ANNEXE-21` s'appuie sur `LOT-ANNEXE-18`/`19`, pas sur ce garde-fou CI).

## Navigation des tâches
- @subpage lot-annexe-20-tache-01-check-ai-replays
- @subpage lot-annexe-20-tache-02-integration-ci
