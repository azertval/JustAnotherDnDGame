# TACHE-08 — Aperçu live et mode planche à raccords {#lot-54-tache-08-apercu-live-planche}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Graphics` · **Statut :** fait

> **Amendement de portée.**
> - **Aperçu live** : livré tel que cadré, via `GameViewport::invalidateAsset` (invalidation
>   ciblée, déjà prévue depuis LOT-43). Comme `TextureCache` lit depuis le disque, un aperçu
>   *avant* tout premier enregistrement suppose une écriture — `MainWindow::updateLivePreview`
>   écrit silencieusement sur `_pixelAssetPath` à chaque geste complet (regroupé, jamais par
>   pixel) **une fois qu'un chemin existe** (après le premier « Enregistrer »/« Enregistrer
>   sous »). Un tout nouvel asset pas encore enregistré une première fois n'a donc pas d'aperçu
>   live — cohérent avec le point d'attention « rien à montrer » ci-dessous.
> - **Repères des seize cases** et **aperçu d'assemblage 3×3** : livrés, dessinés directement
>   dans `PixelCanvas` (grille de case épaisse + incrustation de l'assemblage dans le coin), et
>   non comme un second panneau séparé — plus simple, toujours visible sans occuper un onglet.
> - **Case survolée** : livrée comme **infobulle** (`QToolTip`) sur le canevas plutôt que comme
>   zone de la barre d'état structurée — évite de complexifier davantage le modèle à six zones de
>   `hmi::EditorStatus`, pour une information qui n'a de sens qu'au survol immédiat de la case.
> - **Aperçu d'animation** : **non livré**. Nécessite d'intégrer `hmi::AnimationCatalog`/
>   `TextureCache::getAnimation` (LOT-46) au canevas pour rejouer un clip — un point d'extension
>   clair (la description est déjà résolue par le cache), laissé à une passe ultérieure faute de
>   temps dans celle-ci.

## Contexte
Sans cette tâche, l'atelier n'apporterait presque rien : un éditeur d'images externe fait déjà mieux
que ce que LOT-54 peut livrer en un lot. Ce qui le justifie, c'est d'être **intégré** — voir
immédiatement, dans le niveau, ce que l'on est en train de peindre.

S'y ajoute un besoin créé par LOT-42 : dessiner une **planche à raccords** de seize cases, où chaque
case correspond à une configuration de voisinage précise, est impossible sans repères. Un auteur
livré à lui-même se tromperait de case une fois sur deux.

## Travail à réaliser
- **Aperçu live** : à chaque modification significative (fin de coup de pinceau, remplissage), mettre
  à jour le rendu du niveau, via l'invalidation ciblée du *TextureCache* (`invalidate(name)`, API
  prévue depuis LOT-40, exploitée par LOT-43).
  - Ne pas invalider à chaque pixel : regrouper par geste, sinon le rechargement dominerait le coût.
  - L'aperçu doit fonctionner que l'asset soit utilisé comme skin, comme surcharge, ou comme décor.
- **Mode planche à raccords** : quand l'asset édité est utilisé en mode `bitmask16` (LOT-42),
  afficher sur le canevas les **repères des seize cases** et, pour chacune, l'indication de la
  configuration de voisinage qu'elle représente (bord haut, coin concave bas-droite, intérieur,
  tuile isolée…).
- **Aperçu de raccord** : une vue montrant un assemblage type (un bloc 3×3 de tuiles) construit avec
  la planche en cours — c'est ce qui permet de voir si les bords se raccordent réellement. Il était
  optionnel au cadrage initial ; il ne l'est plus. Les repères disent à quoi *sert* chaque case ;
  seul l'assemblage dit si le résultat *tient*, et c'est la question que se pose l'auteur.
- **Case survolée dans la barre d'état** : en mode planche, le contexte d'atelier (TACHE-04) indique
  quelle case de la planche est sous le curseur et quelle configuration de voisinage elle décrit.
- **Aperçu d'animation** : si l'asset porte une description d'animation (LOT-46), jouer le clip dans
  le canevas.

## Fichiers impactés
- `Source/HMI/Editor/PixelCanvas.{h,cpp}`.
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (invalidation ciblée, déjà disponible).
- `Source/HMI/Graphics/TileAutotile.{h,cpp}` (libellés des seize configurations, réutilisés).
- `Source/HMI/Editor/EditorStatus.{h,cpp}` (créé en `LOT-57`) — case de planche survolée.
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- **Libellés des seize cases** : chaque index de la table de raccords a un libellé, et il correspond
  bien à la configuration de voisinage de la table (LOT-42, TACHE-02). Fonction pure.
- Regroupement des invalidations : un geste continu ne produit qu'une invalidation, pas une par
  pixel.
- L'aperçu de raccord assemble les cases selon la même table que le rendu — pas une seconde table.
- **Case survolée** : une position de canevas donnée produit l'index de case attendu, et aucun index
  hors de la planche ; fonction pure, réutilisant la conversion de la TACHE-03.

## Points d'attention
- **Une seule table de raccords.** Les libellés et l'aperçu doivent lire la table de LOT-42 ; une
  copie dans l'atelier divergerait au premier ajustement et induirait l'auteur en erreur — le pire
  résultat possible pour un outil censé le guider.
- Les repères des seize cases sont dessinés depuis les jetons (TACHE-03) et doivent rester lisibles
  sur une planche sombre comme sur une planche claire : leur contraste ne peut pas dépendre du
  contenu peint.
- L'aperçu live ne doit pas rendre l'application saccadée : invalider et recharger une texture 16×16
  est peu coûteux, mais le faire à chaque mouvement de souris ne l'est pas.
- Si l'asset édité n'est utilisé par aucun niveau ouvert, l'aperçu live n'a rien à montrer : le
  signaler plutôt que de laisser croire à un dysfonctionnement.

## Définition de fait (DoD)
- Le niveau reflète les modifications en cours d'édition, par geste et non par pixel ; l'édition
  d'une planche affiche les repères et libellés des seize cases, issus de la table unique, ainsi
  qu'un aperçu d'assemblage 3×3 ; la case survolée figure dans la barre d'état ; un asset animé se
  prévisualise ; chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art, avec aperçu dans le niveau) ; réutilise `EX-EDIT-025`
(raccords automatiques), `EX-EDIT-026` (rechargement à chaud), `EX-REN-005` (animations par
données), `EX-IHM-060` (état de travail affiché en permanence), `EX-REN-033` (traduction).
