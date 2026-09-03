# TACHE-06 — Sélection de niveau côté joueur {#lot-59-tache-06-selection-niveau}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
Le menu principal offre quatre entrées, dont un « Jouer » qui ne fait qu'une chose : relancer la
séquence complète depuis le premier tableau. Avec la progression de la `TACHE-05`, ce bouton devient
ambigu — reprend-il ou recommence-t-il ?

Par ailleurs, un joueur ne peut **pas** rejouer un tableau précis, ni jouer un niveau qu'il vient de
créer : la seule façon d'atteindre un niveau arbitraire est l'essai de l'éditeur. L'éditeur possède
pourtant déjà tout ce qu'il faut — `hmi::LevelBrowserPanel` (`LOT-36`) liste, recherche et ouvre les
niveaux d'un dossier.

## Travail à réaliser
- **Remplacer « Jouer »** par trois entrées explicites : *Continuer* (grisée si aucune progression),
  *Nouvelle partie* (avec confirmation si une progression existe), *Choisir un niveau*.
- **Écran de sélection** listant les tableaux de la séquence, avec leur état (terminé, atteint,
  verrouillé). Sont jouables les tableaux **terminés** et le **premier non terminé** ; les suivants
  sont visibles mais verrouillés — sinon la progression qu'on vient d'ajouter ne signifierait rien.
- **Réutiliser `hmi::LevelBrowserPanel`** et `hmi::LevelFileOperations` plutôt que d'écrire un
  second parcours de dossier. Si le panneau est trop lié à l'éditeur, en extraire la partie
  **listage et filtrage** — pas le dupliquer.
- **Accès aux niveaux personnels** : un mode « tous les niveaux du dossier », hors séquence et hors
  progression, pour jouer un niveau créé dans l'éditeur.
- Navigation clavier / souris / manette, libellés traduits.

## Fichiers impactés
- `Source/HMI/Interface/LevelSelectScreen.{h,cpp}` (nouveau), mise en page dans
  `Source/Elements/UI/LevelSelectScreen.ui`.
- `Source/HMI/Interface/MainMenu.{h,cpp}`, `Source/Elements/UI/MainMenu.ui` — trois entrées
  (Continuer/Nouvelle partie/Choisir un niveau) remplacent « Jouer ».
- `Source/HMI/Game/Progression.{h,cpp}` — `hmi::isLevelUnlocked` (fonction pure, règle de
  déverrouillage), à côté de la classe qu'elle consulte.
- `Source/HMI/Game/GameViewport.{h,cpp}` — `startGame` accepte un indice de départ.
- `Source/HMI/Interface/ScreenFlow.{h,cpp}` — `ScreenId::LevelSelect` (page normale, pas un
  recouvrement) et ses transitions (`OpenLevelSelect`/`CloseLevelSelect`/`LevelChosen`).
- `Source/HMI/Interface/MainWindow.{h,cpp}` — câblage complet (voir État).
- `Source/Elements/Localization/{fr,en}.lang` — retrait de `menu.play`, nouvelles clés.
- `Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt` — nouveaux fichiers.
- `Source/Test/Unit/HMI/Game/test_progression.cpp` (étendu : `isLevelUnlocked`, quels tableaux
  sont jouables).
- `Source/Test/Unit/HMI/Interface/test_screen_flow.cpp` (étendu : `ScreenId::LevelSelect`).
- `Source/Test/Unit/HMI/Localization/test_localization.cpp` — `menu.play` retiré, remplacé par
  `menu.continue` dans le test existant.

## Tests (obligatoires)
- Règle de déverrouillage, sous forme de fonction **pure** testée : progression vide → seul le
  premier tableau est jouable ; trois tableaux terminés → les trois plus le quatrième ; séquence
  entièrement terminée → tous.
- *Continuer* est indisponible sans progression, et mène au premier tableau **non terminé** sinon.
- Un tableau verrouillé ne peut pas être lancé, même par le chemin manette.
- Un niveau personnel hors séquence se lance sans toucher à la progression de la séquence.
- Libellés présents dans les deux catalogues.

## Points d'attention
- **La règle de déverrouillage est une fonction pure**, pas une condition disséminée dans le
  widget : c'est elle qu'on teste, l'écran ne fait que l'afficher.
- Jouer un niveau **hors séquence** ne doit rien écrire dans la progression — sans quoi essayer un
  niveau personnel déverrouillerait la campagne.
- Ne pas dupliquer le balayage de dossier : `hmi::LevelBrowserPanel` le fait déjà, et un second
  parcours divergerait au premier changement de convention de nommage.
