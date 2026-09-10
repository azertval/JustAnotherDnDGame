# Concevoir les écrans dans Qt Design Studio {#guide-conception-qds}

> Statut : **en place** (`LOT-86`). Cette page ne s'adresse pas au développeur mais à **qui dessine
> les écrans**. Elle décrit ce qu'on peut faire sans jamais ouvrir un fichier source, ce qui demande
> encore un développeur, et pourquoi la frontière est là.

## En une phrase

Ouvrir `Source/Ui/JadgUi.qmlproject` dans Qt Design Studio, modifier, enregistrer, relancer le jeu :
le changement est à l'écran, sans qu'aucun compilateur n'ait tourné (`EX-IHM-100`).

## Ce que le projet vous montre, et ce qu'il vous cache

`JadgUi.qmlproject` ne décrit que `Source/Ui` et les assets. Ni CMake, ni `Source/HMI`, ni une ligne
de C++ n'y apparaissent — ce n'est pas une commodité d'affichage, c'est le **périmètre du fichier**.
La frontière entre les deux métiers n'est donc pas une consigne de relecture : c'est ce que le
projet vous laisse voir.

```
Source/Ui/
  Main.qml            la fenêtre — développeur
  Theme/Tokens.qml    LES JETONS : couleurs, polices, grandeurs. À vous.
  Controls/           les briques réutilisées d'un écran à l'autre. À vous.
  Screens/            un FORMULAIRE et son jumeau par écran (voir ci-dessous).
  Logic/              le câblage — développeur.
  Mocks/              les données d'exemple de la conception.
```

## La règle des deux fichiers

Chaque écran existe en **deux** exemplaires, et il faut savoir lequel est le vôtre :

| Fichier | À qui | Ce qu'il contient |
|---|---|---|
| `CharacterSheetForm.ui.qml` | **la conception** | tout ce qui se voit : disposition, couleurs, tailles, animations |
| `CharacterSheet.qml` | le développement | d'où viennent les données, et ce que font les touches |

Le suffixe `Form` n'est pas décoratif : sans lui, les deux fichiers déclareraient un type du même
nom et le module refuserait de se charger.

**Pourquoi cette séparation.** Un `.ui.qml` est le sous-ensemble **déclaratif** de QML — celui que
Design Studio sait relire *et réenregistrer* sans l'abîmer. Ce qu'il n'y comprend pas, il le
**perd**, sans avertir. La règle ne vise donc pas le style, mais ce que l'outil détruirait :
n'écrivez jamais de fonction, de bloc de code ni de gestionnaire impératif dans un `Form`. Si vous
avez besoin de logique, c'est le jumeau qu'il faut, et donc un développeur.

`scripts/check_ui_layers.py` le vérifie à chaque *Pull Request* (`EX-IHM-103`).

## Les jetons : le seul endroit où s'écrit une couleur

`Source/Ui/Theme/Tokens.qml` porte la palette, les polices et les grandeurs. **Aucune couleur, aucune
famille de police, aucune taille de texte ne s'écrit ailleurs** (`EX-IHM-105`, vérifié par le même
contrôle).

Ce n'est pas de la discipline pour la discipline. Une valeur écrite dans un écran survit à un
changement de palette : elle ne suit plus rien, et personne ne remarque qu'un seul écran a cessé de
ressembler aux autres. Le dépôt a déjà payé ce prix — la palette y a vécu **écrite deux fois**
pendant des mois.

Les couleurs y sont nommées par **rôle** (`accent`, `surface`, `frameEdge`), jamais par teinte. Un
jeton `accent` survit à un changement de couleur ; un jeton qui s'appellerait `or` deviendrait un
mensonge le jour où l'accent passe au bleu.

### Le facteur d'agrandissement

`Tokens.scale` est un **entier** borné à 1–3, et les grandeurs sont déjà multipliées par lui :
écrire `font.pixelSize: Tokens.body` suffit, ne multipliez jamais vous-même.

Entier, parce que les filets d'un pixel du cadre de parchemin se corrompent silencieusement à une
échelle fractionnaire. Design Studio affiche le facteur 2, celui d'une fenêtre 720p.

## Les données d'exemple

Chaque formulaire porte des valeurs d'exemple — c'est ce qui vous permet de juger une mise en page
au lieu de regarder un écran vide. Le jeu ne les voit jamais : à l'exécution, le jumeau les remplace.

