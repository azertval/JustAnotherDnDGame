# TACHE-01 — Réseau critique (estimation de la valeur d'état) {#lot-annexe-13-tache-01-reseau-critique}

**Lot :** [LOT-ANNEXE-13](epic.md) · **Emplacement :** `Source/AiSolver/Training/ActorCritic` ·
**Statut :** fait

## Contexte
LOT-ANNEXE-12 n'entraîne qu'un réseau (la politique). Ce lot introduit un second réseau, le
**critique**, dont le rôle est différent : il ne choisit aucune action, il **estime** — pour une
observation donnée — la récompense future totale attendue si la politique courante continue à
jouer depuis cet état. C'est cette estimation qui, en TACHE-02, permettra de calculer l'avantage.
La bibliothèque de réseaux de LOT-ANNEXE-03 est directement réutilisable : le critique n'est qu'une
architecture différente (une sortie scalaire au lieu d'une distribution sur l'espace d'action), pas
un nouveau mécanisme.

## Travail à réaliser
- **`aisolver::training::CriticNetwork`** (`Source/AiSolver/Training/ActorCritic/CriticNetwork.h/.cpp`) :
  réseau construit avec les couches de LOT-ANNEXE-03 (mêmes types de couches denses/activations que
  le réseau de politique), même encodage d'observation en entrée (LOT-ANNEXE-06) que la politique,
  **une seule sortie scalaire** (pas de couche de normalisation en distribution comme pour la
  politique — la sortie est une valeur réelle non bornée).
- Constructeur paramétré par la taille de l'observation encodée et la taille de la ou des couches
  cachées (configuration séparée de celle de la politique — aucune contrainte à ce qu'elles soient
  identiques).
- Méthode `autodiff::NodePtr forward(const Observation& observation)` — un unique passage avant, nœud
  de graphe d'autodiff en sortie, réutilisable directement par TACHE-03 pour la rétropropagation de
  la perte du critique.
- Initialisation des poids : réutilise le même schéma d'initialisation (et le même générateur
  pseudo-aléatoire, LOT-ANNEXE-01) que celui déjà retenu pour le réseau de politique en
  LOT-ANNEXE-03, sans en introduire un nouveau spécifiquement pour le critique.

## Fichiers impactés
- `Source/AiSolver/Training/ActorCritic/CriticNetwork.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/CriticNetwork.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **Forme de sortie** : `forward` renvoie systématiquement un unique scalaire, quelle que soit
  l'observation d'entrée (dimension fixe et validée par assertion en développement).
- **Déterminisme à poids fixés** : deux appels à `forward` avec la même observation et les mêmes
  poids renvoient exactement la même valeur.
- **Sensibilité aux poids** : une perturbation des poids du critique change la valeur de sortie pour
  une même observation (vérifie que le réseau n'est pas dégénéré/constant après construction).
- **Indépendance du réseau de politique** : construire et faire varier un `CriticNetwork` n'affecte
  en rien les sorties du réseau de politique (`nn::Network`, LOT-ANNEXE-03) construit séparément
  sur la même observation (pas de poids ou d'état partagé accidentel).

## Points d'attention
- **Aucun partage de poids avec le réseau de politique** (décision de cadrage de l'épic) : même si
  les deux réseaux consomment la même observation encodée en entrée, ce sont deux instances
  entièrement indépendantes de la bibliothèque de LOT-ANNEXE-03, avec leurs propres paramètres.
- La sortie du critique est une **valeur non bornée** (pas de fonction d'activation finale de type
  sigmoïde/softmax) : contrairement à la politique, rien ne contraint son échelle a priori — c'est
  la perte d'entraînement (TACHE-03) qui la fait converger vers l'échelle des retours observés.
- Garder l'architecture du critique **volontairement simple** (comparable en taille à la politique) :
  la comparaison chiffrée de TACHE-04 doit isoler l'effet de la réduction de variance, pas celui
  d'un critique disproportionnellement plus puissant.

## Définition de fait (DoD)
- `CriticNetwork` disponible et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen
  à jour.

## Notions abordées
@ref guide-annexe-acteur-critique — variance du gradient, fonction de valeur, critique, avantage.

## Exigences
`EX-IA-014` (nouvelle, couverte par le lot).
