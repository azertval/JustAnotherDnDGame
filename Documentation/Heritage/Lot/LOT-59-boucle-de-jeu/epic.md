# LOT-59 — Boucle de jeu complète : pause, fin de niveau, progression {#lot-59}

> Statut : **code, tests et documentation faits ; essai manuel humain final restant** (voir État de
> [TACHE-07](tache-07-documentation-verification.md)). Prérequis : [LOT-38](@ref lot-38) (IHM Qt
> unifiée), [LOT-52](@ref lot-52) (texte dans la scène). **Premier lot de contenu** du programme
> `0.1.0`, après le durcissement du [LOT-58](@ref lot-58).

## Objectif
Faire du moteur un **jeu** : une partie qu'on peut mettre en pause, dont on voit la fin d'un
tableau, et qu'on retrouve là où on l'avait laissée.

Trois manques se tiennent, et c'est pour ça qu'ils forment un seul lot :

1. **Aucune pause.** Échap quitte immédiatement vers le menu, en pleine partie, sans confirmation.
   Il n'existe aucun moyen de suspendre une partie.
2. **Aucun écran de fin de niveau.** L'enchaînement à la réussite est instantané : le joueur ne
   sait pas qu'il a terminé un tableau, seulement que le décor a changé.
3. **Aucune progression.** « Jouer » relance toujours la séquence des quinze niveaux **depuis le
   premier**, et la séquence est un **littéral C++** (`MainWindow::startGame`). Quitter perd tout ;
   rejouer un tableau précis est impossible ; jouer un niveau qu'on vient de créer suppose de
   passer par l'essai de l'éditeur.

Les deux premiers points sont écrits dans les spécifications depuis longtemps, marqués ⚠️ :
`EX-REN-031` (« non implémenté ») et `EX-GP-040` (« partiellement implémenté »). Ce lot les lève.

C'est le premier lot de **contenu** du programme parce que tous les autres se voient à travers lui : un bruitage de
victoire (`LOT-60`) ou un effet de mort (`LOT-53`) n'ont pas de place où exister tant qu'il n'y a ni
écran de fin, ni pause.

## Périmètre

### Inclus
- **États d'écran manquants** : `Pause` et `NiveauTermine` ajoutés à `hmi::ScreenId`, avec des
  transitions explicites et unidirectionnelles (`EX-GP-041`). Échap **ouvre la pause** au lieu de
  quitter ; quitter devient un choix du menu de pause.
- **Écran de pause** : Reprendre, Recommencer le niveau, Options, Quitter vers le menu. La
  simulation est **suspendue**, pas ralentie : aucun pas de temps fixe n'est consommé en pause.
- **Écran de fin de niveau** entre deux tableaux, et **écran de fin de séquence** après le dernier.
- **Séquence de niveaux en donnée de contenu** : sortie du littéral de `MainWindow.cpp`, vers un
  fichier de `Source/Elements/Levels` (`EX-LVL-013`).
- **Progression persistée** entre deux lancements (`EX-LVL-014`) : dernier niveau atteint et
  niveaux terminés, dans un fichier de réglages à côté de `Settings/keybindings.json`.
- **Sélection de niveau côté joueur** (`EX-IHM-005`) : Continuer / Nouvelle partie / Choisir un
  niveau, ce dernier limité aux niveaux déjà atteints plus le suivant.
- Le tout **navigable au clavier, à la souris et à la manette**, comme le reste de l'IHM.

### Exclus (hors périmètre de ce lot)
- Sauvegarde en cours de niveau (points de contrôle, état exact du personnage) : la granularité est
  le **tableau**, pas l'instant.
- Profils de joueur multiples, statistiques, chronomètre, score, classement.
- Refonte générale de `hmi::MainWindow` ou de `hmi::GameViewport` : on extrait ce que ce lot
  touche — la machine à états d'écran — et rien de plus (cf. décisions de cadrage).
- Écran-titre animé, transitions visuelles élaborées entre écrans.

## Décisions de cadrage
- **Un seul lot pour les trois manques.** Ils partagent la même machine à états et le même écran de
  fin de niveau : les livrer séparément supposerait d'écrire deux fois la navigation, puis de la
  défaire.
- **Pause = suspension du pas fixe, pas un facteur de temps.** Multiplier `dt` par zéro laisserait
  la boucle consommer des pas et ferait dériver tout ce qui compte en pas (animations, dangers
  temporisés, budgets). L'accumulateur de `core::FixedTimestep` n'est simplement pas alimenté.
- **La progression est un réglage, pas une sauvegarde de partie.** Elle suit le patron déjà établi
  par `hmi::GameKeyBindings`/`EditorKeyBindings` : JSON à côté de l'exécutable, lecture tolérante,
  fichier absent = partie neuve. Aucun nouveau format, aucun nouveau mécanisme de persistance.
