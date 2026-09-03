# TACHE-05 — Écran de menu principal {#lot-06-tache-05-ecran-menu-principal}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
L'écran d'accueil du jeu. Il présente les trois options et route l'application selon le choix
de l'utilisateur (`EX-REN-030`), au clavier **et** à la souris.

## Travail à réaliser
- `MenuScreen` (implémente `IScreen`) avec les trois entrées : **« Charger niveau »**,
  **« Mode Edition »**, **« Quitter »**.
- **Navigation clavier** : flèches haut/bas pour changer la sélection (front montant),
  **Entrée** pour valider.
- **Navigation souris** : le **survol** d'une option la sélectionne ; le **clic** la valide
  (test de survol sur les rectangles des libellés).
- **Libellés via le catalogue de traduction** (TACHE-03) : les trois entrées et le titre
  référencent des **clés** (ex. `menu.jouer`, `menu.mode_edition`, `menu.quitter`,
  `menu.titre`) résolues par le catalogue — **aucun texte en dur** dans le `MenuScreen`.
- **Rendu** : titre + libellés via `BitmapFont` (TACHE-02), textes issus du catalogue ;
  l'option sélectionnée est mise en évidence (couleur/teinte ou surbrillance).
- **Actions** (transitions, cf. TACHE-04) : « Charger niveau » → écran Jeu ; « Mode Edition »
  → écran Éditeur ; « Quitter » → demande de fermeture.

## Fichiers impactés
- `Source/HMI/Interface/MenuScreen.h`, `MenuScreen.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- La navigation clavier fait défiler la sélection et **boucle** (ou se borne) aux extrémités,
  de façon déterministe.
- Valider une option produit la **transition attendue** (logique de sélection testable en
  injectant un `InputState`, sans fenêtre).
- Le survol souris d'une zone d'option sélectionne cette option.

## Points d'attention
- Séparer la **logique de sélection/transition** (testable) du **dessin** (visuel).
- Cohérence clavier/souris : le survol met à jour la même sélection que les flèches.
- En langue par défaut (français), le catalogue résout exactement « Charger niveau »,
  « Mode Edition », « Quitter » ; le `MenuScreen` n'écrit aucun de ces littéraux directement.

## Définition de fait (DoD)
- Menu navigable clavier + souris, actions câblées, logique testée (`ctest` vert) ;
  build `/W4 /WX`, documenté.

## Exigences
`EX-REN-030`, `EX-REN-032`, `EX-CTRL-011`, `EX-CTRL-001`.
