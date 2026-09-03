# LOT-ANNEXE-03 — Bibliothèque de réseaux de neurones maison {#lot-annexe-03}

> Statut : **fait** — TACHE-01 à TACHE-06 faites. Prérequis : [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor<float>`,
> `Rng`) et [LOT-ANNEXE-02](@ref lot-annexe-02) (autodiff). Compose ces briques en couches et
> réseaux réutilisables par **tous** les algorithmes d'apprentissage ultérieurs — évolutionniste
> (génération 2) comme par gradient (génération 3), ces deux familles ayant seulement besoin d'un
> réseau capable de calculer une sortie à partir de paramètres, la façon dont ces paramètres sont
> ensuite ajustés leur étant propre.

## Objectif
`LOT-ANNEXE-01` fournit le calcul brut (`Tensor`), `LOT-ANNEXE-02` la différentiation (`Node`,
`backward()`), mais aucun des deux ne définit ce qu'est une **couche** ni un **réseau** : composer
un `matmul` et un `add` à la main pour chaque couche, à chaque fois, serait répétitif et fragile
(risque d'oublier le biais, d'inverser l'ordre des opérandes du produit matriciel). Ce lot introduit
la couche dense comme brique réutilisable, sa composition en réseau, l'initialisation de ses poids
(dont dépend fortement la qualité de l'apprentissage, quel que soit l'algorithme utilisé ensuite) et
la persistance des poids entraînés — sans laquelle un modèle entraîné en génération 2/3 ne pourrait
jamais être rejoué en jeu (rappel : le jeu ne fait **aucune** inférence live, seulement une séquence
d'actions pré-enregistrée obtenue **hors ligne** à partir d'un modèle chargé depuis un fichier).

## Périmètre

### Inclus
- **Couche dense** (`aisolver::nn::Dense`) : poids + biais comme paramètres différentiables
  (`autodiff::NodePtr`, `LOT-ANNEXE-02`), passe avant `forward()` composant `matmul` + `add`.
- **Activations différentiables** : `sigmoid` et `softmax`, ajoutées à l'ensemble d'opérations posé
  par `LOT-ANNEXE-02` via sa fabrique générique (`unaryOp`/`binaryOp`) — `relu`/`tanhOp`, déjà
  livrées par `LOT-ANNEXE-02`, sont réutilisées telles quelles.
- **Opérations différentiables complémentaires** (`subtract`, `divide`, `addScalar`,
  `multiplyScalar`, `logOp`, `expOp`, `selectIndex`, `minimum`, `clamp`) : complètent l'ensemble
  livré par `LOT-ANNEXE-02`, qui les avait explicitement reportées ici faute de consommateur. Elles
  sont ajoutées dans `Math/Autodiff/Ops.h`, via la même fabrique `unaryOp`/`binaryOp`, sans toucher
  au moteur. Sans elles, aucune perte de *policy gradient* n'est écrivable (`LOT-ANNEXE-12`).
- **Composition en réseau** (`aisolver::nn::Network`) : séquence de couches (+ activation associée
  à chacune), `forward()` bout-en-bout, accès aux paramètres de toutes les couches (pour
  l'optimiseur, `LOT-ANNEXE-04`).
- **Initialisation des poids** : schémas Xavier et He, via `aisolver::Rng` (`LOT-ANNEXE-01`).
- **Sérialisation des poids** : format de fichier **versionné**, lisible et inscriptible, pour
  sauvegarder un réseau entraîné et le recharger — c'est le mécanisme par lequel un modèle entraîné
  hors ligne (génération 2/3) devient rejouable en jeu (génération 5, hors périmètre de ce lot, qui
  ne fait que poser le format).

### Exclus (hors périmètre de ce lot)
- **Autres types de couches** (convolution, récurrentes, attention) : aucun besoin identifié pour un
  agent de plateforme 2D à observations de dimension modeste (génération 1) — la couche dense
  suffit à l'objectif du programme.
- **Régularisation** (dropout, normalisation par lot, pénalité de poids) : aucun signe de sur-
  apprentissage à corriger avant qu'un premier entraînement réel (génération 2) n'ait eu lieu ;
  prématuré de s'en prémunir ici.
- **Entraînement lui-même** (boucle d'optimisation, fonction de perte liée au jeu) : ce lot livre la
  **structure** du réseau (forward, paramètres), pas l'algorithme qui les ajuste — c'est
  `LOT-ANNEXE-04` (optimiseurs génériques) puis génération 2/3 (boucle d'entraînement spécifique).
- **Compatibilité du format de sérialisation avec un format tiers** (ONNX, PyTorch `state_dict`) :
  contrainte dure du programme (aucune dépendance à un framework ML), le format est **propre au
  projet**, versionné pour ses propres évolutions futures, sans objectif d'interopérabilité externe.

## Décisions de cadrage
- **`sigmoid`/`softmax` construites via la fabrique `unaryOp`/`binaryOp` de `LOT-ANNEXE-02`, sans
  modifier `Node.h` ni `backward()`.** Démonstration directe de la décision de cadrage prise dans
  `LOT-ANNEXE-02` : un lot consommateur ajoute des opérations différentiables sans rouvrir le moteur.
  Chacune passe le **même** contrôle bloquant (`GradientCheck.h`, `LOT-ANNEXE-02` TACHE-04) avant
  d'être utilisée par `Dense`/`Network`.
- **Les paramètres d'une couche (`_weights`, `_bias`) sont des `autodiff::NodePtr` créés via
  `variable()`, jamais recréés à chaque `forward()`.** Une couche possède ses paramètres pour toute
  sa durée de vie ; seule la sortie de `forward()` (et les nœuds intermédiaires du calcul) change à
  chaque appel — cohérent avec un entraînement qui ajuste les **mêmes** paramètres sur plusieurs
  passes (l'optimiseur, `LOT-ANNEXE-04`, doit pouvoir retrouver le même `Node` d'une itération à
  l'autre pour y accumuler puis appliquer un gradient).
- **Format de sérialisation binaire, versionné dès la première version** (`en-tête magique +
  numéro de version + par couche : forme + valeurs brutes`). Verser un numéro de version dès le
  départ, même pour une version `1`, évite d'avoir à deviner plus tard le format d'un fichier ancien
  sans indice — coût nul aujourd'hui, protection immédiate dès que le format devra évoluer (ex. un
  type de couche supplémentaire).
- **Xavier pour les couches à activation `tanh`/`sigmoid`, He pour les couches à activation
  `relu`** (recommandation standard reprise telle quelle, pas réinventée) : le choix du schéma est
  un paramètre explicite à la construction de chaque `Dense`, jamais déduit automatiquement de
  l'activation qui suivra (une couche ne connaît pas, à sa construction, quelle activation
  `Network` lui appliquera) — c'est à l'appelant de faire le choix cohérent.
- **`Network` ne possède aucune fonction de perte ni logique d'entraînement.** Une perte est propre
  au problème résolu (génération 2/3, spécifique au jeu) ; `Network` se limite à `forward()` et à
  l'accès aux paramètres — un réseau reste utilisable en dehors de tout contexte de jeu, cohérent
  avec le caractère « bibliothèque de calcul pure » de la génération 0 entière.

## Notions abordées
Voir @ref guide-annexe-reseaux-neurones (neurone, couche dense, non-linéarité, activations,
initialisation des poids). Sources directes : Glorot & Bengio (2010, initialisation Xavier) ; He,
Zhang, Ren, Sun (2015, initialisation He) ; Nair & Hinton (2010, popularisation de ReLU) ;
Goodfellow, Bengio, Courville (2016, référence générale) — bibliographie complète dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-003`](@ref EX-IA-003).
- Réutilisées : `EX-IA-001` (`Tensor<float>`, `Rng`), `EX-IA-002` (moteur d'autodiff, fabrique
  générique d'opérations différentiables), `EX-NFR-010`/`EX-NFR-012`/`EX-NFR-013`/`EX-NFR-020`
  (testabilité headless, conventions, `/W4 /WX`, couverture de tests), `EX-ARCH-001` (sens de
  dépendance `AiSolver → Core`, jamais l'inverse).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-couche-dense.md) | Couche dense (poids + biais, forward/backward) | `Source/AiSolver/Nn` | ✅ |
| [TACHE-02](tache-02-activations.md) | Fonctions d'activation différentiables (`sigmoid`, `softmax`) | `Source/AiSolver/Nn` | ✅ |
| [TACHE-03](tache-03-composition-reseau.md) | Composition en réseau (`Network`) | `Source/AiSolver/Nn` | ✅ |
| [TACHE-04](tache-04-initialisation-serialisation.md) | Sérialisation des poids | `Source/AiSolver/Nn` | ✅ |
| [TACHE-05](tache-05-operations-differentiables-complementaires.md) | Opérations différentiables complémentaires (`log`, `exp`, `divide`, `selectIndex`, `minimum`, `clamp`…) | `Source/AiSolver/Math/Autodiff` | ✅ |
| [TACHE-06](tache-06-tests.md) | Tests : reproductibilité, stabilité numérique | `Source/Test/Unit/AiSolver/Nn` | ✅ |

## Critères d'acceptation du lot
1. Un `Network` composé de plusieurs `Dense` avec activations mêlées (`relu`, `tanh`, `sigmoid`,
   `softmax`) calcule une sortie cohérente en `forward()`, et `autodiff::backward()` appliqué à une
   perte scalaire construite sur cette sortie produit des gradients non nuls sur **tous** les
   paramètres du réseau.
2. `sigmoid` et `softmax` passent le contrôle de gradient de `LOT-ANNEXE-02` (`GradientCheck.h`)
   avant d'être utilisées dans un `Dense`/`Network`. Les neuf opérations complémentaires de
   TACHE-05 passent le **même** contrôle, et la chaîne
   `multiplyScalar(logOp(selectIndex(softmax(sortie), a)), -G)` — squelette exact de la perte de
   `LOT-ANNEXE-12` — produit des gradients non nuls sur tous les poids traversés.
3. Deux réseaux construits avec les **mêmes** poids (chargés depuis le même fichier sérialisé) et la
   **même** entrée produisent une sortie **identique** (reproductibilité, `EX-NFR-002` par analogie
   — déterminisme).
4. Un réseau sauvegardé (`LOT-ANNEXE-03` TACHE-04) puis rechargé produit une sortie identique à
   l'original, pour la même entrée.
5. Aucune sortie `NaN`/`inf` sur des entrées de forme et d'amplitude plausibles, y compris aux
   extrêmes (test dédié, TACHE-06).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `scripts/lint_exigences.py` verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor<float>`, `Rng`) et
[LOT-ANNEXE-02](@ref lot-annexe-02) (`autodiff::Node`, `backward()`, fabrique générique
d'opérations). [LOT-ANNEXE-04](@ref lot-annexe-04) (optimiseurs) en dépend directement (un
optimiseur ajuste les paramètres exposés par `Network::parameters()`).

## Navigation des tâches
- @subpage lot-annexe-03-tache-01-couche-dense
- @subpage lot-annexe-03-tache-02-activations
- @subpage lot-annexe-03-tache-03-composition-reseau
- @subpage lot-annexe-03-tache-04-initialisation-serialisation
- @subpage lot-annexe-03-tache-05-operations-differentiables-complementaires
- @subpage lot-annexe-03-tache-06-tests