- **La séquence sort du code.** Un littéral C++ énumérant du contenu oblige à recompiler pour
  réordonner un tableau, et a déjà nécessité un script de garde-fou
  (`scripts/check_demo_sequence.py`) pour rester synchronisé avec le test système. En donnée, le
  script vérifie un fichier plutôt qu'un littéral — même garantie, sans recompilation.
- **Extraction minimale de `MainWindow`.** `MainWindow.cpp` fait 1413 lignes et `GameViewport.h`
  déclare 43 membres : ce lot ajoute des écrans, donc la tentation de tout réorganiser est réelle.
  Elle est écartée. On extrait la **navigation entre écrans** parce qu'on la modifie de toute
  façon ; le reste est laissé en l'état. Un refactoring de confort, non demandé et non couvert par
  les tests d'IHM, est exactement le genre de chantier qui casse une version.
- **La sélection de niveau ne déverrouille pas tout.** Proposer les quinze tableaux d'emblée
  annulerait la progression qu'on vient d'ajouter.

## Exigences couvertes
- Nouvelles : `EX-IHM-004` (écrans de pause et de fin de niveau), `EX-IHM-005` (sélection de niveau
  côté joueur), `EX-LVL-013` (séquence en donnée de contenu), `EX-LVL-014` (progression persistée).
- Levées : `EX-REN-031` (⚠️ non implémenté → livré), `EX-GP-040` (⚠️ partiellement implémenté →
  livré).
- Réutilisées : `EX-GP-041` (machine à états), `EX-GP-030`/`EX-GP-031`/`EX-GP-032` (fin de niveau,
  échec, redémarrage), `EX-LVL-010`/`EX-LVL-011` (ordre des niveaux, enchaînement), `EX-REN-033`
  (catalogue de traduction), `EX-IHM-001` (interface hors-jeu en Qt), `EX-CTRL-012` (remappage),
  `EX-NFR-040` (erreur récupérable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-machine-etats-ecrans.md) | Machine à états d'écran : `Pause` et `NiveauTermine`, transitions explicites | `Source/HMI/Interface` | ✅ |
| [TACHE-02](tache-02-ecran-pause.md) | Écran de pause et suspension réelle du pas fixe | `Source/HMI/Interface`, `Source/HMI/Game` | ✅ |
| [TACHE-03](tache-03-ecran-fin-niveau.md) | Écran de fin de niveau et écran de fin de séquence | `Source/HMI/Interface` | ✅ |
| [TACHE-04](tache-04-sequence-en-donnee.md) | Séquence de niveaux en donnée de contenu | `Source/Elements/Levels`, `Source/Core/Levels` | ✅ |
| [TACHE-05](tache-05-progression-persistee.md) | Progression persistée entre deux lancements | `Source/HMI/Game` | ✅ |
| [TACHE-06](tache-06-selection-niveau.md) | Sélection de niveau côté joueur (Continuer / Nouvelle / Choisir) | `Source/HMI/Interface` | ✅ |
| [TACHE-07](tache-07-documentation-verification.md) | Documentation et vérification | `Source/Test`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Échap en cours de partie ouvre un écran de **pause** ; Reprendre restitue la partie **exactement**
   dans l'état où elle était (position, vitesse, budgets, phase des dangers temporisés).
2. Aucun pas de simulation n'est consommé pendant la pause — asserté par test sur le compteur de pas.
3. Terminer un tableau affiche un écran de fin de niveau ; terminer le dernier affiche un écran de
   fin de séquence, puis revient au menu.
4. La séquence jouée provient d'un **fichier de contenu** ; plus aucun nom de niveau n'est écrit
   dans `Source/HMI`.
5. Quitter l'application en cours de partie puis la relancer propose **Continuer** au bon tableau.
6. Un fichier de progression absent, vide ou corrompu donne une partie neuve, sans erreur bloquante.
7. Tous les nouveaux écrans sont navigables clavier / souris / manette et entièrement traduits
   (fr/en), sans libellé codé en dur.
8. `EX-REN-031` et `EX-GP-040` ne portent plus de mention ⚠️ dans les spécifications.
9. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Bâtit sur [LOT-38](@ref lot-38) (IHM Qt unifiée, écrans et navigation manette) et
[LOT-36](@ref lot-36) (`LevelBrowserPanel`, réutilisé pour la sélection de niveau). Prérequis de
[LOT-60](@ref lot-60) (le bruitage de victoire a besoin d'un écran de fin) et de
[LOT-53](@ref lot-53) (l'effet de mort se lit dans une boucle qui sait perdre).

## Navigation des tâches
- @subpage lot-59-tache-01-machine-etats-ecrans
- @subpage lot-59-tache-02-ecran-pause
- @subpage lot-59-tache-03-ecran-fin-niveau
- @subpage lot-59-tache-04-sequence-en-donnee
- @subpage lot-59-tache-05-progression-persistee
- @subpage lot-59-tache-06-selection-niveau
- @subpage lot-59-tache-07-documentation-verification
