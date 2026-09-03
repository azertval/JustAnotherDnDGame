# LOT-07 — Niveaux : modèle et chargement {#lot-07}

> Statut : **terminé**. « Charger niveau » ouvre désormais un **niveau chargé depuis un fichier** (format JSON, `Source/Elements/Levels/demo.json`) et en affiche la **grille de tuiles**, au lieu de la scène codée en dur. Le déplacement du personnage et le comportement des mécanismes restent des lots ultérieurs.

## Objectif
Représenter un niveau comme une **grille de tuiles typées** et le **charger depuis un fichier** au
format **JSON structuré** (liste de tuiles-objets, `EX-LVL-003`), avec **validation**
(`EX-LVL-004`). À l'issue du
lot, « Charger niveau » ouvre un **niveau de démonstration réel** chargé depuis
`Source/Elements/Levels`, dont la grille s'affiche à l'écran.

Le **déplacement du personnage** et le **comportement des mécanismes** restent des lots
ultérieurs : ce lot pose la **fondation de données** dont dépend tout le gameplay.

## Périmètre

### Inclus
- **Modèle (Core)** : types de tuiles (`Vide`, `Solide`, `Danger`, `Entrée`, `Sortie`,
  `Interrupteur`, `Porte`), grille (`TileMap`) et niveau (`Level` = grille + entrée/sortie +
  mécanismes avec liaisons) — `EX-GP-001`, `EX-LVL-002`.
- **Format & chargement** : lecture d'un fichier de niveau au format **JSON structuré** — un
  objet JSON portant les métadonnées et une **liste de tuiles-objets** `{x, y, type, …}` avec
  liaisons de mécanismes par identifiant (`EX-LVL-001`, `EX-LVL-003`), parsé via une
  **bibliothèque JSON épinglée**.
- **Validation** : dimensions cohérentes avec la grille, présence d'une entrée et d'une sortie,
  liaisons de mécanismes valides ; erreur **récupérable** et exploitable si le fichier est
  invalide (`EX-LVL-004`, `EX-NFR-040`).
- **Contenu** : au moins **un niveau de démonstration** valide dans `Source/Elements/Levels`,
  copié à côté de l'exécutable.
- **Rendu** : affichage de la **grille de tuiles** du niveau chargé (via l'atlas) ; « Charger
  niveau » ouvre ce niveau au lieu de la scène codée en dur (`EX-REN-010`).

### Exclus (lots ultérieurs)
- **Déplacement / saut / collisions** du personnage (`EX-GP-010`…`EX-GP-014`).
- **Comportement des mécanismes** (interrupteur ↔ porte, `EX-GP-020`/`EX-GP-021`) : ce lot les
  **charge et représente**, sans interaction ni animation.
- **Progression** multi-niveaux et enchaînement (`EX-LVL-010`/`EX-LVL-011`) et les **3 niveaux**
  du MVP (`EX-LVL-012`) : un seul niveau de démonstration ici.
- **Éditeur** (`EX-EDIT-*`) et **décors** (`EX-DEC-*`).

## Décisions de cadrage
- **Format** : **JSON structuré orienté objets** conforme à `EX-LVL-003` — le fichier est un
  objet JSON (`name`, `width`, `height`, `tiles`), où `tiles` est une **liste d'objets tuile**
  `{x, y, type, …}` (cases vides omises). Types : `entry`, `exit`, `solid`, `danger`, `switch`,
  `door` ; liaison interrupteur↔porte par **identifiant** (`switch.id` ↔ `door.opensWith`).
  Choisi pour un moteur **extensible** (données riches par tuile, round-trip d'éditeur direct).
- **Dépendance JSON** : bibliothèque **nlohmann/json**, ajoutée via **FetchContent** et
  **épinglée** à une version (`EX-NFR-031`) ; *header-only*, elle n'entame pas la testabilité de
  `Core`.
- **Emplacement** : modèle, chargement et validation dans **`Source/Core/Levels`** (le
  chargement relève de `Core`) ; fichiers de niveaux dans **`Source/Elements/Levels`**.
- **Erreurs** : un fichier invalide **n'interrompt pas** le programme — le chargement renvoie un
  **résultat** décrivant l'échec (politique d'erreurs des conventions, `EX-NFR-040`).
- **Rendu** : la grille est peuplée en **entités ECS** (une tuile = un sprite) dessinées par le
  `SpriteRenderer` existant (réutilisation du LOT-05) ; le `TileMap` reste la **source de
  vérité** pour le futur gameplay (collisions).

## Exigences couvertes
- `EX-LVL-001` (fichier externe), `EX-LVL-002` (contenu du niveau), `EX-LVL-003` (format
  JSON structuré), `EX-LVL-004` (validation).
- `EX-GP-001` (grille de tuiles typées), `EX-REN-010` (rendu de la grille depuis l'atlas).
- `EX-NFR-020` (tests), `EX-NFR-031` (dépendance épinglée), `EX-NFR-040` (erreur récupérable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-dependance-json.md) | Dépendance JSON (nlohmann/json épinglé) | `External`, CMake | ✅ Fait |
| [TACHE-02](tache-02-modele-niveau.md) | Modèle de tuiles et de niveau | `Core/Levels` | ✅ Fait |
| [TACHE-03](tache-03-chargement-json.md) | Chargement du niveau (JSON) | `Core/Levels` | ✅ Fait |
| [TACHE-04](tache-04-validation.md) | Validation du niveau | `Core/Levels` | ✅ Fait |
| [TACHE-05](tache-05-niveau-demo.md) | Niveau de démonstration | `Elements/Levels` | ✅ Fait |
| [TACHE-06](tache-06-rendu-integration.md) | Rendu du niveau + intégration « Charger niveau » | `HMI/Interface` | ✅ Fait |

## Critères d'acceptation du lot
1. Un fichier de niveau au format **JSON structuré** (liste de tuiles-objets) est chargé en un `Level` (grille typée
   + entrée/sortie + mécanismes avec liaisons).
2. Un fichier **invalide** (dimensions incohérentes, entrée/sortie manquante, liaison de
   mécanisme invalide) est **rejeté proprement** avec un message exploitable, **sans planter**.
3. **« Charger niveau »** ouvre le **niveau de démonstration** et sa grille s'affiche à l'écran ;
   **Échap** revient au menu.
4. La logique (modèle, chargement, validation) est **couverte par des tests** (`ctest` vert) ;
   build `/W4 /WX` sans avertissement, documentation Doxygen et `CHANGELOG.md` à jour.

## Dépendances
- Réutilise l'**ECS** (`core::World`, LOT-03) et le **rendu 2D** (`SpriteRenderer`,
  `TextureAtlas`, `Camera2D`, LOT-05).
- S'intègre au `GameScreen` (LOT-06) : « Charger niveau » ouvre le niveau chargé.
- Introduit la **première dépendance tierce de production** (`nlohmann/json`).

## Navigation des tâches
- @subpage lot-07-tache-01-dependance-json
- @subpage lot-07-tache-02-modele-niveau
- @subpage lot-07-tache-03-chargement-json
- @subpage lot-07-tache-04-validation
- @subpage lot-07-tache-05-niveau-demo
- @subpage lot-07-tache-06-rendu-integration
