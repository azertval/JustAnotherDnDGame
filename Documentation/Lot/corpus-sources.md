# Corpus source — analyse et programme d'exploitation {#corpus-sources}

`Documentation/SourceBook/` rassemble le **matériel de référence** ayant servi à construire le monde
et les règles : huit PDF, environ 1 200 pages, 280 Mo. Cette page dit ce qu'ils contiennent, ce
qu'on a le droit d'en tirer, ce que l'extraction a appris techniquement, et découpe le travail en
lots.

---

## 1. Le corpus n'est pas versionné

Le dépôt est **public**. `Documentation/SourceBook/` est exclu **en entier** par le `.gitignore` : il
contient des PDF commerciaux, dont deux scans de livres sous copyright plein, et un `git push` est
irréversible — le contenu reste dans l'historique et dans les clones même après suppression du
fichier.

Deux conséquences pratiques, à retenir avant d'écrire quoi que ce soit dans ce dossier :

- **Rien de ce qu'on y dépose ne sera versionné**, pas même un fichier texte. Le manifeste du
  `LOT-30` vit donc dans `scripts/sourcebook/corpus.toml`, avec l'outil qui le lit, et non à côté
  des PDF qu'il décrit.
- Le corpus texte intermédiaire produit par l'extraction n'est pas versionné non plus, et pour la
  même raison : c'est le même contenu sous une autre forme.

---

## 2. Inventaire du corpus

| Document | Nature | Pages PDF | Langue | Extraction |
|---|---|---|---|---|
| `Tanares_Sourcebook.pdf` | Univers de Tanares : monde, factions, panthéon, régions, organisations, histoire, bestiaire, objets magiques | 179 | EN | texte natif, **double page** |
| `Players_Guide_to_Tanares_Version_20231218.pdf` | Espèces, classes, sous-classes, historiques, dons, sorts, règles optionnelles | 165 | EN | texte natif, **double page** |
| `Manuel-Des-Monstres.pdf` | *Monster Manual* D&D 5 en français | 353 | FR | **scan OCR bruité** |
| `Manuel-Des-Joueurs.pdf` | *Player's Handbook* D&D 5 en français | 304 | FR | **scan OCR bruité** |
| `Basic-Rules-FR.pdf` | Règles de base D&D 5 en français (aidedd.org) | 137 | FR | texte natif **propre** |
| `Animaux.pdf` | Bêtes du SRD traduites (aidedd.org) | 32 | FR | texte natif **très propre** |
| `Glossaire.pdf` | Lexique de traduction anglais → français (aidedd.org) | 22 | EN/FR | texte natif, 2 colonnes |
| `Character_Sheets_Tanares.pdf` | Feuilles de personnage | 5 | EN | **aucun texte** (vectoriel) |

### Ce que chaque document apporte, chiffré

- **`Animaux.pdf`** — **94 blocs de statistiques** complets (CA, PV, vitesse, six caractéristiques,
  compétences, sens, langues, facteur de puissance, traits, actions), au format le plus régulier de
  tout le corpus. C'est le gisement le plus rentable : traduction française déjà faite, mise en page
  mécanique, licence ouverte.
- **`Basic-Rules-FR.pdf`** — le socle jouable complet et redistribuable : 4 races, 4 classes,
  6 historiques, et surtout **les tables d'équipement** (armes p. 48, armures p. 50, matériel
  d'aventurier p. 51, outils p. 55, montures et véhicules p. 56, marchandises p. 56, dépenses p. 57,
  babioles p. 59), les sorts, et l'appendice des états. C'est aussi la **référence terminologique
  française** du projet.
- **`Glossaire.pdf`** — environ **1 200 paires `anglais = français ; catégorie`**, catégorie
  comprise (`; classe`, `; sort`, `; état`, `; type de dégâts`, `; capacité (paladin)`…). C'est une
  table d'autorité de traduction directement exploitable, et la réponse à « comment dit-on *saving
  throw* partout pareil dans le jeu ».
