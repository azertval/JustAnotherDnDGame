# TACHE-02 — *TextRenderer* : composition d'une chaîne en quads {#lot-52-tache-02-text-renderer}

**Lot :** [LOT-52](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Une fois la police chargée (TACHE-01), afficher du texte revient à émettre un quad par glyphe — soit
exactement ce que `hmi::SpriteBatch` fait déjà pour les sprites. Aucun nouveau chemin de rendu,
aucune nouvelle dépendance : c'est ce qui rend cette brique peu coûteuse malgré son intitulé.

Le calque `UI` de *RenderLayer* a été réservé en LOT-40 sans être utilisé ; cette tâche l'active.

## Travail à réaliser
- ***TextRenderer*** : à partir d'une chaîne, d'une police, d'une position et d'une échelle, produire
  la suite de quads correspondante, sur le calque `UI`.
- **Ancrage** : position d'ancrage paramétrable (gauche, centre, droite ; haut, milieu, bas) —
  centrer un texte sans devoir mesurer soi-même est le besoin le plus courant.
- **Espace écran, pas espace monde** : le HUD ne doit ni tourner, ni changer de taille avec le zoom
  de la caméra. Le calque `UI` utilise donc une projection distincte de celle de `Camera2D`, calculée
  à partir des dimensions du viewport.
- **Netteté** : positions arrondies au pixel écran entier ; le rendu est en pixel art avec filtrage
  *nearest* (`EX-ARCH-022`), un texte à position fractionnaire serait tremblant.
- **Teinte** : couleur du texte via la teinte du quad, sans variante de police.
- Retour à la ligne uniquement sur caractère explicite ; pas de justification ni de césure
  automatique (hors périmètre du lot).

## Fichiers impactés
- `Source/HMI/Graphics/TextRenderer.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}` (passe du calque `UI` avec sa propre projection).
- `Source/Test/Unit/HMI/Graphics/test_text_renderer.cpp` (nouveau).

## Tests (obligatoires)
- **Composition** : une chaîne de N caractères produit N quads (hors espaces si le choix est de ne
  pas les émettre), aux positions attendues selon l'avance de chaque glyphe.
- **Ancrage** : pour chaque combinaison d'ancrage, la boîte du texte est positionnée comme attendu.
- Chaîne vide → aucun quad.
- Chaîne accentuée → nombre de quads correspondant aux **points de code**, pas aux octets.
- Sans GPU, via la composition de quads (*QuadRecorder*, LOT-40).

## Points d'attention
- **Deux projections dans une même image.** Le calque `UI` n'utilise pas la matrice de `Camera2D` :
  c'est le premier cas du projet où une passe de rendu a sa propre projection. Le vérifier
  explicitement, notamment au redimensionnement de la fenêtre et au passage en plein écran.
- Le texte ne doit **pas** être soumis au culling par salle (LOT-40, TACHE-05) : il est en espace
  écran, il n'a pas de position monde.
- Ne pas recomposer la chaîne à chaque image si elle n'a pas changé, mais ne pas non plus
  sur-optimiser : quelques dizaines de quads par image sont négligeables.

## Définition de fait (DoD)
- Une chaîne s'affiche dans le viewport, nette, correctement ancrée, en espace écran, sans être
  affectée par le zoom ni le culling ; la composition est testée sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-REN-032` (affichage de texte dans la scène) ; réutilise `EX-REN-043` (calques et multi-textures),
`EX-ARCH-022` (*nearest*), `EX-NFR-004` (vérification sans GPU), `EX-NFR-005` (culling, dont le
texte est exclu).