**Elles doivent avoir exactement la forme des vraies données.** Les listes d'exemple sont des
`ListModel` et non des tableaux JavaScript, parce qu'un tableau n'expose que `modelData` là où un
modèle expose ses **rôles** — un écran validé sur des tableaux se serait affiché vide une fois
branché, sans la moindre erreur.

## Les écrans dessinés mais pas encore alimentés

Sept écrans du RPG et la page Options existent **sans données** : les lots qui les produiront ne sont
pas écrits. Leur pied l'avoue — « Écran dessiné, données à brancher » — et leurs champs affichent un
tiret cadratin plutôt que de fausses valeurs. Un écran rempli de valeurs plausibles se prend pour un
écran fini : il passe les relectures, on l'oublie, et un jour quelqu'un s'étonne que le marchand
vende toujours les mêmes trois objets.

Vous pouvez **les dessiner entièrement** dès maintenant : le jour où le lot fonctionnel arrive, seul
le jumeau change. `python scripts/list_pending_bindings.py` en donne l'inventaire.

## Les modules que vous pouvez importer

Uniquement ceux que connaissent **à la fois** Qt et Design Studio : `QtQuick`, `QtQuick.Controls`,
`QtQuick.Layouts`, `QtQuick.Shapes`, `QtQuick.Effects`, et `Jadg.Ui`.

Design Studio livre les siens (`QtQuick.Studio.*`), absents d'une installation Qt ordinaire : un
formulaire qui en importerait s'ouvrirait parfaitement chez vous et **casserait le jeu** — le pire
des deux mondes, découvert le plus tard possible. `EX-IHM-104` l'interdit, et le contrôle le vérifie.

## Les ornements se tracent, ils ne se collent pas

Cadres, cabochons, bandeaux et fleurons sont des `Shape` — des polygones, pas des images
(`EX-IHM-075`). Trois raisons, et elles tiennent toutes :

- **une image ne s'étire pas honnêtement** : un cabochon posé sur un panneau bas s'ovalise ;
- **une image fige ses couleurs hors des jetons**, et devrait être réexportée à chaque retouche ;
- **une image n'est nette qu'à un seul facteur** d'agrandissement ; un tracé l'est aux trois.

Leurs proportions sont **relevées** sur `Character_Sheets_Tanares.pdf`, jamais choisies à vue. Les
retoucher est votre droit ; savoir qu'elles viennent de là évite de croire qu'elles sont arbitraires.

## Les textes

Écrivez-les en **français, directement dans le fichier** : `qsTr("Nouvelle partie")`. Le français est
la langue source — c'est ce qui vous permet de juger une mise en page, une clé technique ne se lisant
pas.

Les traductions se maintiennent ensuite dans **Qt Linguist**, sur
`Source/Elements/Localization/jadg_en.ts`. Un outil de traducteur, pas de développeur.

**Une exception** : le vocabulaire des **règles** — noms de caractéristiques, d'emplacements
d'équipement. Ceux-là viennent d'un lexique (`rpg.glossary.csv`) qui garantit une seule traduction
par terme dans tout le jeu, et les écrire dans un écran casserait cette garantie.

## Ce qui demande encore un développeur

Aucune chaîne ne met la totalité d'une interface entre vos mains, et le prétendre ne servirait
personne :

| Vous, seul | Un développeur |
|---|---|
| disposition, tailles, marges, ancrages | exposer une **donnée** que le jeu ne calculait pas |
| couleurs, typographie, jetons | une **interaction** qui change l'état du jeu |
| ornements, images, icônes, effets | une **règle de navigation** |
| animations, transitions, états visuels | faire exister un **écran** |
| textes | — |

C'est la même frontière que dans les moteurs du commerce, et c'est la bonne : vous disposez
librement de tout ce que le jeu sait déjà dire ; il faut un développeur pour lui apprendre à dire
quelque chose de neuf.

## Voir aussi

- @ref guide-ihm-qt — le socle applicatif : deux binaires, module QML, surface de rendu.
- @ref guide-design-ihm — les jetons et la répartition de l'information.
- [Spécification IHM](@ref spec-interface-ihm), section 11 — le *pourquoi* de cette frontière
  (`EX-IHM-100` à `EX-IHM-105`).
