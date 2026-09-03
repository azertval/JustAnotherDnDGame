# LOT-06 — Menu principal {#lot-06}

> Statut : **terminé**. Au lancement, l'exécutable ouvre un **menu principal** navigable (clavier + souris) piloté par une structure d'**écrans**, avec les briques introduites en chemin : entrées clavier/souris, rendu de texte et catalogue de traduction (i18n). Un **sélecteur de langue** (bouton drapeau, fr/en) complète l'i18n (TACHE-08).

## Objectif
Afficher au démarrage un **menu principal** navigable proposant trois options —
**« Charger niveau »**, **« Mode Edition »** et **« Quitter »** — et router l'application vers
l'écran correspondant. En préalable, poser les briques manquantes : un module d'**entrées**
(clavier + souris), un **rendu de texte** (police bitmap) et un **catalogue de traduction**
(i18n) par lequel passent tous les textes affichés.

À l'issue du lot : au lancement, l'exécutable ouvre le menu ; on navigue au clavier et à la
souris ; « Quitter » ferme l'application, « Charger niveau » ouvre la scène de démonstration
(niveau provisoire) et « Mode Edition » un écran « à venir », tous deux avec retour au menu
par **Échap**.

## Périmètre

### Inclus
- **Entrées** : état clavier et souris exposé par `Window`, avec distinction **pressée /
  maintenue / relâchée** (`EX-CTRL-011`), échantillonné une fois par frame (`EX-CTRL-021`).
- **Rendu de texte** : police **bitmap** intégrée et fonction de dessin de texte via le
  `SpriteBatch` existant (`EX-REN-032`).
- **Catalogue de traduction (i18n)** : tous les textes affichés passent par des **clés**
  résolues depuis un fichier par langue (français par défaut) ; ajouter une langue = ajouter
  un fichier, sans toucher au code.
- **États d'application (écrans)** : écran courant et transitions (Menu / Jeu / Éditeur).
- **Écran de menu principal** : trois options, surbrillance de la sélection, navigation
  **clavier + souris**, actions câblées (`EX-REN-030`).
- **Écrans cibles** : « Charger niveau » réutilise la scène de démo du LOT-05 (niveau
  provisoire) ; « Mode Edition » affiche un écran « à venir » ; retour au menu par Échap.
- **Intégration `main`** : la boucle pilote l'écran courant ; la scène codée en dur quitte
  `main` pour l'écran de jeu.

### Exclus (lots ultérieurs)
- **Chargement de niveaux depuis fichier** (« Charger niveau » ouvre la démo, pas un fichier).
- **Éditeur** réel (`EX-EDIT-*`) : « Mode Edition » est un placeholder.
- Écrans de **pause** et de **fin de niveau** (`EX-REN-031`).
- Manette (`EX-CTRL-002`), remapping des touches (`EX-CTRL-012`), audio.

## Décisions de cadrage
- **Texte** : police **bitmap** intégrée (dessinée via `SpriteBatch`), cohérente avec le
  pixel art et l'esprit « from scratch ».
- **Internationalisation** : tout texte affiché passe par un **catalogue de traduction**
  (clé → chaîne, un fichier par langue, français par défaut). Aucun littéral d'UI en dur dans
  les écrans, pour rendre l'ajout d'une langue trivial. Format volontairement simple
  (`clé = valeur`, pas de dépendance JSON à ce stade).
- **Navigation** : **clavier + souris** (flèches/Entrée et survol/clic).
- **Actions** : « Charger niveau » → scène de démo (niveau provisoire) ; « Mode Edition » →
  écran « à venir » ; « Quitter » → fermeture. Retour menu par Échap.

## Exigences couvertes
- `EX-REN-030` (menu principal), `EX-REN-032` (texte), `EX-REN-033` (catalogue de
  traduction), `EX-ARCH-012` (le rendu lit l'état).
- `EX-CTRL-011` (pressée/maintenue/relâchée), `EX-CTRL-021` (entrées échantillonnées une
  fois par frame, en amont de la logique).
- `EX-NFR-040` (fichier/​clé de traduction manquant traité comme erreur récupérable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-entrees-clavier-souris.md) | Entrées clavier & souris | `HMI/Input` | ✅ Fait |
| [TACHE-02](tache-02-rendu-texte-bitmap.md) | Rendu de texte (police bitmap) | `HMI/Graphics` | ✅ Fait |
| [TACHE-03](tache-03-catalogue-traduction.md) | Catalogue de traduction (i18n) | `HMI/Localization` | ✅ Fait |
| [TACHE-04](tache-04-etats-application.md) | États d'application (écrans) | `HMI/Interface` | ✅ Fait |
| [TACHE-05](tache-05-ecran-menu-principal.md) | Écran de menu principal | `HMI/Interface` | ✅ Fait |
| [TACHE-06](tache-06-ecrans-cibles.md) | Écrans cibles (jeu démo + éditeur placeholder) | `HMI/Interface` | ✅ Fait |
| [TACHE-07](tache-07-integration-main.md) | Intégration `main` (boucle pilotée par l'écran) | `HMI/main.cpp` | ✅ Fait |
| [TACHE-08](tache-08-selecteur-langue.md) | Sélecteur de langue (bouton drapeau fr/en) | `HMI/Interface`, `HMI/Graphics` | ✅ Fait |

## Critères d'acceptation du lot
1. Au lancement, l'exécutable affiche le **menu principal** avec les trois options.
2. La sélection se fait **au clavier** (flèches + Entrée) **et à la souris** (survol + clic) ;
   l'option sélectionnée est mise en évidence.
3. **« Quitter »** ferme l'application ; **« Charger niveau »** ouvre la scène de démo ;
   **« Mode Edition »** ouvre l'écran « à venir » ; **Échap** revient au menu depuis ces écrans.
4. Les entrées distinguent pressée/maintenue/relâchée et sont lues **une fois par frame**.
5. Tous les textes affichés proviennent du **catalogue de traduction** (aucun littéral d'UI
   en dur) ; charger un autre fichier de langue change les textes sans recompiler.
6. Build `/W4 /WX` sans avertissement, tests verts (logique testable : entrées, catalogue de
   traduction, sélection de menu), documentation Doxygen à jour, `CHANGELOG.md` mis à jour.

## Dépendances
- Réutilise le rendu 2D de [LOT-05](@ref lot-05) (`SpriteBatch`, `Camera2D`, `SpriteRenderer`,
  scène de démo) et l'ECS de [LOT-03](@ref lot-03).
- S'appuie sur `hmi::Window` (LOT-01) pour la capture des événements d'entrée.

## Navigation des tâches
- @subpage lot-06-tache-01-entrees-clavier-souris
- @subpage lot-06-tache-02-rendu-texte-bitmap
- @subpage lot-06-tache-03-catalogue-traduction
- @subpage lot-06-tache-04-etats-application
- @subpage lot-06-tache-05-ecran-menu-principal
- @subpage lot-06-tache-06-ecrans-cibles
- @subpage lot-06-tache-07-integration-main
- @subpage lot-06-tache-08-selecteur-langue
