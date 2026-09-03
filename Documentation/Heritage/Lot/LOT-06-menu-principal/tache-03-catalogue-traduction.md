# TACHE-03 — Catalogue de traduction (i18n) {#lot-06-tache-03-catalogue-traduction}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Localization`, `Source/Elements/Localization` · **Statut :** à faire

## Contexte
Le menu (TACHE-05) et l'écran éditeur (TACHE-06) affichent des libellés. Plutôt que de coder
ces textes **en dur** dans les écrans, on les fait passer par un **catalogue de traduction** :
chaque texte affiché est désigné par une **clé** stable, résolue vers une chaîne selon la
**langue active**. Ajouter une langue devient trivial (un fichier de plus, aucun code à
changer) et prépare l'internationalisation du jeu et, plus tard, de l'éditeur.

## Travail à réaliser
- **Format du catalogue** : un **fichier texte par langue** (`fr.lang`, puis `en.lang`, …),
  **une paire `clé = valeur` par ligne**, encodage **UTF-8**, commentaires en `#` et lignes
  vides ignorés. Choisi volontairement simple (pas de dépendance JSON à ce stade — le JSON
  est réservé aux niveaux, cf. `EX-LVL-003`) et éditable à la main.
- **Fichier français** `fr.lang` (langue par défaut) portant au minimum les clés du lot :
  `menu.titre`, `menu.jouer`, `menu.mode_edition`, `menu.quitter`, `editeur.a_venir`.
- **Service `Localization`** (dans `HMI/Localization`) :
  - `loadLanguage(id)` charge le fichier `<id>.lang` en mémoire (map clé → valeur) ;
  - `text(clé)` renvoie la chaîne de la langue active ;
  - **repli déterministe** si la clé manque dans la langue active : retomber sur la **langue
    par défaut** (français), sinon renvoyer la **clé elle-même** — jamais de plantage ;
  - langue active changeable à l'exécution (`loadLanguage` d'une autre langue).
- **Clés stables** : les écrans référencent **uniquement des clés**, jamais un littéral d'UI.
- **CMake** : copier `Source/Elements/Localization/*.lang` à côté de l'exécutable (comme les
  autres assets), pour un chargement par chemin relatif au démarrage.

## Fichiers impactés
- `Source/HMI/Localization/Localization.h`, `Localization.cpp` (nouveau).
- `Source/Elements/Localization/fr.lang` (nouveau).
- `Source/HMI/CMakeLists.txt` (source + copie des `.lang`), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Une **clé existante** renvoie la valeur attendue de la langue chargée.
- Une **clé absente** déclenche le repli déterministe (langue par défaut, sinon la clé) sans
  plantage.
- **Parsing** : lignes vides et commentaires `#` ignorés ; espaces autour de `=` tolérés ;
  un `=` présent dans la valeur est préservé.
- **Changement de langue** : après `loadLanguage`, `text(clé)` renvoie la valeur de la
  nouvelle langue ; une clé manquante dans cette langue retombe sur la langue par défaut.
- Le catalogue est testable **sans fenêtre ni GPU** (logique pure, `EX-NFR-010`).

## Points d'attention
- **UTF-8 de bout en bout** : les accents doivent survivre du fichier au rendu (cohérence
  avec la police, TACHE-02).
- **Aucun littéral d'UI en dur** hors des fichiers `.lang` : les écrans passent par les clés.
- Un **fichier de langue manquant** ou une clé absente est une erreur **récupérable**
  (repli + éventuel log), pas un crash (`EX-NFR-040`).
- Pas de sur-ingénierie : ni pluriels, ni genres, ni interpolation d'arguments pour ce lot
  (extensions futures possibles) — hors périmètre.
- Séparer **données** (`.lang`) et **code** (chargeur) : source unique de vérité des textes.

## Définition de fait (DoD)
- Catalogue chargé depuis fichier, `text(clé)` avec repli, changement de langue fonctionnels
  et testés (`ctest` vert) ; **langue française** complète pour les libellés du lot ;
  build `/W4 /WX` sans avertissement, documenté.

## Exigences
`EX-REN-033`, `EX-REN-032`, `EX-NFR-010`, `EX-NFR-040`.
