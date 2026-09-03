# TACHE-01 — Barre d'état : état permanent et aide contextuelle {#lot-57-tache-01-barre-etat}

**Lot :** [LOT-57](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/HMI/Editor` · **Statut :** fait

## Contexte
La barre d'état de l'éditeur affiche une chaîne unique, `status.edit_help`, qui concatène sept
raccourcis. Elle est identique quel que soit l'outil actif, et posée une seule fois à l'entrée en mode
éditeur. Tout message transitoire — texture assignée, décor posé, zone copiée, niveau enregistré — la
remplace pour cinq secondes, après quoi **la barre reste vide** : rien ne la restaure, hors changement
de langue.

Pendant ce temps, l'information réellement utile existe déjà dans le code et n'est affichée nulle part :
le viewport connaît son indicateur de modifications non enregistrées, la case survolée et son facteur de
zoom ; la fenêtre connaît le niveau ouvert et l'outil actif. Il n'y a rien à calculer, seulement à
montrer.

Le `LOT-52` a établi le patron applicable ici : `hmi::gameHudLines` décide **quoi** afficher dans une
fonction pure, testée sans GPU, et le rendu se contente de la suivre. La même séparation vaut pour la
barre d'état, à ceci près que le rendu est un widget Qt et non la scène.

## Travail à réaliser
- **Zones permanentes** dans la barre d'état : niveau ouvert, indicateur de **modifications non
  enregistrées**, outil actif, case survolée (colonne, ligne), facteur de zoom. Une zone vide quand
  l'information n'a pas de sens — curseur hors de la grille, aucun niveau ouvert — plutôt qu'un
  libellé de remplacement.
- **Zone de message transitoire distincte** des zones permanentes : un message ne recouvre plus l'état
  et ne peut donc plus l'effacer.
- **Aide contextuelle à l'outil actif** : la ligne d'aide décrit les gestes de l'outil courant, et
  **revient** après l'expiration d'un message.
- **Décision d'affichage en fonction pure** : à partir de l'état d'édition (niveau, indicateur de
  modification, outil, case survolée, zoom), produire la liste des libellés à afficher, sans dépendance
  à Qt ni au GPU.
- **Traduction** : tous les libellés, y compris les aides par outil, dans les deux catalogues ; retrait
  de la chaîne fourre-tout devenue inutile.
- **Raccordement aux signaux existants** de changement d'outil, de survol, de zoom et de modification,
  plutôt qu'un rafraîchissement périodique.
- **Ouvert à un second contexte d'édition** : l'atelier pixel art de [LOT-54](@ref lot-54) affichera
  dans la même barre l'asset ouvert, son état modifié, l'outil de canevas actif, le pixel survolé, le
  zoom et la couleur courante. C'est la même barre, la même règle de restauration après message et la
  même exigence de pureté ; la décision doit donc accepter un contexte d'édition d'asset à côté du
  contexte niveau, plutôt que de figer sa signature sur ce dernier et d'obliger LOT-54 à créer un
  second modèle.

## Fichiers impactés
- `Source/HMI/Editor/EditorStatus.{h,cpp}` (nouveau) — décision pure du contenu.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — zones de la barre d'état, raccordement.
- `Source/HMI/Game/GameViewport.{h,cpp}` — exposition de la case survolée, du zoom et de l'indicateur
  de modification.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Editor/test_editor_status.cpp` (nouveau).

## Tests (obligatoires)
- **Contenu par outil** : chaque outil produit son aide propre ; changer d'outil change l'aide.
- **Champs vides** : curseur hors grille → aucune coordonnée affichée ; aucun niveau ouvert → aucun nom
  ni indicateur de modification.
- **Indicateur de modification** : présent après une édition, absent après un enregistrement.
- **Restauration après message** : l'aide affichée après expiration d'un message transitoire est
  identique à celle d'avant, pour le même outil — le test qui décrit le défaut corrigé.
- Chaque clé de traduction utilisée existe dans les deux catalogues.
- Décision **pure**, testée sans Qt ni GPU.

## Points d'attention
- **Ne pas afficher l'état dans la scène rendue.** Le `LOT-52` a livré un moteur de texte sur le calque
  d'interface du pipeline Direct3D 11, mais il n'est branché que sur la session de jeu ; l'éditeur passe
  par son propre rendu de brouillon. L'état d'édition s'affiche dans des widgets Qt — deux chemins
  distincts, à ne pas confondre.
- **Une barre d'état qui saute est illisible.** Les zones permanentes doivent avoir une largeur stable :
  un compteur qui redimensionne sa zone à chaque déplacement de souris déplace tout ce qui suit.
- Le survol change à chaque déplacement de souris : ne recalculer et ne réécrire que si la case a
  réellement changé, sous peine d'un travail continu pour rien.
- Afficher les coordonnées dans le même ordre que le reste de l'éditeur — le panneau des liens présente
  déjà des positions, la convention doit être identique.
- Vérifier que le passage en mode jeu et le retour au menu n'exposent pas une barre d'état d'édition
  résiduelle.

## Définition de fait (DoD)
- Le niveau ouvert, les modifications non enregistrées, l'outil actif, la case survolée et le zoom sont
  visibles en permanence en édition ; l'aide dépend de l'outil et revient après tout message
  transitoire ; la décision de contenu est pure et testée sans Qt ; libellés dans les deux langues ;
  `/W4 /WX` propre.

## Exigences
`EX-IHM-060` (état affiché en permanence, aide contextuelle) ; réutilise `EX-EDIT-013` (déplacement et
zoom), `EX-EDIT-012` (modifications non enregistrées), `EX-REN-033` (traduction).