- **`Tanares_Sourcebook.pdf`** — **10 régions**, **5 factions**, **7 lieux du plan pénombral**,
  **18 divinités**, **11 organisations**, une chronologie, une économie de minerais rares, puis
  **17 familles de créatures pour 82 blocs de statistiques** (ch. 9) et un chapitre d'objets
  magiques (ch. 10). C'est le modèle le plus complet de ce à quoi ressemble un monde de jeu
  *structuré* : chaque région y a ses lieux, ses factions présentes, ses PNJ notables.
- **`Players_Guide_to_Tanares.pdf`** — 13 espèces, 4 classes inédites, 4 classes simplifiées, une
  vingtaine de sous-classes (**31 tables de progression** détectées), 7 historiques. Sa valeur est
  la **structure** : comment se décrit une espèce, comment se tabule une progression de classe.
- **`Manuel-Des-Joueurs.pdf` / `Manuel-Des-Monstres.pdf`** — les deux livres officiels français,
  dont **416 blocs de statistiques** pour le second. Inexploitables : voir §3 et §4.
- **`Character_Sheets_Tanares.pdf`** — 5 feuilles, **zéro caractère de texte** hors filigrane
  d'achat. Ce n'est pas une source de données : c'est une **maquette d'interface**, et elle vaut
  pour ça (quels champs, groupés comment, dans quel ordre de lecture).

---

## 3. Statut juridique — trois niveaux, à ne pas mélanger

Le dépôt est sous **GPL-3.0-or-later**, et `THIRD-PARTY-NOTICES.md` distingue déjà soigneusement la
licence du *code* de celle des *ressources*. Le corpus se range en trois niveaux, et cette
distinction commande tout le programme de lots.

### Niveau vert — redistribuable

`Basic-Rules-FR.pdf`, `Animaux.pdf`, `Glossaire.pdf`. Publiés librement par aidedd.org ; les deux
premiers portent l'**OGL 1.0a** en toutes lettres et déclarent leur contenu *Open Game Content*
dérivé du *System Reference Document 5.1*.

Utilisables comme données livrées avec le jeu, **à condition** de reproduire la notice OGL et
l'attribution SRD. C'est un ajout à `THIRD-PARTY-NOTICES.md`, dans la lignée de ce qui y est déjà
fait pour Qt, le CC0 et la SIL OFL — l'OGL 1.0a n'est pas la GPL, et ces données vivent donc sous
leur propre notice, comme les `.png` et les polices.

### Niveau orange — mécanique ouverte, univers fermé

`Tanares_Sourcebook.pdf`, `Players_Guide_to_Tanares.pdf`. Achetés légitimement (le filigrane
d'exemplaire en atteste). Leur page de crédits est explicite : le contenu issu du SRD 5.1 est *Open
Game Content*, **tout le reste est réservé**, et sont déclarés *Product Identity* les noms
« Tanares », « Penumbral Plane », « madwalker », « taii'maku », « Golgöggoth », « Isendden »,
« emogum ».

Conséquence pratique : on peut s'inspirer de la **structure** (comment un monde se découpe en
régions, comment une fiche d'espèce s'organise, comment une progression de classe se tabule) ; on ne
peut ni recopier les textes, ni réutiliser les noms propres, ni extraire les illustrations.

### Niveau rouge — référence privée uniquement

`Manuel-Des-Joueurs.pdf`, `Manuel-Des-Monstres.pdf`. Scans des éditions françaises officielles
(Wizards of the Coast / Black Book Éditions), sous copyright plein, sans clause ouverte. **Aucun
octet issu de ces deux fichiers ne doit entrer dans le dépôt**, ni comme donnée, ni comme texte
extrait, ni comme illustration.

### Ce que cela ne remet pas en cause

Rien, en fait — et c'est le point rassurant. La décision de cadrage prise avant le
[LOT-01](@ref lot-01) est déjà la bonne : **règles d20 maison, compatibles SRD dans leur structure,
sans en dépendre**. Le corpus n'est donc pas une source à porter, c'est une source à *lire*. Le
niveau vert fournit une amorce livrable ; le niveau orange fournit des patrons de conception ; le
niveau rouge fournit de la culture générale au concepteur, et rien d'autre.

