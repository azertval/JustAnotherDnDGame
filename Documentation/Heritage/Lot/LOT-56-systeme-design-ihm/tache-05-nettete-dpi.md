# TACHE-05 — Netteté des vignettes à toute échelle d'affichage {#lot-56-tache-05-nettete-dpi}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le facteur d'échelle d'affichage du système (`devicePixelRatio`) n'est pris en compte **que** par
*GameViewport*, qui dimensionne correctement sa surface Direct3D 11 et convertit les positions de
souris. Les trois chemins de vignettes Qt de l'éditeur — *PalettePanel* (apparence des types de tuiles),
*AssetThumbnailView* (grilles d'assets, quatre instances à l'écran) et *TexturePanel* (icônes de lignes)
— produisent leurs images à une taille exprimée en pixels **logiques**, sans jamais déclarer leur
échelle.

Sur un écran configuré à 125 % ou 150 % — la configuration par défaut de la plupart des portables
récents — Qt agrandit donc ces images par interpolation. Toute la palette et toutes les grilles
d'assets sont floues.

C'est plus qu'un défaut de finition : le contenu affiché est du **pixel art**, rendu dans le jeu en
filtrage *nearest* (`EX-ARCH-022`) précisément pour que chaque pixel reste carré. Une vignette
interpolée montre à l'auteur du niveau autre chose que ce que le jeu affichera.

## Travail à réaliser
- **Rendu des vignettes à la résolution réelle** : produire l'image à la taille demandée **multipliée
  par le facteur d'échelle** du support d'affichage, puis déclarer ce facteur sur l'image, de sorte que
  Qt l'affiche à la bonne taille logique sans la redimensionner.
- **Agrandissement sans lissage** : tout agrandissement d'un asset pixel art se fait au plus proche
  voisin, jamais par interpolation, conformément à `EX-ARCH-022`.
- **Application aux trois chemins** : *PalettePanel*, *AssetThumbnailView* et *TexturePanel*, y compris
  la vignette d'aperçu animée de l'onglet Animations.
- **Facteur lu sur le support réel** et non supposé : il peut différer d'un écran à l'autre, et un
  déplacement de la fenêtre d'un écran à l'autre doit régénérer les vignettes.
- **Tailles issues des jetons** (TACHE-01), en remplacement des constantes locales à chaque widget.
- **Calcul exposé, pas enfoui** : la fonction pure de dimensionnement doit être utilisable depuis un
  autre widget que ces trois-là. Le canevas de [LOT-54](@ref lot-54) est une quatrième surface de
  pixel art agrandie au plus proche voisin et devra l'appeler ; une fonction privée à *PalettePanel*
  y serait réécrite, et le défaut reviendrait par la porte de service.

## Fichiers impactés
- `Source/HMI/Editor/PalettePanel.{h,cpp}`.
- `Source/HMI/Editor/AssetThumbnailView.{h,cpp}`.
- `Source/HMI/Editor/TexturePanel.{h,cpp}`.
- `Source/Test/Unit/HMI/Editor/test_thumbnail_geometry.cpp` (nouveau).

## Tests (obligatoires)
- **Dimensionnement** : pour une taille logique et un facteur d'échelle donnés, la taille en pixels
  réels attendue est produite — vérifiée aux facteurs 1, 1,25, 1,5 et 2. Calcul exposé en fonction
  **pure**, testé sans GPU ni instance d'application.
- **Facteur non entier** : un facteur de 1,25 ne doit pas produire de dimension nulle ni de nombre de
  pixels arrondi à zéro pour la plus petite taille d'icône employée.
- **Aucune taille de vignette codée en dur** ne subsiste dans les trois widgets : elles proviennent
  toutes des jetons.

## Points d'attention
- **Déclarer le facteur sans agrandir l'image produit une vignette deux fois trop petite ; agrandir
  sans déclarer le facteur produit une vignette floue.** Les deux gestes vont ensemble, et l'oubli de
  l'un est l'erreur classique.
- **Vérifier réellement à 125 % et 150 %**, pas seulement à 100 % et 200 % : les facteurs non entiers
  sont ceux qui révèlent les erreurs d'arrondi, et ce sont les plus répandus.
- Le déplacement de la fenêtre vers un écran d'échelle différente doit régénérer les vignettes — sinon
  le défaut réapparaît après coup, sans que rien ne le signale.
- Ne pas confondre avec le chemin du viewport : *GameViewport* gère déjà correctement son échelle, et
  n'entre pas dans cette tâche.
- Le coût mémoire augmente avec le carré du facteur d'échelle. Vérifier que la palette et les grilles
  d'assets restent fluides avec un grand nombre d'assets importés.

## Définition de fait (DoD)
- Les vignettes de la palette, des grilles d'assets et des lignes du panneau Textures restent nettes à
  100 %, 125 % et 150 % d'échelle d'affichage, sans lissage du pixel art ; le déplacement entre écrans
  d'échelles différentes les régénère ; le calcul de dimension est pur et testé ; aucune taille n'est
  codée en dur ; `/W4 /WX` propre.

## Exigences
`EX-IHM-053` (netteté à toute échelle d'affichage) ; réutilise `EX-IHM-051` (tailles issues des
jetons), `EX-ARCH-022` (pixel art net, filtrage *nearest*), `EX-EDIT-027` (la palette affiche
l'apparence réelle des tuiles).
