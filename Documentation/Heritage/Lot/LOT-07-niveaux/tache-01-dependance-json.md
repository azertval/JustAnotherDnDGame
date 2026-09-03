# TACHE-01 — Dépendance JSON (nlohmann/json épinglé) {#lot-07-tache-01-dependance-json}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `External/`, CMake · **Statut :** à faire

## Contexte
Le format de niveau est un **JSON structuré** (liste de tuiles-objets, `EX-LVL-003`).
Il faut donc parser du JSON. Plutôt qu'un parser maison, le lot s'appuie sur **nlohmann/json**
(*header-only*, éprouvée), première dépendance tierce de **production** du projet.

## Travail à réaliser
- Ajouter **nlohmann/json** via **FetchContent**, **épinglée** à une version précise
  (tag/commit) — jamais une branche mobile (`EX-NFR-031`).
- Exposer la cible d'interface (`nlohmann_json::nlohmann_json`) et la lier au module qui en a
  besoin (**`Core`**, cible du chargement de niveaux).
- Vérifier que l'inclusion et un parse minimal (`nlohmann::json::parse("{}")`) compilent.

## Fichiers impactés
- `CMakeLists.txt` racine (ou `External/CMakeLists.txt`) : déclaration FetchContent épinglée.
- `Source/Core/CMakeLists.txt` : lien de la dépendance à `Core`.

## Vérifications (obligatoires)
- Le projet **configure et compile** avec la dépendance récupérée.
- Un usage minimal de `nlohmann::json` compile (couvert concrètement par le chargeur, TACHE-03).
- La version est **épinglée** (reproductible sur un autre poste, `EX-NFR-030`/`EX-NFR-031`).

## Points d'attention
- *Header-only* : n'entame pas la testabilité de `Core` sans fenêtre ni GPU (`EX-NFR-010`).
- Isoler la récupération (FetchContent) comme les autres dépendances (GoogleTest) ; ne pas
  vendoriser les sources dans le dépôt.
- Éviter d'exposer `nlohmann/json` dans les **en-têtes publics** de `Core` (l'inclure seulement
  dans les `.cpp` de chargement) pour ne pas propager la dépendance aux appelants.

## Définition de fait (DoD)
- Dépendance disponible et liée à `Core`, version épinglée ; build `/W4 /WX` vert.

## Exigences
`EX-NFR-031`, `EX-NFR-030`, `EX-NFR-010`.