---

## 4. Ce que l'extraction a appris

Cinq constats techniques, tous vérifiés sur le corpus, qui déterminent la faisabilité des lots.

**Les livres Tanares sont paginés en double page.** 179 pages PDF pour un livre qui numérote jusqu'à
358 ; une page PDF porte deux pages du livre côte à côte. Toute table de correspondance « page du
sommaire → page PDF » doit intégrer ce facteur 2, sinon on cible systématiquement le mauvais
chapitre. C'est la première chose que le manifeste du `LOT-30` doit enregistrer.

**`pdftotext -layout` mélange les colonnes des tableaux.** Sur la table des armes des *Basic Rules*,
il produit des lignes où le poids et le prix appartiennent à l'arme d'en dessous — une donnée
fausse, et fausse **silencieusement**, ce qui est pire qu'une extraction qui échoue. Le `pdftotext`
installé sur le poste est celui de **Xpdf 4.00**, qui offre un mode `-table` dédié : il restitue
correctement `Bâton | 1d6 contondant | 2 kg | 2 pa | Polyvalente (1d8)`. Le pipeline doit donc
choisir son mode par nature de contenu : `-table` pour les tableaux, `-layout` pour les blocs de
statistiques et la prose. Ce n'est pas un détail d'implémentation, c'est la condition pour que le
catalogue d'équipement ne soit pas faux.

**Les blocs de statistiques d'`Animaux.pdf` suivent un gabarit strict.** Toujours le même ordre,
toujours les mêmes libellés :

```
AIGLE
Bête de taille P, sans alignement
Classe d'armure 12
Points de vie 3 (1d6)
Vitesse 3 m, vol 18 m
   FOR DEX CON INT SAG CHA
  6 (-2) 15 (+2) 10 (+0) 2 (-4) 14 (+2) 7 (-2)
Compétences Perception +4
Sens Perception passive 14
Langues -
Puissance 0 (10 PX)
Vue aiguisée. …
ACTIONS
Serres. Attaque au corps à corps avec une arme : +4 au toucher, allonge 1,50 m, une cible.
Touché : 4 (1d4 + 2) dégâts tranchants.
```

Un automate suffit. Les 94 entrées sont accessibles sans jugement humain, hors relecture de
contrôle.

**Les deux manuels français sont des OCR bruités.** Leurs pages de crédits donnent « Cordeil » pour
« Cordell », « )on Schindehette » pour « Jon Schindehette », « Chefde projet », « W'àyne Reynolds ».
Sur un nom d'auteur c'est amusant ; sur un jet de dégâts, c'est un `1d8` devenu `ld8` qu'aucun test
ne rattrape. Ces fichiers cumulent donc l'obstacle juridique et l'obstacle qualité : la question de
les exploiter ne se pose même pas.

**`Character_Sheets_Tanares.pdf` ne contient aucun texte** — uniquement du tracé vectoriel. Pour s'en
servir comme maquette il faut le **rendre en images**, ce qui demande `pdftoppm` ou `pdftocairo`
(poppler), **absents du poste** : le `pdftotext` disponible vient d'Xpdf, qui ne les fournit pas.
C'est un prérequis d'outillage à poser explicitement, pas une surprise à découvrir en cours de lot.

---

## 5. Programme de lots proposé — phase F, contenu du monde

Les lots [LOT-13](@ref lot-13) (fiche de personnage), [LOT-14](@ref lot-14) (inventaire),
[LOT-25](@ref lot-25) (sorts) et [LOT-26](@ref lot-26) (économie) exigent tous un « catalogue en
**JSON** », conformément à [`EX-VIS-007`](@ref EX-VIS-007). Aucun ne dit **d'où sortent ces JSON**.
C'est exactement le trou que ce corpus comble, et c'est le périmètre de la phase F.

