# TACHE-04 — Déduplication des commandes et raccourcis d'éditeur {#lot-57-tache-04-deduplication-raccourcis}

**Lot :** [LOT-57](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Input`, `Source/HMI/Interface` · **Statut :** fait

## Contexte
Plusieurs états de l'éditeur sont pilotables depuis deux contrôles distincts : la couche d'un décor se
règle dans le panneau Outils **et** dans l'onglet Décors ; le mode d'affichage Physique/Texture bascule
par sa touche **et** par une case à cocher. Deux contrôles pour une valeur, c'est une divergence
possible et une question sans réponse pour l'utilisateur : lequel fait autorité.

La dette la plus nette est ailleurs. `hmi::EditorKeyBindings` définit **dix** actions d'éditeur
remappables, les charge au démarrage et les enregistre dans le fichier de configuration partagé avec les
raccourcis de jeu. Le viewport n'en consulte **qu'une seule** : celle de l'outil d'assignation de
texture. Les neuf autres — enregistrer, annuler, refaire, copier, coller, essayer, grille, aide,
renommer — sont écrites dans le fichier et **jamais lues** : les touches correspondantes sont
interceptées en dur dans le viewport, donc non remappables, et deux d'entre elles (aide et renommer) ne
sont implémentées nulle part.

Symétriquement, aucun écran n'expose ces raccourcis : l'écran de remappage ne présente que les six
actions de **jeu**. Les dix clés de traduction correspondantes existent pourtant dans les deux
catalogues et ne sont référencées par aucun code — le travail a été fait à moitié, puis oublié.

`EX-EDIT-015` exige enfin « un aperçu des raccourcis clavier » depuis le `LOT-15` : il n'existe pas, et
la touche qui devrait l'ouvrir est justement l'une des actions définies mais non branchées.

## Travail à réaliser
- **Un état, un contrôle** : suppression du sélecteur de couche de décor dupliqué, en conservant celui
  qui est le plus proche du geste d'édition. Suppression de la case doublant la bascule
  Physique/Texture, remplacée par une entrée unique du menu Affichage **affichant son raccourci**.
- **Commandes issues des actions du [LOT-56](@ref lot-56)** : chaque commande interceptée en dur dans le
  viewport devient une action portant son raccourci, de sorte que l'entrée de menu, le bouton de barre
  d'outils et la touche proviennent d'une définition unique.
- **Branchement des actions remappables** : les neuf actions définies et non lues deviennent effectives,
  ou sont **retirées explicitement** de la définition si elles ne correspondent à aucune commande
  voulue. Aucune action ne doit rester définie sans effet.
- **Annuler, refaire, copier et coller à cible contextuelle** : quatre de ces actions désigneront
  bientôt deux cibles distinctes, car l'atelier pixel art de [LOT-54](@ref lot-54) apporte un second
  historique et un second presse-papiers, indépendants de ceux de `core::LevelDraft`. C'est
  précisément la raison pour laquelle elles doivent être **une** action chacune, dont la cible suit le
  contexte d'édition actif — et non deux commandes homonymes arbitrées par le focus au dernier moment.
  Prévoir ici la notion de contexte d'édition courant évite d'avoir à défaire ce branchement en
  LOT-54.
- **Écran de remappage de l'éditeur** : un onglet « Éditeur » dans la page Options, sur le modèle exact
  de l'onglet de remappage du jeu, réutilisant les clés de traduction déjà présentes.
- **Aperçu des raccourcis** atteignable depuis l'application, listant les commandes et leurs touches
  **actuelles** — donc alimenté par les actions, pas par un texte figé.
- **Traduction** : libellés dans les deux catalogues ; aucune clé orpheline ne subsiste.

## Fichiers impactés
- `Source/HMI/Input/EditorKeyBindings.{h,cpp}` — actions effectivement branchées.
- `Source/HMI/Game/GameViewport.{h,cpp}` — interceptions en dur remplacées par les raccourcis des
  actions.
- `Source/HMI/Interface/EditorActions.{h,cpp}` (créé en `LOT-56`) — raccourcis remappables.
- `Source/HMI/Interface/EditorKeybindingsWidget.{h,cpp}` (nouveau) — onglet de remappage.
- `Source/HMI/Interface/OptionsPage.{h,cpp}`, `Source/HMI/Interface/MainWindow.{h,cpp}`.
- `Source/HMI/Editor/ToolPanel.{h,cpp}`, `Source/HMI/Editor/TexturePanel.{h,cpp}` et leurs fichiers
  d'interface — contrôles dupliqués retirés.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Input/test_editor_key_bindings.cpp` — complété.

## Tests (obligatoires)
- **Aucune action orpheline** : chaque action définie dans les raccourcis d'éditeur correspond à une
  commande effectivement déclenchable — un test qui échoue si une action est ajoutée sans être branchée.
  C'est le garde-fou contre la réapparition exacte du défaut corrigé.
- **Remappage effectif** : après remappage d'une action d'éditeur, la nouvelle touche déclenche la
  commande et l'ancienne ne la déclenche plus.
- **Persistance** : les raccourcis d'éditeur sont relus au démarrage suivant, et l'enregistrement ne
  détruit pas la section des raccourcis de jeu du fichier partagé.
- **Aucun raccourci en double** entre actions d'éditeur, et aucune collision avec les raccourcis de jeu
  utilisés en mode essai.
- **Aucune clé de traduction orpheline** : toutes les clés d'action d'éditeur sont référencées.

## Points d'attention
- **Un raccourci clavier n'est pas un doublon à supprimer.** L'objectif est qu'une commande ait une
  **définition** unique, pas un seul chemin d'accès. Enregistrer doit rester atteignable par le menu et
  par sa touche — mais depuis la même action.
- **La bascule Physique/Texture est traitée avant toute autre touche** dans le viewport, ce qui la rend
  active en édition, en essai et en jeu. Ce comportement est voulu et ne doit pas être perdu en la
  transformant en action de menu.
- **Le fichier de raccourcis est partagé** entre jeu, éditeur et manette, chaque classe possédant sa
  section. Toute écriture doit préserver les sections des autres — le mécanisme existe déjà, ne pas le
  contourner.
- Retirer une action plutôt que la brancher est un **choix légitime**, à condition d'être explicite :
  retirer aussi sa clé de traduction et le mentionner dans le lot, plutôt que de laisser une définition
  morte.
- L'aperçu des raccourcis doit lire les touches **effectives** : un aperçu figé redeviendrait faux dès
  le premier remappage.

## Définition de fait (DoD)
- Aucun état n'est pilotable depuis deux contrôles distincts ; chaque commande a une définition unique
  fournissant menu, bouton et raccourci ; toutes les actions d'éditeur définies sont effectives et
  remappables depuis les Options, ou retirées explicitement ; un aperçu des raccourcis effectifs est
  atteignable ; aucune clé de traduction orpheline ; `/W4 /WX` propre.

## Exigences
`EX-IHM-062` (une commande, un endroit) ; concrétise la partie « aperçu des raccourcis » d'`EX-EDIT-015`
(découvrabilité) ; réutilise `EX-IHM-055` (actions, `LOT-56`), `EX-CTRL-012` (raccourcis d'éditeur
remappables), `EX-EDIT-044` (visibilité des calques), `EX-REN-033` (traduction).
