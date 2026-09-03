# TACHE-03 — Section « Objets » et priorité de résolution {#lot-45-tache-03-section-objets-priorite}

**Lot :** [LOT-45](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Graphics`, `Source/HMI/CMakeLists.txt` · **Statut :** fait

## Contexte
Avec ce lot, une case peut recevoir son apparence de trois sources concurrentes : sa **surcharge**
propre (LOT-45), le **skin** de son type dans le jeu courant (LOT-42), ou le **repli** en damier
(LOT-40). L'ordre entre les trois devient un contrat qui sera invoqué par tous les lots suivants —
et délibérément **inversé** par LOT-51 pour son affichage isolé.

C'est donc moins une fonctionnalité qu'une règle à figer et à verrouiller par un test.

## Travail à réaliser
- **Résolveur de priorité**, dans le point de résolution unique introduit en LOT-41 :
  **surcharge > skin du jeu courant > damier magenta**. Une seule implémentation, partagée par le
  jeu, l'éditeur et la palette.
- **Portée de la priorité** : elle détermine **quel asset** est affiché. Le choix du **clip**
  d'animation au sein de cet asset relève de LOT-46/47 et n'entre pas ici — une surcharge désignant
  un asset animé s'affichera en image fixe jusqu'à LOT-46, sans que le format change ensuite.
- **Section « Objets »** du panneau « Textures » : liste des surcharges du niveau courant (position
  et asset), avec sélection croisée avec le canevas et retrait depuis la liste. Pas un panneau
  séparé.
- **Dossier `Assets/Objects/`** + copie `POST_BUILD` dans `Source/HMI/CMakeLists.txt`.
- Traduction de toutes les chaînes.

## Fichiers impactés
- `Source/HMI/Graphics/TileVisuals.{h,cpp}` (résolution étendue à la surcharge).
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/Elements/Assets/Objects/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Graphics/test_texture_resolution.cpp` (nouveau).

## Tests (obligatoires)
- **Les huit combinaisons** de la table de priorité : surcharge présente ou absente × skin présent
  ou absent × asset trouvé ou introuvable. Fonction **pure**, sans GPU. C'est le test qui verrouille
  le contrat pour les dix lots suivants.
- Une surcharge sur une case dont le type a un skin **différent** affiche bien la surcharge —
  asserté via le *QuadRecorder*.
- La liste des surcharges du panneau reflète le brouillon courant, y compris après annulation.

## Points d'attention
- **Une seule implémentation du résolveur.** Si la palette, le canevas d'édition et le jeu résolvent
  chacun de leur côté, ils divergeront — et l'éditeur cessera de montrer ce que le joueur verra.
- Une surcharge dont l'asset est **introuvable** doit afficher le damier, pas retomber
  silencieusement sur le skin du type : sinon l'auteur croit son assignation prise en compte alors
  qu'elle est cassée.
- La section « Objets » peut lister beaucoup d'entrées sur un grand niveau : prévoir un tri stable
  (par position) et un filtre, pas une liste brute.

## Définition de fait (DoD)
- La priorité surcharge > skin > damier est appliquée par un résolveur unique et testée
  exhaustivement ; la section « Objets » liste et permet de retirer les surcharges ; le dossier est
  déployé ; chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-043` (texture par instance, prioritaire) ; réutilise `EX-EDIT-042` (skin, priorité
inférieure), `EX-REN-043` (calques), `EX-REN-046` (bascule), `EX-NFR-040` (repli), `EX-NFR-004`
(vérification sans GPU), `EX-REN-033` (traduction).
