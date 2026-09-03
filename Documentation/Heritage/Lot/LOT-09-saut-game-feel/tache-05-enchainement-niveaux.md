# TACHE-05 — Enchaînement de niveaux (séquence, auto-avance, retour titre) {#lot-09-tache-05-enchainement-niveaux}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
Aujourd'hui, « Charger niveau » ouvre **un** niveau et la réussite revient au menu. On veut une
**progression** (`EX-LVL-010`, `EX-LVL-011`) : écran titre → niveau 1 → niveau 2 → … → dernier →
retour au titre. La simulation `Core` ne connaît qu'**un niveau à la fois** ; l'enchaînement est
une responsabilité **d'orchestration**, donc côté `HMI` (`GameScreen`).

## Travail à réaliser
- **Séquence ordonnée** : le `GameScreen` reçoit une **liste ordonnée** de chemins de niveaux
  (plutôt qu'un chemin unique) et un **indice courant** (démarrage à 0). Adapter la **fabrique
  d'écrans** (menu « Charger niveau ») pour fournir cette liste.
  - Source de la liste à trancher : **liste explicite** (`demo1.json`, `demo2.json`, …) ou
    **balayage ordonné** du dossier `Elements/Levels` (tri stable par nom). Préférence : extensible
    (« niveau XX ») sans recompiler → un tri de dossier est séduisant, mais une liste explicite
    donne un ordre de difficulté maîtrisé. À arbitrer à l'implémentation, documenter le choix.
- **Chargement d'un niveau de la séquence** : factoriser une routine `loadLevel(index)` qui
  (re)construit la **scène** — monde ECS neuf (tuiles) + personnage à l'entrée — depuis le fichier
  d'indice `index`. Réutilise le chargement/spawn du LOT-08 ; attention à repartir d'un **monde
  neuf** (pas d'état résiduel entre niveaux).
- **Transition à la réussite** (`EX-LVL-011`) : quand `evaluateOutcome` renvoie `Won`,
  - s'il existe un **niveau suivant** (`index + 1 < taille`) : `loadLevel(index + 1)` (rester sur
    l'écran de jeu, enchaîner) ;
  - sinon (dernier niveau) : **retour au menu/titre** (`ScreenTransition::switchTo(Menu)`).
- **Échec** : inchangé (redémarrage du **niveau courant** à l'entrée). **Échap** : retour au menu.

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (liste + indice, `loadLevel`, transition de réussite).
- La **fabrique** d'écrans (là où `GameScreen` est construit avec un chemin — `MenuScreen`/
  `ScreenManager`/`main`) : fournir la séquence.

## Vérification / tests
- **Logique testable** : isoler autant que possible la **décision de progression** (indice courant,
  « y a-t-il un suivant ? », prochain indice) dans une petite logique **pure** testable sans GPU
  (ex. un `LevelSequence` : `current()`, `advance()`, `isFinished()`), couverte par tests unitaires.
- L'**intégration** GPU (rechargement de scène, rendu) est vérifiée **visuellement** ; le
  franchissement effectif de la séquence est prouvé par le test d'intégration de la TACHE-06.

## Points d'attention
- **Monde neuf entre niveaux** : si `core::World` n'est pas ré-assignable simplement, prévoir une
  remise à zéro explicite ; ne pas laisser d'entités du niveau précédent.
- **Frontière** : aucune logique de simulation nouvelle dans `Core` ; la progression est de
  l'orchestration `HMI`. Toute logique testable (séquence) doit rester **pure**.
- **Extensibilité** : ajouter un niveau ne doit pas demander de modifier la logique d'enchaînement.
- **Robustesse** : une séquence **vide** ou un niveau **illisible** ne doit pas planter (état neutre
  récupérable, `EX-NFR-040`).

## Définition de fait (DoD)
- Réussite → niveau suivant chargé ; après le dernier → retour titre ; échec → redémarrage du niveau
  courant. Logique de séquence **testée** (`ctest` vert) ; enchaînement **vérifié visuellement** ;
  build `/W4 /WX`.

## Exigences
`EX-LVL-010`, `EX-LVL-011`, `EX-NFR-010`, `EX-NFR-040`, `EX-ARCH-030`.