Neuf lots, `LOT-30` à `LOT-38`. Les numéros sont, comme toujours, des identifiants stables : ils
viennent après [LOT-29](@ref lot-29) dans la numérotation, mais plusieurs s'exécutent **avant** les
lots qui les consomment (voir §6). Une nouvelle famille d'exigences `EX-CNT-*` les couvre, à écrire
dans un `Documentation/Specification/contenu.md`.

### `LOT-30` — Chaîne d'extraction du corpus

*Aucun prérequis. Prérequis de tous les autres.*

L'outillage, et rien d'autre. `scripts/sourcebook/` : extraction (choix `-table` / `-layout` par
nature de contenu), découpe en colonnes, gestion de la pagination en double page, cache disque. Un
manifeste `scripts/sourcebook/corpus.toml` enregistre par document son empreinte SHA-256, son nombre
de pages, son décalage de pagination et **son niveau de licence** — vert, orange ou rouge. Le
pipeline **refuse de traiter un document rouge**, ce qui transforme la règle du §3 en garde-fou
exécutable plutôt qu'en bonne intention.

Le manifeste est versionné (c'est lui qui porte la règle) ; le corpus intermédiaire produit ne l'est
pas, et la CI ne l'exécute pas — les PDF ne sont pas sur le runner. Seules les **données produites**
sont validées en CI (`LOT-32`).

*Acceptation* — deux exécutions successives produisent des empreintes identiques ; un document
marqué rouge dans le manifeste fait échouer l'extraction avec un message explicite.

### `LOT-31` — Lexique bilingue et autorité de traduction

*Prérequis : `LOT-30`.*

Les ~1 200 paires du `Glossaire.pdf` deviennent `Source/Elements/Localization/rpg.glossary.csv`
(`anglais;français;catégorie`), complétées par le vocabulaire des *Basic Rules* (états, écoles de
magie, types de dégâts, propriétés d'armes).

L'intérêt n'est pas de traduire l'interface — `fr.lang` et `en.lang` font déjà cela. Il est
d'imposer **une seule traduction par terme de règle dans tout le jeu**. Sans table d'autorité,
*saving throw* devient « jet de sauvegarde » dans la fiche, « JdS » dans le journal de combat et
« sauvegarde » dans l'infobulle, et le joueur croit à trois mécaniques différentes.

*Acceptation* — `scripts/check_glossary.py` échoue si une clé de règle de `fr.lang` emploie un terme
absent du lexique ou en contredit la traduction.

### `LOT-32` — Schémas de données RPG

*Prérequis : `LOT-30`. Prérequis de `LOT-33` à `LOT-37`.*

Le contrat avant les données. Un JSON Schema par famille — créature, objet, arme, armure, sort,
espèce, classe, historique, état, type de dégâts — sous `Source/Elements/Rpg/schema/`, et
`scripts/check_rpg_data.py` qui valide tout `Source/Elements/Rpg/**.json` en CI.

Un test vérifie en plus que les énumérations C++ (`DamageType`, `Condition`…) et celles des schémas
**coïncident**. C'est le point où une donnée et un moteur divergent en silence : le JSON déclare
`"psychique"`, le C++ ne connaît que `Psychic`, la valeur tombe dans le cas par défaut, et le sort
ne fait plus de dégâts sans que rien ne l'annonce.

*Acceptation* — une donnée invalide fait échouer la CI avec le fichier et la ligne fautifs ; ajouter
une valeur d'énumération d'un seul côté fait échouer un test.

### `LOT-33` — Bestiaire de base

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13), [LOT-21](@ref lot-21),
[LOT-23](@ref lot-23).*

Les 94 bêtes d'`Animaux.pdf` vers `Source/Elements/Rpg/creatures/*.json`. Gabarit régulier, parseur
dédié, attribution OGL ajoutée à `THIRD-PARTY-NOTICES.md`.

C'est le premier jeu de données réel du projet, et il arrive avec un bénéfice de calendrier : loup,
ours, araignée géante et sanglier suffisent à peupler le donjon du [LOT-27](@ref lot-27) sans
attendre le moindre monstre original.

Le bestiaire étendu — les 82 blocs de Tanares, les 416 du *Monster Manual* — n'entre **pas** dans ce
lot : niveaux orange et rouge. Les créatures propres au monde du projet seront des créations
originales, structurées comme celles-ci.

*Acceptation* — les 94 profils chargent ; un test rejoue dix d'entre eux (CA, PV, vitesse, facteur
de puissance, dégâts de l'attaque principale) contre des valeurs recopiées à la main du PDF.

### `LOT-34` — Équipement, monnaie, marchandises

*Prérequis : `LOT-32`. Alimente [LOT-14](@ref lot-14), [LOT-26](@ref lot-26).*

Les tables d'équipement des *Basic Rules* vers `Source/Elements/Rpg/items/` : armes (dégâts, poids,
prix, propriétés), armures (CA, exigence de Force, discrétion), matériel, outils, montures,
marchandises, services.

