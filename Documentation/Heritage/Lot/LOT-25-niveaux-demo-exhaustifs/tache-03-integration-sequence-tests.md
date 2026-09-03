# TACHE-03 — Intégration séquence et tests système {#lot-25-tache-03-integration-sequence-tests}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `HMI/main.cpp`, `Test/Systeme` · **Statut :** fait

## Contexte
Les niveaux de `TACHE-02` n'ont d'effet que si le jeu les charge **et** que le test système les
rejoue — c'est l'écart entre les deux (constaté avec `demo5.json`, absent du test système alors que
chargé en jeu) qui a déclenché ce lot ; cette tâche referme cet écart et empêche sa réapparition.

## Travail à réaliser
- **`Source/HMI/main.cpp`** : la liste de niveaux passée au constructeur de `GameScreen` (fabrique
  d'écrans, `ScreenId::Game`) reflète exactement la séquence finale décidée en `TACHE-01`.
- **`Source/Test/Systeme/test_parcours_complet.cpp`** : `sequence` (dans
  `ParcoursCompletSysteme.FranchitTouteLaSequence`) référence exactement les **mêmes** fichiers,
  dans le **même ordre**, chacun avec un scénario d'entrées déterministe qui exploite la mécanique
  visée (sur le modèle des scénarios déjà écrits pour `demo.json`…`demo4.json`) — **le niveau final
  combiné (dernier de la séquence) inclus**, avec un scénario qui enchaîne réellement les
  mécaniques qu'il mélange (pas seulement un scénario générique qui le franchirait par chance).
- **Garde-fou anti-divergence** (voir décision de cadrage de l'épic) : implémenté comme un script
  Python, `scripts/check_demo_sequence.py`, exécuté en CI (job `lint-exigences`, aux côtés du lint
  des exigences et du contrôle du cahier de test). Il extrait, par une simple recherche de motif
  `"demo-*.json"`, la séquence de fichiers dans `Source/HMI/main.cpp` et dans
  `Source/Test/Systeme/test_parcours_complet.cpp`, puis échoue (code de sortie 1) si les deux listes
  diffèrent (contenu ou ordre). Choisi plutôt qu'un fichier de configuration partagé : ne demande
  aucun refactoring des deux fichiers existants (qui restent lisibles en C++ pur, sans indirection
  vers un fichier externe), et le script est trivial à maintenir (une seule expression régulière).

## Fichiers impactés
- `Source/HMI/main.cpp`.
- `Source/Test/Systeme/test_parcours_complet.cpp`.
- Éventuel nouveau mécanisme de garde-fou (fichier de configuration partagé, ou test dédié).

## Tests (obligatoires)
- `ParcoursCompletSysteme.FranchitTouteLaSequence` franchit tous les niveaux de la séquence
  complète (`Won` partout), avec des scénarios d'entrées couvrant réellement la mécanique visée par
  chaque niveau (pas un simple « avancer à droite » qui passerait par chance).
- Le garde-fou anti-divergence échoue délibérément si un niveau est ajouté à l'une des deux listes
  sans l'autre (vérifié en le cassant intentionnellement pendant le développement, puis en le
  corrigeant).

## Points d'attention
- **Scénarios d'entrées réalistes** : un scénario qui franchit un niveau par accident (sans
  utiliser la mécanique visée) invaliderait le critère d'acceptation 1 de l'épic aussi sûrement
  qu'un niveau mal conçu — revoir chaque scénario en le faisant échouer volontairement (mécanique
  désactivée en pensée) pour confirmer qu'il échouerait vraiment sans elle.

## Définition de fait (DoD)
- Séquence de jeu et test système strictement synchronisés, garde-fou anti-divergence en place et
  vérifié ; tous les niveaux franchis par le test système.

## Exigences
Aucune exigence propre — cohérence entre le jeu et sa suite de tests.
