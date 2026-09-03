# TACHE-02 — Raccords automatiques : voisinage solide → case de planche {#lot-42-tache-02-raccords-automatiques}

**Lot :** [LOT-42](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Une image unique par type de tuile répète la même case partout : un mur de vingt tuiles affiche
vingt fois le même motif, la grille reste visible, et le dessus d'une plateforme est indiscernable
de son intérieur. C'est exactement la limite du rendu en couleurs plates, transposée en couleur.

La solution standard consiste à choisir l'image affichée **en fonction des voisins solides** de la
case, dans une planche fournie par l'auteur. Sur quatre voisins (haut, bas, gauche, droite), il y a
seize configurations — d'où une planche de 4×4 cases.

## Travail à réaliser
- **Fonction pure de masque** : à partir de la grille (`core::TileMap`) et d'une position, produire
  un masque de quatre bits indiquant quels voisins orthogonaux sont solides, via
  `core::isSolid`/`TileMap::isSolid`. **Aucune nouvelle surface `Core`** n'est requise.
- **Table masque → case** : correspondance des seize valeurs vers un couple (colonne, ligne) dans
  une planche 4×4. La table est une constante nommée, documentée case par case (bord haut, coin
  concave bas-droite, intérieur, tuile isolée…).
- **Bord du niveau** : décider explicitement si l'extérieur de la grille compte comme solide ou
  comme vide. Traiter l'extérieur comme **solide** évite qu'un mur de bordure affiche un contour sur
  sa face invisible ; c'est la convention à retenir, et elle doit être écrite, pas implicite.
- **Application** : en mode `bitmask16`, la région d'atlas d'une tuile est calculée à partir de la
  planche et du masque ; en mode `single`, l'image entière est utilisée.
- **Contrat de planche** : 4×4 cases de `TILE_SIZE`, déclaré dans le contrat d'asset (LOT-40) donc
  validé au chargement, et documenté dans `Source/Elements/Assets/README.md`.

## Fichiers impactés
- `Source/HMI/Graphics/TileAutotile.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/TileVisuals.{h,cpp}` (résolution étendue au mode).
- `Source/Elements/Assets/README.md` (convention de planche).
- `Source/Test/Unit/HMI/Graphics/test_tile_autotile.cpp` (nouveau).

## Tests (obligatoires)
- **Les seize configurations** de masque produisent la case attendue — test exhaustif, la table
  étant petite et figée.
- Calcul du masque sur une grille de test : coin, bord, intérieur, tuile isolée, voisinage mixte.
- Comportement au bord du niveau, conforme à la convention retenue.
- Tout est **pur** : ni GPU, ni Qt, ni fichier.

## Points d'attention
- **Le voisinage est celui de la solidité, pas du type.** Deux types solides différents (`Solid` et
  `Block`) doivent-ils se raccorder ? Trancher explicitement : se raccorder au **même type** est plus
  prévisible visuellement, se raccorder à **toute solidité** évite les coutures. Documenter le choix.
- Les pentes et arrondis ne sont **jamais** solides (`core::isSolid` les exclut) : ils ne
  participent donc pas au voisinage et restent en mode `single` avec leur masque de silhouette
  (TACHE-03). C'est cohérent, mais il faut vérifier que le résultat visuel ne laisse pas de couture
  visible entre une pente et le sol plein qu'elle rejoint.
- Le masque dépend de la grille **de collision**, qui peut différer de la grille du niveau quand une
  porte est fermée (`MechanismController::collisionMap`). Utiliser la grille **du niveau** : une
  porte qui s'ouvre ne doit pas faire changer l'habillage des murs voisins.

## Définition de fait (DoD)
- Une planche 4×4 produit des bords, coins et intérieurs distincts selon le voisinage ; les seize
  cas sont testés ; le comportement au bord et la règle de raccord entre types sont documentés ;
  `/W4 /WX` propre.

## Exigences
`EX-EDIT-025` (raccords automatiques) ; réutilise `EX-EDIT-042` (association type → texture),
`EX-REN-007` (contrat d'asset), `EX-NFR-010` (testable sans GPU), `EX-GP-001` (tuiles solides).