**Extraction en mode `-table` obligatoire**, plus une relecture humaine des tables : c'est le lot où
le §4 se paie. Une valeur de prix décalée d'une ligne ne casse rien, ne lève aucune alerte, et
déséquilibre l'économie du [LOT-26](@ref lot-26) sans que personne ne comprenne pourquoi.

*Acceptation* — la CA calculée par le moteur pour chacune des 13 armures du catalogue égale la
colonne CA de la table, vérifié par un test ; le poids total d'un inventaire de départ correspond au
calcul manuel.

### `LOT-35` — Sorts et états

*Prérequis : `LOT-32`. Alimente [LOT-25](@ref lot-25).*

Les sorts et l'appendice des états des *Basic Rules* vers `spells/` et `conditions/`.

La règle qui compte est celle que le [LOT-25](@ref lot-25) énonce déjà : le C++ ne porte que des
*mécanismes* (dégâts de zone, jet de sauvegarde, condition appliquée, durée), la donnée les compose.
Un sort dont l'effet n'entre pas dans ces mécanismes se déclare **explicitement**
`"effet": "narratif"` — un sort tu, qu'on croit implémenté et qui ne fait rien, coûte bien plus cher
à diagnostiquer qu'un sort déclaré non joué.

*Acceptation* — chaque sort porte école, niveau, portée, durée, composantes, effet ; aucun sort ne
tombe silencieusement dans un cas par défaut, un test énumère le catalogue et l'exige.

### `LOT-36` — Espèces, classes, historiques

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13).*

Les 4 races, 4 classes et 6 historiques des *Basic Rules* comme socle jouable et redistribuable. Le
*Player's Guide* sert de **modèle de structure** — comment une fiche d'espèce s'organise, comment
une progression se tabule sur 20 niveaux, comment une sous-classe se greffe — sans reprise de texte
ni de nom.

*Acceptation* — celle du [LOT-13](@ref lot-13), tenue par les données : trois classes chargent et
donnent les bons modificateurs, et la progression du niveau 1 au niveau 5 ne fait intervenir aucune
valeur codée en C++.

### `LOT-37` — Atlas du monde et graphe de cartes

*Prérequis : `LOT-32`. Alimente [LOT-09](@ref lot-09), [LOT-27](@ref lot-27).*

Le `Tanares_Sourcebook` ne fournit pas ici des données mais un **patron de monde jouable** : une
région porte des lieux, un lieu porte des factions présentes et des PNJ notables, une faction porte
des objectifs qui se traduisent en quêtes, un panthéon porte des domaines qui se traduisent en
capacités. Dix régions, cinq factions, dix-huit divinités, onze organisations : c'est la
démonstration de ce qu'il faut renseigner pour qu'un monde tienne debout.

Le lot produit `Source/Elements/World/` — régions, lieux, factions, panthéon — **avec le vocabulaire
propre du projet**. Ce n'est pas un portage de Tanares, et le §3 en fait une contrainte, pas une
préférence : les noms propres du Sourcebook sont *Product Identity*.

Le branchement est direct sur le [LOT-09](@ref lot-09) : une région est un nœud du graphe de cartes,
un lieu est une carte à créer.

*Acceptation* — `check_world_graph.py` (prévu au [LOT-27](@ref lot-27)) valide l'atlas ; aucun lieu
sans région, aucune faction référencée qui n'existe pas, aucune région inatteignable.

