# TACHE-04 — Sérialisation des poids {#lot-annexe-03-tache-04-initialisation-serialisation}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/AiSolver/Nn` · **Statut :** fait

## Contexte
`Network` (TACHE-03) n'a encore aucun moyen de survivre à la fin du processus qui l'a entraîné :
cette tâche livre la sérialisation d'un réseau entraîné sur disque — le mécanisme par lequel un
modèle appris hors ligne devient rejouable en jeu.

**Écart avec la planification initiale** : cette tâche devait aussi livrer le corps de
`initializeWeights` (`WeightInitScheme::Xavier`/`He`), TACHE-01 n'en posant que le contrat. En
pratique, le DoD de TACHE-01 (« deux couches ont des poids différents ») exigeait une initialisation
réellement aléatoire pour être testable — `WeightInit.h`/`.cpp` ont donc été livrés complets dès
TACHE-01 (cf. ses points d'attention). Cette tâche se limite à la sérialisation ; ses propres tests
statistiques sur l'initialisation (bornes Xavier, plausibilité He, reproductibilité par graine)
restent ici, dans `test_weight_init.cpp` — seule l'implémentation a changé de tâche, pas sa
couverture de test.

## Travail à réaliser
- **`Source/AiSolver/Nn/Serialization.h`/`.cpp`** : format binaire versionné.
  - Constantes `constexpr std::uint32_t WEIGHTS_FILE_MAGIC = 0x41494E4Eu;` (ASCII `"AINN"`) et
    `constexpr std::uint32_t WEIGHTS_FILE_VERSION = 1;`.
  - `bool saveWeights(const Network& network, const std::filesystem::path& path);` — écrit
    séquentiellement : magique (4 octets), version (4 octets), nombre de couches
    (`network.layerCount()`), puis pour chaque couche : forme des poids (rang + dimensions), valeurs
    brutes des poids, forme du biais, valeurs brutes du biais. Renvoie `false` si l'ouverture du
    fichier échoue (erreur récupérable, pas d'exception — cf. politique d'erreurs des conventions).
  - `bool loadWeights(Network& network, const std::filesystem::path& path);` — lit et **valide**
    magique et version avant toute lecture de données (renvoie `false` sur fichier invalide ou
    version inconnue, sans planter) ; vérifie que le nombre de couches et la forme de chaque couche
    lue correspondent **exactement** à celles du `Network` déjà construit (erreur récupérable si
    incompatible — l'appelant doit avoir construit un `Network` de structure identique avant de
    charger ses poids, ce lot ne reconstruit pas la structure depuis le fichier).

## Fichiers impactés
- `Source/AiSolver/Nn/Serialization.h` (nouveau).
- `Source/AiSolver/Nn/Serialization.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Nn/Serialization.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Nn/test_weight_init.cpp`,
  `Unit/AiSolver/Nn/test_serialization.cpp`).
- `Source/Test/Unit/AiSolver/Nn/test_weight_init.cpp` (nouveau).
- `Source/Test/Unit/AiSolver/Nn/test_serialization.cpp` (nouveau).

## Tests (obligatoires)
- **Xavier reste dans les bornes** : sur un grand nombre d'éléments initialisés, tous restent dans
  `[-bound, bound]` calculé pour la forme donnée.
- **He plausible** : moyenne empirique proche de `0`, écart-type empirique proche de
  `sqrt(2/fanIn)` (tolérance large, même esprit que le test statistique faible de `Rng`,
  `LOT-ANNEXE-01` TACHE-01).
- **Reproductibilité de l'initialisation** : deux `Dense` construits avec des `Rng` de même graine
  produisent des poids **identiques**.
- **Sauvegarde puis rechargement produit la même sortie** : un `Network` entraîné (ou simplement
  initialisé) sauvegardé via `saveWeights` puis rechargé dans un `Network` de structure identique
  via `loadWeights` produit, pour une même entrée, une sortie **identique** à l'original (critère
  d'acceptation n°4 de l'épic).
- **Rejet d'un fichier invalide** : `loadWeights` sur un fichier au magique incorrect, à la version
  inconnue, ou dont la forme d'une couche ne correspond pas au `Network` cible renvoie `false`, sans
  planter ni corrompre l'état du `Network` (les poids restent ceux d'avant l'appel en cas d'échec —
  pas de modification partielle).
- **Fichier absent** : `loadWeights` sur un chemin inexistant renvoie `false` proprement.

## Points d'attention
- **`loadWeights` ne modifie le `Network` cible qu'en cas de succès complet** : lire d'abord
  l'intégralité du fichier en mémoire (ou valider la cohérence de toutes les formes avant d'écrire
  quoi que ce soit) évite un `Network` dans un état à moitié chargé si une incohérence est détectée
  à mi-fichier.
- **Le format ne stocke aucune information de structure au-delà des formes** (pas de type
  d'activation, pas de schéma d'initialisation) : reconstruire un `Network` compatible (mêmes
  couches, mêmes activations, dans le même ordre) reste la responsabilité du code appelant avant
  d'appeler `loadWeights` — cohérent avec la décision de ne pas viser l'interopérabilité ni
  l'auto-description complète (cf. exclusions de l'épic).
- **`WEIGHTS_FILE_VERSION` doit être incrémenté** dès que le format binaire change (nouveau champ,
  ordre différent) — jamais réutilisé pour un format incompatible avec la version précédente, sous
  peine de charger silencieusement des données corrompues dans un futur lot.

## Définition de fait (DoD)
- `initializeWeights`, `saveWeights`, `loadWeights` disponibles et testés (`ctest` vert), y compris
  les cas d'erreur (fichier invalide, absent, incompatible) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
@ref guide-annexe-reseaux-neurones — neurone, couche dense, fonctions d'activation, initialisation
des poids.

## Exigences
Contribue à `EX-IA-003` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
