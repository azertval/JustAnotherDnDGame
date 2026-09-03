# TACHE-01 — Qt épinglé sur les trois environnements {#lot-66-tache-01-qt-epingle}

**Lot :** [LOT-66](epic.md) · **Emplacement :** `Source/HMI/CMakeLists.txt`, `.github/workflows` ·
**Statut :** fait

## Contexte
`EX-BUILD-010` demande qu'une dépendance non gérable par `FetchContent` — Qt, explicitement — soit
provisionnée de façon reproductible sur les **trois** environnements, avec une version **épinglée**.

Deux le sont : `ci.yml` et `release.yml` installent `6.8.1` via `install-qt-action`. Le troisième,
le poste local, ne l'est pas. `Source/HMI/CMakeLists.txt` complète la recherche par les emplacements
conventionnels de l'installateur officiel :

```
file(GLOB _qt_installs "$ENV{SystemDrive}/Qt/*/msvc*_64" "C:/Qt/*/msvc*_64" …)
list(SORT _qt_installs COMPARE NATURAL ORDER DESCENDING)
```

Ce mécanisme est bon — il a résolu un vrai problème, celui de la cible ignorée en silence sur un
poste neuf — mais son tri décroissant retient la version **la plus récente installée**. Un
développeur ayant Qt 6.10 construit contre 6.10 ; la CI valide contre 6.8.1 ; et rien, nulle part,
ne le dit. Le `LOT-60` aggrave l'enjeu en ajoutant un module.

## Travail à réaliser
- **Déclarer la version de référence** à un seul endroit du CMake, et l'utiliser comme **version
  minimale** dans `find_package(Qt6 ...)`.
- **Avertir en cas d'écart** : si la version trouvée diffère de celle de la CI, émettre un
  `message(WARNING)` nommant les deux versions et rappelant que la CI valide contre l'autre. Un
  avertissement, pas une erreur : interdire de construire sur une version voisine coûterait plus
  qu'il ne rapporterait.
- **Journaliser la version retenue** au démarrage, dans le contexte de [LOT-61](@ref lot-61) — un
  rapport de défaut doit permettre de savoir contre quel Qt le binaire a été construit.
- **Vérifier la cohérence CI ↔ CMake** : la version épinglée dans les workflows et celle du CMake
  doivent être la même. Un contrôle léger (script existant ou étape de workflow) évite qu'elles
  divergent silencieusement — c'est exactement l'erreur que `build_docs.py` prévient déjà pour le
  numéro de version.
- **Documenter** l'installation locale de la bonne version, modules compris.

## Fichiers impactés
- `Source/HMI/CMakeLists.txt`.
- `.github/workflows/ci.yml`, `.github/workflows/release.yml`.
- `README.md` (prérequis), `CONTRIBUTING.md`.
- `Documentation/Specification/exigences-non-fonctionnelles.md` — `EX-BUILD-010` mentionne le
  contrôle.

## Tests (obligatoires)
- Configurer avec une version de Qt différente de la référence produit l'avertissement attendu, et
  la configuration **aboutit** quand même.
- Configurer avec une version inférieure à la version minimale échoue avec un message qui nomme la
  version attendue.
- La version épinglée dans les workflows et celle du CMake sont identiques — vérifié
  automatiquement, pas par relecture.
- Le comportement de découverte automatique sur un poste neuf est **inchangé** : la cible n'est
  toujours pas ignorée en silence, et le message d'aide existant reste affiché quand Qt est absent.

## Points d'attention
- **Ne pas casser la découverte automatique.** Le bloc de recherche existe parce qu'un poste neuf
  produisait un build « réussi » sans exécutable. Cette tâche ajoute une vérification de version, et
  ne touche pas à la recherche elle-même.
- **Avertissement, pas erreur.** Rendre l'écart bloquant obligerait à désinstaller une version de Qt
  pour construire — la première réaction serait de désactiver le contrôle.
- Ne pas coder la version en dur à quatre endroits : c'est reproduire, pour Qt, le problème que la
  `TACHE-02` corrige pour le numéro de version.
- `install-qt-action` apparaît **quatre fois** entre les deux workflows : les traiter tous.

## Définition de fait (DoD)
- La version de Qt est déclarée une fois, exigée comme minimale, signalée en cas d'écart avec la CI,
  journalisée au démarrage, et la cohérence CI ↔ CMake est vérifiée automatiquement ; la découverte
  automatique sur poste neuf est inchangée ; documenté.

## Exigences
Réutilise `EX-BUILD-010` (provisionnement reproductible et épinglé), `EX-NFR-031` (dépendances
épinglées), `EX-NFR-030` (build via CMake reproductible), `EX-NFR-042` (contexte journalisé).
