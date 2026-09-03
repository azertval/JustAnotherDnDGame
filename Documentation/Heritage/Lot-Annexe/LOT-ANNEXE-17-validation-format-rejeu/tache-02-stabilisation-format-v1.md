# TACHE-02 — Stabilisation du format v1 {#lot-annexe-17-tache-02-stabilisation-format-v1}

**Lot :** [LOT-ANNEXE-17](epic.md) · **Emplacement :** `Source/AiSolver/Replay` · **Statut :** fait

## Contexte
Le format de rejeu v1 (`LOT-ANNEXE-07`) a été défini en génération 1, avant qu'aucun algorithme
réel n'ait exporté le moindre fichier. Après usage effectif par les générations 2 (évolutionniste)
et 3 (policy gradient, acteur-critique), deux besoins concrets sont apparus, absents du format
initial : connaître la **durée totale** d'un rejeu sans recompter ses pas (utile à l'affichage en
jeu, `LOT-ANNEXE-18`, et aux rapports du CLI, `LOT-ANNEXE-19`), et savoir **quel algorithme** a
produit un rejeu donné (traçabilité, utile au garde-fou CI comme au diagnostic). Cette tâche ajoute
ces champs sans casser les fichiers déjà écrits.

## Travail à réaliser
- **Champ `totalDurationSeconds`** (`float`) : ajouté aux métadonnées du rejeu (`ReplayMetadata`,
  type déjà défini par `LOT-ANNEXE-07`) — égal à `stepCount × fixedDelta`, calculé une fois à
  l'export plutôt que recalculé à chaque lecture.
- **Champ `algorithmId`** (`std::string`) : identifiant court de l'algorithme d'origine (`"evo"`,
  `"pg"`, `"ac"`, `"avance"` — mêmes valeurs que l'argument `--algo` du CLI, `LOT-ANNEXE-19`), pour
  retrouver a posteriori de quel algorithme un rejeu est issu sans consulter les journaux
  d'entraînement.
- **Incrément de version** : le champ `version` déjà prévu par le format v1 passe de `1` à `2` sur
  tout nouvel export. Le lecteur (`LOT-ANNEXE-07`) accepte les deux valeurs : sur un fichier
  `version == 1`, `totalDurationSeconds` et `algorithmId` sont absents du JSON et pris à une valeur
  sentinelle (`0.0f` et `""` respectivement) plutôt que de faire échouer la lecture — ces fichiers
  restent des rejeux valides, seulement moins renseignés.
- Mise à jour de l'écrivain de rejeu (`LOT-ANNEXE-07`, module d'export) pour toujours produire
  `version == 2` avec les deux nouveaux champs renseignés à partir de ce lot.

## Fichiers impactés
- Le fichier portant `ReplayMetadata`/la sérialisation JSON du rejeu (défini par `LOT-ANNEXE-07`,
  sous `Source/AiSolver/Replay`) : ajout des deux champs, gestion des deux versions à la lecture.
- Tests : `Source/Test/Unit/AiSolver/Replay/test_replay_format.cpp` (mise à jour ou nouveau,
  couvrant les deux versions).

## Tests (obligatoires)
- **Lecture d'un fichier `version == 1`** (sans les nouveaux champs) : se charge sans erreur,
  `totalDurationSeconds == 0.0f` et `algorithmId == ""`.
- **Lecture d'un fichier `version == 2`** : `totalDurationSeconds` et `algorithmId` reflètent
  exactement les valeurs écrites à l'export.
- **Round-trip** : écrire puis relire un rejeu produit un `algorithmId` et un `totalDurationSeconds`
  identiques à ceux fournis à l'export.
- **Non-régression** : les tests existants de `LOT-ANNEXE-07` sur le format v1 (séquence
  d'entrées, empreinte de niveau) restent verts sans modification.

## Points d'attention
- **Aucun fichier `version == 1` déjà écrit ne doit devenir illisible** : c'est le critère
  d'acceptation central de ce lot — toute évolution du format qui romprait la lecture d'un fichier
  existant est à exclure, quitte à repousser un champ à une version ultérieure.
- **`totalDurationSeconds` est une commodité dérivée, pas une source de vérité** : en cas de
  divergence avec `stepCount × fixedDelta` recalculé (ne devrait jamais arriver si l'export est
  correct), c'est le nombre de pas réel de la séquence qui fait foi pour la simulation — ce champ
  ne sert qu'à l'affichage/au reporting.
- **`algorithmId` est une chaîne libre à ce stade, pas une énumération figée** : de nouveaux
  algorithmes pourraient apparaître après ce lot ; contraindre la valeur à un ensemble fermé
  obligerait à retoucher le format à chaque nouvel algorithme, ce que ce lot cherche justement à
  éviter.

## Définition de fait (DoD)
- Format v1 stabilisé (`version` 1 et 2 tous deux lisibles), nouveaux champs disponibles et testés
  (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour. Aucune exigence propre à
  cette tâche (raffinement du format couvert par les exigences de `LOT-ANNEXE-07`).

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
Aucune exigence propre à cette tâche (raffinement du format de rejeu v1, couvert par les exigences
déclarées en `LOT-ANNEXE-07`).
