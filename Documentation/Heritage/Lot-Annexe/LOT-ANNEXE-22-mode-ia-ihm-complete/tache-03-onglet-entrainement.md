# TACHE-03 — Onglet Entraînement complet {#lot-annexe-22-tache-03-onglet-entrainement}

**Lot :** [LOT-ANNEXE-22](epic.md) · **Emplacement :** `Source/HMI`, `Source/Elements/UI` ·
**Statut :** fait

## Contexte
Trois défauts distincts se cumulaient dans cet onglet.

**Un champ qui ne pilotait rien.** `TrainingWorker` recopiait la saisie « Épisodes / générations
max » dans `config.episodes`, que l'algorithme évolutionniste n'utilise pas : son plafond vit dans
`config.stopping`, laissé aux défauts. Un run évolutionniste allait donc à 200 générations quelle
que soit la valeur affichée — écart si visible qu'il fallait le documenter dans le manuel, et que
la barre de progression était forcée en indéterminé faute de plafond connu.

**Des indicateurs morts.** `stabilityValue`, `dqnEpsilonCurrentValue` et `trainingEtaLabel` étaient
posés dans le `.ui` sans qu'aucune ligne de C++ ne les référence : ils affichaient « — » pour
toujours.

**Des mesures jetées.** `TrainingStatsRow` porte neuf champs et le `stats.csv` du run les écrit
tous ; le signal `progress` n'en transportait que quatre, et le tableau n'en affichait que quatre.

## Travail à réaliser
- **`TrainingProgress`** (`HMI/Ai/TrainingWorker.h`) : remplace les quatre paramètres du signal
  `progress` par une structure portant toute la ligne journalisée, plus `epsilon` (chemin DQN) et
  le couple stabilité/seuil (chemin évolutionniste). Enregistrée auprès du système de métatypes de
  Qt : le signal traverse une frontière de thread.
- **`TrainingRequest`** élargi aux cinq nouveaux hyperparamètres, et **propagé dans
  `CommandLineOverrides` par désignateurs** — c'est la correction du premier défaut ci-dessus.
- **Ordre d'émission** : les statistiques et la stabilité d'une même génération arrivent par deux
  rappels successifs. La ligne est retenue et n'est émise qu'après le second, sans quoi l'écran
  afficherait la stabilité de la génération précédente.
- **`.ui`** : taille de couche cachée et dossier des runs (groupe Commun) ; taille du tournoi,
  force de mutation, générations max, réussites stables exigées (groupe Évolutionniste) ; tableau
  porté à huit colonnes ; boutons de presets. Le libellé « Épisodes / générations max » redevient
  « Épisodes », le plafond ayant désormais son propre champ.
- **Valeurs par défaut du `.ui`** alignées sur les constantes de `EvolutionaryConfig` (population
  `32`, mutation `0,05`) : l'écran au démarrage doit décrire le run de `aisolver-cli train` sans
  option.
- **Presets** : « Charger », « Enregistrer », « Réinitialiser aux défauts », branchés sur
  `loadTrainingConfig`/`writeTrainingConfigJson` — jamais une sérialisation écrite à la main. La
  correspondance champ ↔ hyperparamètre est écrite **une** fois (`configFromForm` /
  `applyConfigToForm`), partagée par le lancement d'un run, les presets et la reprise des réglages
  d'un run passé.
- **Temps restant estimé** : calculé côté IHM (une durée n'a de sens qu'affichée), par moyenne
  cumulée du temps par étape plutôt que par écart instantané — le coût d'une génération varie trop
  pour qu'un écart instantané ne fasse pas osciller l'affichage.
- **`MainWindow::confirmLeavingActiveTraining`** : premier appelant de
  `AiModeScreen::trainingActive()`. « Voir en jeu » pendant un run n'est **pas** entravé — c'est le
  suivi en direct voulu par `LOT-ANNEXE-21`.

## Critères de validation
- Un run évolutionniste à « Générations max » = `N` s'arrête à `N`, barre de progression à l'appui.
- Les trois indicateurs évoluent pendant un run de la famille concernée.
- Le tableau affiche les huit colonnes du `stats.csv`.
- Enregistrer puis recharger un preset restitue exactement les mêmes champs.
