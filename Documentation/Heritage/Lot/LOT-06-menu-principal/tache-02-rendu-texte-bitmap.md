# TACHE-02 — Rendu de texte (police bitmap) {#lot-06-tache-02-rendu-texte-bitmap}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** à faire

## Contexte
Le menu (et les futurs écrans) doivent afficher du **texte** (`EX-REN-032`). On l'obtient
avec une **police bitmap** intégrée, dessinée via le `SpriteBatch` du LOT-05, ce qui reste
cohérent avec le pixel art et le « from scratch ».

## Travail à réaliser
- `BitmapFont` : une texture de police **générée en code** (grille de glyphes de taille
  fixe) couvrant au minimum **A–Z**, **0–9**, l'espace et la ponctuation utile aux libellés
  (« Charger niveau », « Mode Edition », « Quitter », titre), ainsi que les **lettres
  accentuées** nécessaires aux libellés du catalogue de traduction français (é, è, à, ç, É…,
  cf. TACHE-03). Données de glyphes intégrées (déterministes), `ID3D11Texture2D` + vue de
  ressource en RAII.
- Métrique simple : chaque glyphe occupe une cellule de taille connue ; l'avance horizontale
  est fixe (police à chasse fixe) pour ce lot.
- `drawText(SpriteBatch&, texte, position, échelle, couleur)` : émet un quad par glyphe
  (régions de la texture de police), en unités écran/pixels.

## Fichiers impactés
- `Source/HMI/Graphics/BitmapFont.h`, `BitmapFont.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt`.

## Vérifications (obligatoires)
- Le texte s'affiche à la position et à l'échelle demandées (vérification visuelle via le menu).
- Les caractères non couverts sont gérés proprement (glyphe vide ou ignoré, pas de plantage).
- La couleur/teinte s'applique ; la netteté pixel art est préservée (échantillonnage nearest).

## Points d'attention
- Se limiter aux caractères réellement nécessaires ; à chasse fixe pour simplifier la mise
  en page.
- Réutiliser le pipeline `SpriteBatch` (mêmes états blend/nearest) : le texte est un lot de
  quads texturés comme les sprites.
- Rendu de texte en **espace écran** (pixels), indépendant de la caméra du monde.

## Définition de fait (DoD)
- `BitmapFont` + `drawText` fonctionnels, RAII, documentés ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-REN-032`, `EX-REN-011`, `EX-ARCH-022`.
