# LOT-ANNEXE-09 — Journalisation CSV des statistiques d'entraînement {#lot-annexe-09}

> Statut : **fait**. Aucun prérequis fonctionnel direct (peut être développé en parallèle
> des autres lots de la génération 1). Dernier lot de la génération 1 : exigence explicite de
> l'utilisateur — en complément de la démo rejouée en jeu, savoir **rapidement** si l'IA a atteint
> son plafond sur le niveau en cours d'entraînement.

## Objectif
Un entraînement (génération 2 : évolutionniste, génération 3 : par gradient) peut durer des milliers
de générations/épisodes sans qu'aucun signal autre que « ça tourne encore » ne soit disponible tant
qu'il n'est pas terminé. L'utilisateur a demandé explicitement — en plus du rejeu final observable
en jeu — un fichier CSV, produit **au fil de l'entraînement**, permettant de répondre en quelques
secondes à « est-ce que ça progresse encore, ou est-ce que ça a atteint un plafond ? » sans attendre
la fin d'un entraînement potentiellement infructueux. Ce lot livre ce mécanisme, une seule fois,
réutilisé sans duplication par tous les algorithmes des générations 2 et 3.

## Périmètre

### Inclus
- **`aisolver::TrainingStatsRecorder`** : interface générique `record(int generationOrEpisodeIndex,
  const TrainingStatsRow& row)`, indépendante de la famille d'algorithme (évolutionniste : une ligne
  par génération de population ; par gradient : une ligne par épisode ou par lot d'épisodes).
- **Colonnes** : index génération/épisode, meilleure récompense, récompense moyenne, récompense
  pire, écart-type, nombre de pas de l'individu/épisode le meilleur, taux de réussite (`Won`) du
  batch, graine utilisée, horodatage, nom du niveau.
- **Moyenne mobile et delta** sur les `N` dernières lignes (colonnes dédiées), calculées et écrites
  au fil de l'eau — pour qu'un plateau soit visible directement dans le fichier, sans traitement
  externe (tableur, script) après coup.
- **Un fichier CSV par run d'entraînement**, sous `/TrainingRuns/<niveau>/<run-id>/stats.csv`
  (dossier racine déjà ajouté à `.gitignore` par le cadrage général du programme — ce lot ne le
  recrée pas, il y écrit).
- Tests : écriture/relecture, stabilité du format de colonnes entre lignes successives.

### Exclus (hors périmètre de ce lot)
- **Visualisation graphique** (courbes, tableau de bord) : le format CSV est choisi précisément pour
  rester consultable par un tableur générique sans outil dédié — une visualisation intégrée est une
  amélioration future possible, non demandée ici (voir aussi la génération 5, qui n'en prévoit pas
  non plus dans son périmètre actuel).
