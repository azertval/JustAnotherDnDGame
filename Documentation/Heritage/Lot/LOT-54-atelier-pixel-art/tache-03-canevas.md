# TACHE-03 — Canevas : rendu net et surface invariante {#lot-54-tache-03-canevas}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le widget qui montre l'image en cours d'édition et transforme les gestes de souris en appels aux
opérations de la TACHE-02. Tout repose sur des primitives déjà présentes : Qt Widgets depuis le
`LOT-34`, `QImage` et `QPainter`.

Deux contraintes viennent de [LOT-56](@ref lot-56) et changent la nature de cette tâche par rapport
au cadrage initial du lot.

La première est la **netteté**. Le canevas est une surface de pixel art agrandie au plus proche
voisin — exactement le sujet de la TACHE-05 de LOT-56, qui corrige les trois chemins de vignettes
produisant leurs images en pixels **logiques** et donc interpolées à 125 % ou 150 % d'échelle
d'affichage. Écrire ici un quatrième chemin ignorant `devicePixelRatio` reproduirait le défaut le
jour même où il vient d'être corrigé, et sur le widget où il serait le plus visible : un éditeur de
pixels dont les pixels sont flous.

La seconde est la **provenance des couleurs**. La grille, le damier de transparence, le fond et les
poignées de sélection sont des choix de couleur ; LOT-56 a fait des jetons la source unique de ces
grandeurs, et son critère d'acceptation interdit qu'une constante de style subsiste dans le code
d'un widget.

## Travail à réaliser
- **Canevas** (`QWidget`) affichant une `QImage` en cours d'édition, à un zoom **entier**, avec une
  grille de pixels visible au-delà d'un certain facteur et un **damier de transparence** sous les
  zones à alpha nul.
- **Zoom et déplacement de la vue**, sur le modèle du canevas de niveau (`EX-EDIT-013`).
- **Dimensionnement à l'échelle d'affichage réelle** : réutiliser la fonction pure de dimensionnement
  livrée par la TACHE-05 de [LOT-56](@ref lot-56) plutôt que d'en écrire une seconde, et déclarer le
  facteur sur l'image produite. Le facteur est lu sur le support réel : déplacer la fenêtre d'un
  écran à l'autre régénère le rendu.
- **Agrandissement au plus proche voisin**, jamais interpolé (`EX-ARCH-022`).
- **Couleurs et grandeurs issues des jetons** de la TACHE-01 de LOT-56 : aucune constante de style,
  aucune couleur littérale dans ce widget.
- **Surface de peinture invariante** : le fond et le damier de transparence prennent des jetons de la
  portée **invariante**, et ne changent donc pas avec le thème clair/sombre de l'éditeur (TACHE-06 de
  LOT-56). Le châssis autour du canevas, lui, suit le thème.
- **Conversion position de souris → pixel de l'image**, exposée en fonction **pure** : c'est elle qui
  détermine le pixel survolé affiché dans la barre d'état (TACHE-04), et elle doit rester juste à
  tout zoom, à tout décalage de vue et à toute échelle d'affichage.
- **Regroupement des gestes** : appui, glisser, relâchement produisent **une** opération d'historique
  (TACHE-02), pas une par déplacement de souris.

## Fichiers impactés
- `Source/HMI/Editor/PixelCanvas.{h,cpp}` (nouveau) — widget.
- `Source/HMI/Editor/PixelCanvasGeometry.{h,cpp}` (nouveau) — conversions pures vue ↔ image.
- `Source/HMI/Interface/DesignTokens.h` (créé en `LOT-56`) — jetons de la surface de peinture.
- `Source/Test/Unit/HMI/Editor/test_pixel_canvas_geometry.cpp` (nouveau).

## Tests (obligatoires)
- **Conversion vue → image** : pour une combinaison de zoom, de décalage de vue et de facteur
  d'échelle d'affichage, la position de souris donne le bon pixel ; une position hors de l'image
  produit une absence de pixel, pas un indice tronqué au bord.
- **Aller-retour** : le centre du rectangle d'un pixel reconverti donne ce même pixel, à tous les
  zooms testés.
- **Dimensionnement** : la taille en pixels réels attendue est produite aux facteurs 1, 1,25, 1,5 et
  2, en réutilisant la fonction de LOT-56 — un test vérifie qu'elle est bien appelée et non
  redéfinie.
- **Zoom entier** : aucun facteur fractionnaire n'est atteignable par les commandes de zoom.
- **Aucune couleur en dur** : le widget n'expose aucune constante de style ; ses couleurs proviennent
  des jetons.
- Conversions **pures**, testées sans Qt ni GPU.

## Points d'attention
- **Déclarer le facteur d'échelle sans agrandir l'image produit un canevas deux fois trop petit ;
  agrandir sans déclarer le facteur produit un canevas flou.** Les deux gestes vont ensemble — c'est
  le piège relevé par la TACHE-05 de LOT-56, et il s'applique identiquement ici.
- **Le zoom doit rester entier** pour que les pixels restent carrés et nets ; un zoom fractionnaire
  fait apparaître des pixels de largeurs inégales.
- Le damier de transparence est dessiné dans l'espace de l'**écran**, pas dans celui de l'image :
  sinon il se met à zoomer avec le dessin et devient illisible aux forts facteurs.
- Ne pas dupliquer la logique d'entrée du canevas de niveau : ce sont deux surfaces distinctes, mais
  la convention de déplacement de vue et de zoom doit rester la même pour l'utilisateur.

## Définition de fait (DoD)
- Le canevas affiche une image à zoom entier avec grille et damier, se déplace et zoome ; il reste
  net aux échelles d'affichage 100 %, 125 % et 150 % ; sa surface de peinture est identique en thème
  clair et sombre ; toutes ses couleurs viennent des jetons ; les conversions sont pures et testées ;
  `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-EDIT-013` (déplacement et zoom),
`EX-IHM-051` (source unique des grandeurs), `EX-IHM-053` (netteté à toute échelle d'affichage),
`EX-IHM-050` (thème en deux portées), `EX-ARCH-022` (pixel art net), `EX-NFR-010` (testable sans
GPU).
