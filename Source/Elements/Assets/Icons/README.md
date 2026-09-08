# Assets/Icons/

Icônes **d'interface**, en SVG, embarquées dans le binaire par `Elements/UI/resources.qrc`.

Ce dossier n'est ni `Assets/UI/` ni `Assets/Entities/`, et la distinction est le sujet :

| Dossier | Ce qu'il porte | D'où ça vient |
|---|---|---|
| `Assets/UI/` | illustrations **extraites du corpus** | une page de PDF, avec son empreinte au manifeste (`check_ui_assets.py`) |
| `Assets/Entities/` | illustrations que la **donnée** attend, par clé | un catalogue de règle (`check_asset_keys.py`) |
| `Assets/Icons/` | icônes **écrites ici**, que le code nomme | ce dépôt, et rien d'autre |

Une icône n'a ni page d'origine à citer ni entrée de catalogue à servir. La ranger dans l'un des
deux autres dossiers demanderait de mentir à l'un des deux lints, ou de les affaiblir tous les
deux.

## Ce qui est généré

`slot/` — les huit emplacements d'équipement, produits par `scripts/generate_slot_icons.py`. **À ne
pas retoucher à la main** : la table du générateur est la seule liste, et une retouche locale se
perdrait à la prochaine exécution en laissant l'icône divergente des sept autres.

```bash
python scripts/generate_slot_icons.py
```

## Le trait est `currentColor`

Aucune icône ne porte de teinte. `EX-IHM-051` et le `LOT-76` l'interdisent : une couleur figée dans
un fichier devrait être réexportée à chaque retouche de palette, et le jour où quelqu'un oublie,
l'icône reste *juste* — mais fausse.

Le chargeur substitue donc `currentColor` par le rôle demandé de la portée identité au moment du
rendu. Une icône rendue sans substitution sort en **noir** : une panne qui se voit, plutôt qu'une
teinte plausible qui ment.

## Pourquoi du SVG ici, alors que le `LOT-76` trace ses ornements

Les deux règles répondent à deux questions différentes, et ne se contredisent pas.

Un **ornement de cadre s'étire** : un cabochon posé sur un panneau bas s'ovalise, un bandeau étiré
déforme ses ailes. Il doit être *redessiné* à chaque taille — c'est `hmi::ParchmentOrnaments`, et
c'est pourquoi il est du code et non un fichier.

Une **icône d'emplacement ne s'étire jamais** : elle occupe un carré, et un seul. Ce qui la
distingue des sept autres n'est pas sa taille mais son **sujet** — un heaume, une botte —, et un
sujet se dessine une fois. L'écrire en C++ ne gagnerait rien et coûterait la relecture.
