# TACHE-01 — Aucun écran ne contraint la fenêtre {#lot-73-tache-01-invariant-taille}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
`QStackedWidget::minimumSizeHint` vaut le **maximum sur toutes ses pages**, y compris celles qui ne
sont pas affichées. Un seul écran dense fixait donc la taille minimale de la fenêtre entière —
`OptionsPage` posant le plancher pendant qu'on regarde le menu principal.

Ce plancher n'était pas fixe : les grandeurs d'habillage des écrans du jeu sont multipliées par le
facteur entier de `hmi::pixelArtScale` (`EX-IHM-070`), lui-même dérivé de la **hauteur de la
fenêtre**. Le facteur grossit les polices et les rembourrages, qui grossissent la taille minimale,
qui grossit la fenêtre, qui relance le calcul un cran plus haut. Rien ne redescend jamais, une
fenêtre ne pouvant pas passer sous son propre minimum. Et rien ne bornait la boucle : une recherche
de `availableGeometry` ou `QScreen` dans `Source/` ne renvoyait **aucune occurrence**.

Trois aggravants s'y ajoutaient :
- `restoreGeometry` restituait telle quelle une géométrie débordante persistée à la session
  précédente : le défaut survivait au redémarrage ;
- l'appel à `applyIdentityScale()` placé en fin de construction était **du code mort** — sa garde
  `!isVisible()` est vraie tant que la fenêtre n'est pas montrée, si bien que l'application
  démarrait toujours au facteur 1 et corrigeait ensuite sous les yeux du joueur, exactement ce que
  le commentaire de l'appelant prétendait éviter ;
- la taille minimale imposée par les écrans était **journalisée** au démarrage, jamais confrontée à
  l'écran.

## Travail réalisé
- **`hmi::ScreenPageHost`** (`Source/HMI/Interface/ScreenPageHost.h/.cpp`) : `QScrollArea`
  redimensionnable, sans cadre, barres à la demande, et **politique de taille `Ignored`** sur les
  deux axes. Les deux mécanismes sont complémentaires : le défilement évite le rognage, la politique
  de taille garantit une contribution **nulle** au minimum de la fenêtre — une `QScrollArea` seule
  propage encore un plancher, et n'empêcherait pas un appelant de poser une taille minimale sur la
  page elle-même.
- **`MainWindow::addScreenPage`/`showScreenPage`** : toute page de la pile passe par l'enveloppe, et
  l'association page → enveloppe est retenue pour la sélection. Le **viewport en est exclu** :
  surface de rendu QRhi, il remplit sa page sans jamais défiler, et son minimum (320×240) tient sur
  tout écran.
- **`hmi::pixelArtScaleForDisplay(hauteurFenêtre, hauteurDisponible)`** : le facteur est le minimum
  des deux, jamais la seule zone disponible — une petite fenêtre sur un grand écran doit garder son
  facteur, sans quoi elle rendrait une maquette plus grande qu'elle. Zone inconnue (valeur nulle ou
  négative) : la hauteur de fenêtre décide seule.
- **`MainWindow::restoreLayout`** borne la géométrie restaurée à `availableGeometry` — taille
  d'abord, position ensuite : déplacer une fenêtre trop grande ne la ferait pas tenir.
- **`applyIdentityScale(bool beforeFirstShow)`** : la garde d'invisibilité est levée pour le chemin
  de construction, et la feuille d'identité y est posée même si le facteur n'a pas changé — la pile
  n'en porte encore aucune.
- **`MainWindow::warnIfScreensConstrainWindow`** : le journal du démarrage devient un garde-fou, la
  taille minimale étant désormais **comparée** à la zone disponible et un dépassement journalisé en
  avertissement.

## Vérification
- `test_pixel_art_scale.cpp`, quatre cas ajoutés : le facteur borné n'excède jamais celui de la zone
  disponible ; une fenêtre de 1160 à 3000 pixels sur un écran de 1009 utiles ne gagne aucun facteur
  (le cas rapporté) ; une zone inconnue laisse la fenêtre décider ; une petite fenêtre garde son
  facteur sur un grand écran.
- Vérification à l'IHM : plus aucune ligne `QWindowsWindow::setGeometry: Unable to set geometry`
  dans le journal, sur aucun écran ; contenu défilant plutôt que rogné sur une fenêtre réduite ;
  géométrie restaurée tenant dans l'écran après redémarrage.

## Limite connue
`UnitTests` ne lie que `Qt6::Gui`/`GuiPrivate`, **pas `Qt6::Widgets`** : aucun test ne peut
instancier un widget ni lire un `minimumSizeHint`. L'invariant se teste donc par ses **fonctions
pures** (le facteur borné) et se constate à l'IHM — même partage que le reste des lots de rendu.