### `LOT-38` — Fiche de personnage : maquette et interface

*Prérequis : [LOT-13](@ref lot-13). Alimente [LOT-24](@ref lot-24).*

`Character_Sheets_Tanares.pdf` rendu en images (prérequis d'outillage : poppler, cf. §4) devient la
maquette de référence de l'écran de fiche : quels champs, groupés en quels blocs — identité,
caractéristiques, compétences, combat, équipement, sorts — et dans quel ordre de lecture. La
maquette est une source de **disposition**, pas de contenu : rien n'en est copié, ni graphisme ni
libellé.

L'écran se construit sur le système de design existant.

*Acceptation* — chaque champ de la maquette est soit affiché à l'écran, soit inscrit dans une liste
explicite de champs hors périmètre. Un champ simplement oublié n'est pas un arbitrage.

---

## 6. Ordre d'exécution recommandé

Les numéros suivent [LOT-29](@ref lot-29), l'exécution non. Les lots de données doivent précéder les
lots qui les consomment, sans quoi ces derniers se construisent sur des catalogues fictifs — et un
catalogue fictif finit toujours par se figer en valeurs codées en dur, exactement ce que
[`EX-VIS-007`](@ref EX-VIS-007) interdit.

| Quand | Lots | Pourquoi là |
|---|---|---|
| Tout de suite après [LOT-08](@ref lot-08) | `LOT-30`, `LOT-31`, `LOT-32` | Outillage et contrats, sans dépendance ; le plus tôt est le mieux |
| Avec [LOT-09](@ref lot-09) | `LOT-37` | L'atlas donne au graphe de cartes de vrais nœuds à relier |
| Avant [LOT-13](@ref lot-13) | `LOT-36` | La fiche a besoin de vraies classes pour valider sa progression |
| Avant [LOT-14](@ref lot-14) | `LOT-34` | L'inventaire a besoin d'un vrai catalogue pour prouver l'absence de dérive de CA |
| Avant [LOT-21](@ref lot-21) / [LOT-23](@ref lot-23) | `LOT-33` | Attaques et IA ont besoin de vraies créatures |
| Avant [LOT-25](@ref lot-25) | `LOT-35` | Les sorts sont des données avant d'être un système |
| Après [LOT-13](@ref lot-13) | `LOT-38` | La maquette suppose la fiche existante |

---

## 7. Arborescence cible

```
Documentation/SourceBook/   ← les PDF, exclus en entier du dépôt

scripts/sourcebook/
  corpus.toml               ← manifeste : empreintes, pagination, niveau de licence (LOT-30)
  *.py                      ← chaîne d'extraction (LOT-30)
scripts/check_rpg_data.py, check_glossary.py

Source/Elements/Rpg/
  schema/*.schema.json      ← contrats (LOT-32)
  creatures/*.json          ← 94 bêtes (LOT-33)
  items/*.json              ← équipement, monnaie (LOT-34)
  spells/, conditions/      ← sorts et états (LOT-35)
  species/, classes/, backgrounds/   ← création de personnage (LOT-36)

Source/Elements/World/      ← régions, lieux, factions, panthéon (LOT-37)
Source/Elements/Localization/rpg.glossary.csv   ← lexique (LOT-31)
```

---

## 8. Ce qui reste à trancher

- **Nom du monde et vocabulaire propre.** Le `LOT-37` ne peut pas commencer sans lui : c'est ce qui
  distingue « inspiré de Tanares » de « portage de Tanares ».
- **Périmètre du socle jouable.** Les 4 classes des *Basic Rules* suffisent-elles au *vertical
  slice*, ou faut-il en concevoir dès le `LOT-36` ?
- **Nouvelle famille d'exigences.** `EX-CNT-*` dans un `Documentation/Specification/contenu.md`
  reste à écrire ; les lots ci-dessus la référencent par anticipation, comme le
  [LOT-13](@ref lot-13) référence déjà `EX-DND-*` qui n'existe pas encore.
