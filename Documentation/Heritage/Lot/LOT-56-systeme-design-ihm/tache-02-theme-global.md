# TACHE-02 — Thème couvrant toute l'application {#lot-56-tache-02-theme-global}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/Elements/Themes`, `Source/HMI/Interface` · **Statut :** fait

## Contexte
`Source/Elements/Themes/theme.qss` fait une soixantaine de lignes et n'adresse que deux écrans, par
sélecteur d'`objectName` : le menu principal et la page Options. Le commentaire en tête du fichier
l'assume — l'éditeur « conserve le thème Qt par défaut ». Ce n'était pas un choix esthétique mais une
conséquence : sur le style natif, étendre la feuille de style ne donnait pas un résultat homogène.

La TACHE-01 ayant supprimé cette contrainte, la feuille de style peut enfin couvrir l'application
entière. Elle doit dans le même mouvement cesser d'être une **seconde source de vérité** : ses couleurs
sont aujourd'hui saisies littéralement, en parallèle de celles du code.

## Travail à réaliser
- **Feuille de style produite à partir des jetons** : le fichier devient un modèle où les couleurs et
  les espacements sont des marqueurs substitués au chargement par les valeurs des jetons (TACHE-01).
  Aucune couleur littérale ne subsiste dans le fichier.
- **Couverture de l'ensemble des classes de widgets employées par le projet** : fenêtre principale,
  panneaux dockables (barre de titre, bordure, onglets de regroupement), barre de menus et menus, barre
  d'état, onglets, arbres, tables et listes (en-têtes, lignes alternées, survol, sélection), boutons,
  boutons radio, cases à cocher, listes déroulantes, champs de saisie, compteurs numériques, barres de
  défilement, infobulles, séparateurs.
- **États de focus visibles** sur tout contrôle atteignable au clavier, en plus des états de survol et
  désactivé.
- **Revue des boîtes de dialogue standard** (message, fichier, saisie) et du dialogue de
  redimensionnement construit à la main dans *MainWindow*.
- **Séparation des deux portées dans la feuille de style**, conformément aux jetons de la TACHE-01 :
  les règles du menu principal et de la page Options sont substituées avec les jetons **invariants**,
  toutes les autres avec les jetons **variables**. La portée nominative par `objectName` déjà en place
  sur ces deux écrans devient donc le support de cette frontière, au lieu d'être le contournement
  qu'elle était — elle est **conservée pour cette raison**, et retirée partout où elle ne fait que
  compenser l'ancien style natif.
- **Régénération à la demande** : la feuille de style doit pouvoir être reproduite et réappliquée en
  cours d'exécution avec un autre jeu de valeurs variables, sans reconstruire les widgets. La
  TACHE-06 s'appuiera dessus ; l'écrire maintenant coûte une fonction, l'ajouter après coûte une
  réécriture.
- **Repli** conservé : en l'absence du fichier de thème, l'application démarre et reste utilisable, avec
  un avertissement journalisé — comportement actuel de `main.cpp`, à ne pas régresser.

## Fichiers impactés
- `Source/Elements/Themes/theme.qss` — modèle complet, sans couleur littérale.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` — substitution des marqueurs par les jetons.
- `Source/HMI/main.cpp` — chargement via le thème plutôt qu'en direct.
- `Source/Test/Unit/HMI/Interface/test_application_theme.cpp` (nouveau).

## Tests (obligatoires)
- **Substitution** : chaque marqueur présent dans le modèle est remplacé ; un modèle contenant un
  marqueur inconnu est signalé plutôt que produit silencieusement avec un trou. Fonction **pure**,
  testée sans instance d'application.
- **Aucune couleur littérale résiduelle** : la feuille de style produite ne contient plus de marqueur,
  et le modèle source ne contient aucune valeur de couleur écrite en dur — un test qui lit le fichier
  et échoue si une couleur littérale y réapparaît.
- **Étanchéité des portées** : produire la feuille de style avec deux jeux de valeurs variables
  différents donne deux résultats dont les **règles d'identité sont identiques au caractère près**.
  C'est le test qui garantit que le menu principal ne bougera jamais avec le thème de l'éditeur.
- **Repli** : thème absent ou illisible → l'application démarre, un avertissement est journalisé.

## Points d'attention
- **Le focus n'est pas un détail cosmétique ici.** La navigation à la manette dans les menus
  (`EX-IHM-040`) fonctionne en postant des événements de tabulation au widget focalisé : elle
  s'appuie entièrement sur le parcours de focus de Qt. Un focus invisible la rend **inutilisable**,
  alors même qu'elle fonctionne.
- **Ne pas dégrader la lisibilité des vignettes.** *PalettePanel* et *AssetThumbnailView* affichent le
  rendu réel des tuiles et des assets : le fond appliqué derrière une vignette change la perception de
  son contenu, en particulier pour les assets à zones transparentes. Vérifier sur une tuile claire et
  sur une tuile sombre.
- **Les lignes alternées et la couleur de sélection** doivent rester distinguables une fois le thème
  sombre appliqué : c'est le défaut le plus fréquent d'un thème sombre écrit rapidement.
- Une feuille de style trop spécifique redevient ingérable. Privilégier les sélecteurs par **classe de
  widget**, et réserver la portée nominative aux vraies exceptions.

## Définition de fait (DoD)
- L'ensemble de l'application partage une seule apparence, aucun contrôle ne conservant un rendu natif
  résiduel ; la feuille de style ne contient aucune couleur littérale, est produite depuis les jetons
  et sépare de façon étanche l'identité du jeu du châssis d'édition, ce qu'un test garantit ; elle peut
  être régénérée et réappliquée à chaud ; le focus est visible partout et la navigation à la manette
  reste utilisable de bout en bout ; le repli sans fichier de thème fonctionne ; `/W4 /WX` propre.

## Exigences
`EX-IHM-050` (thème couvrant toute l'IHM, deux portées, focus visible), `EX-IHM-051` (source unique) ;
prépare `EX-IHM-054` (thème clair/sombre de l'éditeur) ; réutilise `EX-IHM-040` (navigation à la
manette dans les menus), `EX-NFR-040` (repli en l'absence d'asset).
