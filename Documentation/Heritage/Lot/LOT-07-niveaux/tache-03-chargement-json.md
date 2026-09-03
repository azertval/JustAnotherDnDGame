# TACHE-03 — Chargement du niveau (JSON) {#lot-07-tache-03-chargement-json}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** à faire

## Contexte
Un niveau est décrit par un **fichier externe** (`EX-LVL-001`) au format **JSON structuré** : un
objet JSON (`name`, `width`, `height`) avec une **liste `tiles`** d'objets `{x, y, type, …}`
(`EX-LVL-003`). Cette tâche lit ce fichier et construit un `Level` (TACHE-02). La **validation**
fait l'objet de la TACHE-04.

## Travail à réaliser
- **`LevelLoader`** : `loadFromString(json)` et `loadFromFile(path)` → un **résultat**
  (`Level` en cas de succès, description d'erreur sinon) — pas d'exception qui remonte
  (`EX-NFR-040`, politique d'erreurs des conventions).
- **Parsing JSON** via nlohmann/json (TACHE-01), inclus **uniquement dans le `.cpp`**.
- **Tuiles → `TileMap`** : pour chaque objet de `tiles`, convertir son `type` (chaîne :
  `entry`, `exit`, `solid`, `danger`, `switch`, `door`) en `TileType` et le placer à sa position
  `(x, y)` dans une `TileMap` dense (cases absentes = `Vide`) ; un `type` inconnu est une erreur.
- **Métadonnées** : `name`, dimensions ; **entrée/sortie** = tuiles de type `entry`/`exit` ;
  **mécanismes** = liaisons résolues à partir des **identifiants** (`switch.id` ↔
  `door.opensWith`) vers les positions correspondantes.
- Erreurs récupérables couvertes : fichier absent, JSON malformé, champ manquant, `type` de tuile
  inconnu (la validation « métier » — bornes, entrée/sortie, liaisons — est en TACHE-04).

## Fichiers impactés
- `Source/Core/Levels/LevelLoader.h`/`.cpp` (nouveau) ; éventuel type de résultat d'erreur
  partagé (`Core/Diagnostics` ou `Core/Levels`).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Un JSON **valide** produit un `Level` correct : dimensions, quelques tuiles typées à des
  positions connues, entrée/sortie, mécanismes.
- Un JSON **malformé** (syntaxe) renvoie une **erreur récupérable** (pas de plantage).
- Un **`type` de tuile inconnu** renvoie une erreur exploitable.
- Un **champ manquant** (ex. `grid`) renvoie une erreur.

## Points d'attention
- Le chargeur travaille sur une **chaîne** (testable sans fichier) ; `loadFromFile` n'ajoute que
  la lecture disque. Séparer les deux facilite les tests (`EX-NFR-010`).
- **UTF-8** : lecture binaire du fichier, cohérente avec le reste du projet.
- Ne pas exposer `nlohmann/json` dans l'en-tête public de `LevelLoader`.

## Définition de fait (DoD)
- Chargement d'un niveau valide et gestion des erreurs de format, testés (`ctest` vert) ; build
  `/W4 /WX`, documenté.

## Exigences
`EX-LVL-001`, `EX-LVL-003`, `EX-NFR-040`, `EX-NFR-010`.
