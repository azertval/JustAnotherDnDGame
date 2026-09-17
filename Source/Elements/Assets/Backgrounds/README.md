# Elements/Assets/Backgrounds/

Fonds de niveau : les PNG qu'un niveau peut désigner comme image de dernier plan (`LOT-44`,
`EX-REN-044`).

Déposer un fichier ici suffit à le rendre sélectionnable dans la section « Fond » du panneau
« Textures » de l'éditeur : la liste est peuplée par **balayage de ce dossier**, jamais par saisie
d'un chemin (même patron que `Skins/`, `LOT-42`).

Dimensions **libres** (`hmi::AssetFamily::Background`, `EX-REN-007`) : le fond est étiré sur les
bornes du niveau au rendu, avec son ratio d'aspect **préservé** — l'image est recadrée par le
centre sur la dimension excédentaire plutôt que déformée. Un fichier illisible ou introuvable
retombe sur le damier magenta, avec un avertissement journalisé nommant l'asset (`EX-NFR-040`).

## Contenu actuel : plus aucun fond de test

**Les six fonds de test sont partis au `LOT-09`**, avec le script qui les produisait
(`generate_test_backgrounds.py`) : ils servaient à vérifier `hmi::computeBackgroundFit` à l'œil
dans l'éditeur, et la première carte du jeu (le Colisée) est une scène **isométrique** qui ne
désigne aucun fond. Le calcul de recadrage, lui, reste couvert par ses tests
(`test_background_fit.cpp`), qui n'ont jamais eu besoin d'une image sur le disque.

Reste `kenney_grass.png`, un fond réel (voir `../CREDITS.md`).

Un artiste déposera ici les vrais fonds, sans toucher au code.

## Fond `kenney_grass.png` : premier fond réel, sous licence libre (`LOT-65`)

Contrairement aux trois fonds ci-dessus (schématiques, générés par script), `kenney_grass.png`
(1024×1024) est une véritable illustration, publiée par **Kenney** (www.kenney.nl) sous licence
**CC0 1.0 Universal** (domaine public), utilisée telle quelle (dimensions déjà libres, aucune
retouche) — voir `../CREDITS.md` pour le détail. Il n'est associé à aucun jeu de skins
(`Backgrounds/` n'est pas structuré en `jeux` comme `skins.json`) : un niveau le désigne
directement par son nom de fichier.
