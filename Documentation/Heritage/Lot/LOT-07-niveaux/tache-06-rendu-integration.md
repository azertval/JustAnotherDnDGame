# TACHE-06 — Rendu du niveau + intégration « Charger niveau » {#lot-07-tache-06-rendu-integration}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
Tâche d'assemblage : donner à voir le niveau chargé. Le `GameScreen` (LOT-06) construit
aujourd'hui une scène codée en dur ; il doit désormais **charger le niveau de démonstration**
(TACHE-05) et **afficher sa grille de tuiles** (`EX-REN-010`), en lecture seule de l'ECS
(`EX-ARCH-012`).

## Travail à réaliser
- `GameScreen` : au lieu de `buildDemoScene`, **charger** le niveau via `LevelLoader`
  (chemin relatif au binaire, cf. TACHE-05) et **peupler le `World`** d'une **entité-tuile par
  tuile** non vide : `Transform` (position `[colonne, ligne]` en unités monde) + `Sprite`
  (région d'atlas / couleur selon le `TileType`).
- **Correspondance type → visuel** : associer chaque `TileType` à une tuile de l'atlas
  (`Solide`, `Danger`, `Entrée`, `Sortie`, `Interrupteur`, `Porte` distincts ; `Vide` non
  dessiné). Centrer/borner la caméra sur le niveau.
- **Échec de chargement récupérable** : si le niveau est introuvable/invalide, ne pas planter —
  journaliser l'erreur et afficher un état neutre (ex. message), `EX-NFR-040`.
- « Charger niveau » (menu) ouvre ce `GameScreen` ; **Échap** revient au menu (inchangé).

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`/`.cpp`.
- Éventuel utilitaire de correspondance `TileType` → région d'atlas (dans `HMI`).

## Vérifications (obligatoires)
- **« Charger niveau »** ouvre le niveau de démo et **sa grille s'affiche** (tuiles solides,
  danger, entrée, sortie, interrupteur/porte visibles) ; Échap revient au menu.
- Un niveau **invalide/introuvable** ne fait **pas planter** l'application (erreur journalisée).
- Vérification **visuelle** de bout en bout (lancer l'exécutable), pas seulement la compilation.

## Points d'attention
- **Réutiliser** le `SpriteRenderer`/`SpriteBatch`/`Camera2D` du LOT-05 : le niveau est rendu
  comme des sprites, sans nouveau pipeline. Lecture seule de l'ECS (`EX-ARCH-012`).
- La grille peut être grande : peupler les entités **au chargement** de l'écran (une fois), pas
  à chaque frame.
- Pas de **déplacement** ni de **collision** ici : uniquement l'affichage du niveau (le gameplay
  est un lot ultérieur).

## Définition de fait (DoD)
- « Charger niveau » ouvre le niveau chargé, la grille s'affiche, échec de chargement géré ;
  build `/W4 /WX`, CI verte, Doxygen et `CHANGELOG.md` à jour ; tous les critères d'acceptation
  de l'[epic](epic.md) satisfaits.

## Exigences
`EX-REN-010`, `EX-GP-001`, `EX-ARCH-012`, `EX-NFR-040`.
