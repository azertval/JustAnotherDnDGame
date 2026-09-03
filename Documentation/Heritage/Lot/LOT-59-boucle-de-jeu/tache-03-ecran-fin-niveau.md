# TACHE-03 — Écran de fin de niveau et de fin de séquence {#lot-59-tache-03-ecran-fin-niveau}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
Atteindre la sortie enchaîne **instantanément** sur le tableau suivant. Rien ne marque la réussite :
le joueur constate que le décor a changé, sans savoir s'il a gagné ou s'il vient d'être téléporté.
Après le dernier tableau, le retour au menu est tout aussi muet — c'est la seconde moitié de
`EX-REN-031`, et la raison pour laquelle `EX-LVL-011` parle d'un « écran de fin » entre parenthèses
depuis le début.

C'est aussi le seul endroit où un bruitage de victoire (`LOT-60`) et un effet de réussite
(`LOT-53`) auront une place.

## Travail à réaliser
- **Écran de fin de niveau** affiché à la réussite (`EX-GP-030`) : nom du tableau terminé, et deux
  entrées — *Continuer* (tableau suivant) et *Rejouer*. Retour au menu accessible.
- **Écran de fin de séquence** après le dernier tableau : message de fin, retour au menu.
- **Avance explicite.** L'enchaînement automatique de `EX-LVL-011` est conservé comme
  **comportement**, mais passe désormais par cet écran : c'est le joueur qui valide, ou une
  temporisation courte si l'on veut préserver le rythme (à trancher à l'implémentation, une
  constante nommée dans les deux cas — jamais une valeur en dur au milieu du code).
- **Marquer le tableau comme terminé** : l'écran est le point où la progression de la `TACHE-05`
  est mise à jour, une seule fois, avant tout chargement du niveau suivant.
- Confondre ou non les deux écrans est une décision d'implémentation : s'ils partagent tout sauf
  un libellé, un seul écran paramétré suffit — et alors `ScreenId` n'a pas besoin d'un troisième
  état.

## Fichiers impactés
- `Source/HMI/Interface/LevelCompleteScreen.{h,cpp}` (nouveau), mise en page dans
  `Source/Elements/UI/LevelCompleteScreen.ui`.
- `Source/HMI/Game/GameViewport.{h,cpp}` — signalement de la réussite (`levelSucceeded`) au lieu du
  chargement direct du suivant ; `isLastGameLevel`/`currentGameLevelName`/`advanceToNextLevel`
  nouveaux, pour l'écran. La logique de fin (`LevelOutcome::Won`) reste dans `Core` (`GameSession`,
  inchangée) : seul l'**appelant** HMI change de comportement, pas la simulation elle-même.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — câblage, même patron de recouvrement que
  `_pauseScreen` (`TACHE-02`).
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/HMI/CMakeLists.txt` — nouveaux fichiers.

## Tests (obligatoires)
- Une réussite mène à `NiveauTermine`, jamais directement au tableau suivant.
- *Continuer* sur le **dernier** tableau mène à l'écran de fin de séquence, pas à un chargement hors
  bornes — le cas limite le plus probable.
- *Rejouer* recharge le tableau **terminé**, pas le suivant.
- Le tableau est marqué terminé **une seule fois**, même si l'écran est traversé deux fois
  (*Rejouer* puis réussite à nouveau).
- Le test système existant (`test_parcours_complet.cpp`) franchit toujours la séquence complète.
- Libellés présents dans les deux catalogues de traduction.

## Points d'attention
- **Ne pas casser le test système.** Il rejoue les quinze niveaux via `Core` ; l'écran vit dans
  `HMI` et ne doit pas devenir un passage obligé de la logique d'enchaînement testée sans GPU.
- Le dernier tableau est le cas limite : vérifier explicitement l'indice de fin plutôt que de s'en
  remettre à un `+1` non borné.
- Un échec (`EX-GP-031`) ne passe **pas** par cet écran : le redémarrage immédiat est le
  comportement voulu, et le manuel le décrit ainsi (« aucune pénalité au-delà de recommencer »).

## Définition de fait (DoD)
- La réussite d'un tableau et la fin de la séquence sont marquées par un écran, la progression y est
  enregistrée une fois, le dernier tableau ne provoque aucun accès hors bornes, le test système
  passe inchangé ; traduit ; `/W4 /WX` propre.

## État
Un seul écran paramétré (`LevelCompleteScreen::configure(sequenceComplete, levelName)`), comme la
tâche l'autorisait explicitement : `ScreenId::NiveauTermine` n'a pas de troisième état, la table de
transitions de la `TACHE-01` (déjà écrite avec `LevelSucceeded`/`ContinueAfterLevel`/`ReplayLevel`/
`ReturnToMenuFromLevelComplete`) n'a pas eu à bouger. En fin de séquence, *Continuer* et *Rejouer*
sont **masqués** plutôt que menant à un chargement hors bornes : `GameViewport::isLastGameLevel()`
choisit la variante avant même l'ouverture de l'écran, donc le bouton qui mènerait hors bornes
n'existe simplement pas dans ce cas (le garde-fou de `GameViewport::loadGameLevel` sur un indice
hors bornes reste en place, mais n'est plus atteignable par ce chemin).

`GameViewport::tick()` fige la simulation (`pauseSimulation`) avant d'émettre `levelSucceeded`, pour
que la scène reste visible et immobile derrière l'écran (même besoin que la pause, `TACHE-02`) --
et donc le même piège : *Continuer* et *Rejouer* doivent **reprendre** (`resumeSimulation`, qui
réarme `_previousFrame`) avant de charger/recharger, sinon `_paused` reste vrai et la nouvelle
session ne serait jamais avancée par `tick()`. Repéré avant tout essai manuel, en relisant
`restartFromPause` (`TACHE-02`) comme référence -- même correction appliquée ici.

**Marquage de la progression reporté.** Le point où « marquer le tableau comme terminé » (une
tâche du travail à réaliser ci-dessus) doit se brancher est balisé par un commentaire `TODO(LOT-59
TACHE-05)` dans `MainWindow::openLevelComplete` : `hmi::Progression` n'existe pas encore. Le critère
de test « marqué une seule fois, même traversé deux fois » n'est donc pas encore vérifiable et est
réputé de la `TACHE-05`, pas de celle-ci.

**Aucune extension de `test_screen_flow.cpp`** n'a été nécessaire : la `TACHE-01` avait déjà écrit
la table complète pour `NiveauTermine` (transitions et habillage) par anticipation de cette tâche,
donc déjà couverte par ses tests.

**Bug réel trouvé et corrigé à l'essai manuel (`TACHE-07`), partagé avec `_pauseScreen`** : le
recouvrement ne s'affichait pas du tout (personnage figé, écran vide) — un widget Qt frère du
conteneur du viewport ne se dessine jamais de façon fiable par-dessus la fenêtre native qu'il
embarque. `_levelCompleteScreen` est depuis une fenêtre de haut niveau propre, comme
`_pauseScreen` ; voir l'État de `TACHE-02` pour le détail de la cause et de la correction.

## Exigences
`EX-IHM-004` (écran de fin de niveau) ; lève `EX-REN-031` pour sa partie fin de niveau ; réutilise
`EX-GP-030` (succès), `EX-GP-031` (échec, non concerné), `EX-LVL-011` (enchaînement),
`EX-LVL-014` (progression), `EX-REN-033` (traduction), `EX-NFR-021` (test système).