- **Détection automatique de plateau** (arrêt anticipé déclenché par le recorder lui-même) : ce lot
  **expose** les données (moyenne mobile, delta) nécessaires à un humain pour juger d'un plateau ;
  il ne décide jamais lui-même d'arrêter un entraînement — cohérent avec le principe déjà retenu en
  génération 4 (mesurer, jamais décider à la place d'un humain).
- **Agrégation multi-runs** (comparer plusieurs fichiers CSV entre eux) : chaque fichier documente un
  run unique ; la comparaison inter-algorithmes structurée est le rôle du harnais de benchmark
  (`LOT-ANNEXE-15`, génération 4), qui réutilise ce même format de colonnes sans le dupliquer.
- **Compression ou rotation des fichiers CSV** : un entraînement de quelques milliers de lignes
  produit un fichier de taille modeste (quelques centaines de kilo-octets au plus) — aucun besoin de
  gestion de taille identifié.

## Décisions de cadrage
- **Une seule implémentation, réutilisée par les générations 2 et 3, jamais un recorder par
  algorithme.** Sans cette contrainte, comparer un run évolutionniste et un run REINFORCE
  supposerait déjà de réconcilier deux formats de fichier différents avant même de comparer les
  chiffres — le harnais de benchmark (`LOT-ANNEXE-15`) part du principe que tous les CSV du
  programme partagent exactement les mêmes colonnes.
- **Écriture incrémentale, une ligne à la fois, jamais un unique dump final.** Un entraînement
  interrompu (arrêt manuel, plantage) avant la fin doit laisser un fichier CSV exploitable
  jusqu'à la dernière génération/épisode enregistré — c'est précisément le scénario où
  l'utilisateur a le plus besoin de savoir « est-ce que ça valait la peine de continuer ? ».
- **La moyenne mobile et le delta sont calculés par le recorder lui-même, pas laissés à un post-
  traitement.** Documenté explicitement comme la réponse directe à la demande de l'utilisateur : une
  colonne de delta proche de zéro sur une fenêtre récente doit être lisible **immédiatement** en
  ouvrant le fichier, sans script ni tableur à écrire pour l'obtenir.
- **Le nom de fichier/dossier encode le niveau et un identifiant de run (horodatage), jamais un
  compteur global partagé entre niveaux différents.** Cohérent avec le régime d'entraînement niveau
  par niveau (décision transverse du programme) : deux runs sur deux niveaux différents ne se
  marchent jamais dessus, et deux runs successifs sur le **même** niveau restent chacun consultables
  séparément (utile pour comparer un réglage d'hyperparamètres à un autre sur le même niveau).
- **Toute donnée déjà disponible ailleurs (graine, nom de niveau) est dupliquée dans le CSV plutôt
  que déduite du nom de fichier/dossier.** Un fichier CSV doit rester interprétable seul, même sorti
  de son arborescence de dossier (ex. copié ailleurs pour comparaison) — redondance délibérée, pas
  un oubli de normalisation.

## Notions abordées
Voir @ref guide-annexe-apprentissage-renforcement, section 7 (« Pourquoi mesurer un plafond de
performance ») et @ref guide-annexe-evaluation-rl pour le lien avec la mesure rigoureuse d'un
entraînement. Aucune source académique nouvelle : ce lot est un outil d'observabilité, pas une
notion d'apprentissage en soi.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-010`](@ref EX-IA-010).
- Réutilisées : `EX-NFR-040` (erreur récupérable, pas de plantage — écriture de fichier).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-training-stats-recorder.md) | `TrainingStatsRecorder` : interface générique | `Source/AiSolver/Stats` | ✅ |
| [TACHE-02](tache-02-colonnes-csv.md) | Colonnes CSV | `Source/AiSolver/Stats` | ✅ |
| [TACHE-03](tache-03-moyenne-mobile-delta.md) | Moyenne mobile et delta (détection de plateau) | `Source/AiSolver/Stats` | ✅ |
| [TACHE-04](tache-04-fichier-par-run.md) | Un fichier CSV par run, sous `/TrainingRuns/` | `Source/AiSolver/Stats` | ✅ |
| [TACHE-05](tache-05-tests.md) | Tests : écriture/relecture, stabilité du format | `Source/Test/Unit/AiSolver/Stats` | ✅ |

## Critères d'acceptation du lot
1. `TrainingStatsRecorder::record` accepte la même structure de ligne quel que soit l'algorithme
   appelant (évolutionniste ou par gradient), sans branchement spécifique à une famille
   d'algorithme.
2. Le fichier CSV produit reste lisible/cohérent (mêmes colonnes, même ordre) même après un arrêt
   prématuré de l'entraînement (pas de ligne finale spéciale requise pour clôturer le fichier).
3. La colonne de delta sur la moyenne mobile approche `0` lorsque la récompense moyenne cesse de
   progresser sur la fenêtre récente (vérifié sur un scénario synthétique de stagnation).
4. Un run interrompu à la ligne `k` laisse un fichier CSV valide et exploitable contenant
   exactement `k` lignes de données.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Aucune dépendance fonctionnelle amont au sein du programme annexe (peut être développé dès la
génération 1, en parallèle des autres lots). Consommé directement par
[LOT-ANNEXE-10](@ref lot-annexe-10)/[11](@ref lot-annexe-11) (génération 2) et par toute la
génération 3 ([LOT-ANNEXE-12](@ref lot-annexe-12) à [14](@ref lot-annexe-14)), puis réutilisé (format
de colonnes) par le harnais de benchmark de la génération 4 ([LOT-ANNEXE-15](@ref lot-annexe-15)).

## Navigation des tâches
- @subpage lot-annexe-09-tache-01-training-stats-recorder
- @subpage lot-annexe-09-tache-02-colonnes-csv
- @subpage lot-annexe-09-tache-03-moyenne-mobile-delta
- @subpage lot-annexe-09-tache-04-fichier-par-run
- @subpage lot-annexe-09-tache-05-tests
