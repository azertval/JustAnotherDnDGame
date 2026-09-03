# LOT-13 — Consolidation de la documentation {#lot-13}

> Statut : **terminé**. Lot de **consolidation documentaire** (sans nouvelle fonctionnalité
> moteur) : mise à jour des pages existantes, **Guide du développeur** exhaustif, et
> **Cahier de test** généré depuis les sources. Clôt la première phase du moteur physique et
> pose le jalon **v0.0.1**.

## Objectif
- **Mettre à jour** les pages Doxygen existantes et le `README.md` pour refléter l'état réel du
  moteur (personnage humanoïde, gravité asymétrique, mécaniques aériennes, mécanismes puzzle).
- **Guide du développeur** : une nouvelle rubrique expliquant **toutes les notions** couvertes par
  le moteur — fonctions détaillées, liens vers les explications **mathématiques** sous-jacentes —
  pour qu'un dev ayant des notions de C++ comprenne l'intégralité du code **en autonomie**.
- **Cahier de test** : documenter **chaque test** (titre, catégorie unitaire/intégration/système,
  sous-catégorie, criticité, étapes détaillées, résultat attendu) via des **balises Doxygen
  avancées** (`\castest`) afin d'en faciliter la **maintenabilité**.

## Périmètre

### Inclus
- **Doxygen / README** : section « Fonctionnalités du moteur (état actuel) », tableau des cibles de
  test (unitaire/intégration/**système**), renvois vers le Guide et le Cahier de test.
- **Guide du développeur** (`Documentation/Guide/`) : page d'accueil + pages par domaine
  (physique, boucle de simulation, ECS, mathématiques, niveaux, entrées), avec liens vers le code
  et les concepts mathématiques.
- **Cahier de test** : mécanisme Doxygen `\castest` (alias + `\xrefitem` agrégeant une page unique)
  et **annotation des 215 cas** de `Source/Test/` (unitaires, intégration, système).

### Exclus
- Toute évolution du **code moteur** (comportement inchangé — lot purement documentaire).

## Décisions de cadrage
- **Cahier de test généré à la source** : chaque test porte un bloc `\castest{...}` (argument
  **unique** délimité par accolades pour tolérer les virgules), avec sous-champs `\tcat`, `\tcrit`,
  `\tetapes`, `\tattendu`. `\xrefitem` **agrège** tous les cas sur la page « Cahier de test ».
  La documentation vit **à côté du test** → maintenue avec lui.
- **Rollout** : les cas **critiques** (balayage AABB, composants, maths) sont enrichis **à la main** ;
  le reste est annoté par un **script consistant** (`\tcat` déduit de la suite GoogleTest), puis
  `clang-format`.
- **Version** : jalon **v0.0.1** (`PROJECT_NUMBER` Doxygen aligné), première base stable du moteur
  physique.

## Exigences couvertes
- `EX-NFR-020` (qualité/documentation), `EX-NFR-021` (traçabilité des tests).
- Consolidation transverse des lots **LOT-01 → LOT-12** (aucune nouvelle exigence fonctionnelle).

## Découpage

> État : ✅ fait.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Mise à jour Doxygen + `README.md` | `Documentation`, `README.md` | ✅ |
| TACHE-02 | Guide du développeur (accueil + 6 domaines) | `Documentation/Guide` | ✅ |
| TACHE-03 | Cahier de test : mécanisme `\castest` + 215 cas annotés | `Documentation/Doxyfile`, `Source/Test` | ✅ |

## Critères d'acceptation du lot
1. `README.md` et les pages Doxygen reflètent l'**état réel** du moteur.
2. Le **Guide du développeur** couvre **toutes** les notions du moteur avec liens code ↔ maths.
3. La page **Cahier de test** liste les **215 cas** (titre, catégorie, sous-catégorie, criticité,
   étapes, résultat attendu), **générée** depuis les annotations des sources.
4. Doxygen **vert** (`WARN_AS_ERROR`), build `/W4 /WX` sans avertissement, **215 tests verts**.
5. Jalon **v0.0.1** posé.

## Dépendances
- Consolide l'ensemble des lots précédents ; aucun changement de comportement moteur.