- Le menu principal est aussi le premier écran vu par un nouveau joueur : trois entrées de jeu plus
  Éditeur, Options et Quitter commencent à faire beaucoup — vérifier la lisibilité à l'essai manuel.

## Définition de fait (DoD)
- Le menu distingue reprendre, recommencer et choisir ; la règle de déverrouillage est pure et
  testée ; un niveau personnel est jouable sans passer par l'éditeur et sans polluer la
  progression ; le balayage de dossier n'est pas dupliqué ; traduit ; `/W4 /WX` propre.

## État
**Réutilisé : `hmi::LevelFileOperations`, pas le widget `hmi::LevelBrowserPanel`.** Ce dernier
porte aussi les commandes d'édition (créer/renommer/dupliquer/supprimer) : les embarquer dans un
écran de sélection **côté joueur** aurait exposé « Supprimer » à quiconque choisit un niveau. Le
balayage de dossier proprement dit est déjà séparé de ce widget dans
`hmi::LevelFileOperations::list()` (LOT-36) : le réutiliser directement satisfait « ne pas
dupliquer le parcours de dossier » sans dupliquer non plus les commandes d'édition. Ce
listage est en outre déjà filtré du préfixe réservé `sequence-` (`TACHE-04`, effet de bord corrigé
sur `LevelFileOperations`), ce dont l'onglet « Niveaux personnels » profite directement.

**Filtrage supplémentaire, pas dans le plan initial** : les niveaux personnels listés excluent
aussi tout fichier qui appartient déjà à la séquence démo (`showGame`/`openLevelSelect` retirent
les noms de `sequence->levels` de la liste). Sans ce filtre, un tableau **verrouillé** dans
l'onglet Séquence serait apparu **librement lançable** dans l'onglet Niveaux personnels — une
fuite de la règle de déverrouillage repérée en concevant l'écran, avant tout essai manuel.

**`ScreenId::LevelSelect` est une page normale** de `_stack` (comme Menu/Options), pas un
recouvrement (comme Pause/NiveauTermine) : atteint depuis le menu, jamais pendant une partie, donc
aucune scène à laisser visible derrière -- pas de synchronisation de géométrie nécessaire.

**Revalidation en profondeur.** `LevelSelectScreen` grise déjà les tableaux verrouillés
(`Qt::ItemIsEnabled` retiré, ce qui exclut aussi l'item de la navigation clavier/manette standard
de Qt) et son gestionnaire d'activation vérifie une seconde fois ce même drapeau avant d'émettre
`sequenceLevelChosen`. `MainWindow::chooseSequenceLevel` revalide une **troisième** fois via
`hmi::isLevelUnlocked`, sur une séquence rechargée à l'instant, avant tout lancement -- aucun de
ces trois niveaux ne fait confiance aux deux autres.

**Bug de régression corrigé avant tout essai manuel** (repéré en relisant le flux, jamais observé
en pratique) : `openLevelComplete` (`TACHE-05`) avançait `Progression::currentLevel` au tableau
suivant celui qui vient d'être réussi, **sans condition**. Rejouer un tableau **déjà terminé plus
ancien** que le tableau atteint (possible depuis cette tâche : les tableaux terminés restent tous
jouables via « Choisir un niveau ») aurait fait **reculer** le tableau atteint vers ce tableau plus
ancien à la prochaine réussite. Corrigé en n'avançant `currentLevel` que si le tableau qui vient
d'être réussi ne l'était **pas déjà** (`Progression::isCompleted` interrogé avant `markCompleted`).

**Étendue non testée automatiquement, réservée à `TACHE-07`** : le câblage Qt de `MainWindow`
(trois boutons du menu, deux onglets, confirmation de nouvelle partie, focus/manette) suit le même
patron que `PauseScreen`/`LevelCompleteScreen` (`TACHE-02`/`TACHE-03`) -- non exercé par un test
automatisé (aucune infrastructure de test Qt pour `MainWindow` dans ce projet), vérifié par
relecture et réservé à l'essai manuel de fin de lot, comme les tâches précédentes.

## Exigences
`EX-IHM-005` (sélection de niveau côté joueur) ; réutilise `EX-LVL-014` (progression),
`EX-LVL-013` (séquence), `EX-IHM-020`/`EX-IHM-021` (panneau de gestion des niveaux),
`EX-REN-030` (menu principal), `EX-REN-033` (traduction), `EX-CTRL-012` (manette et clavier).
