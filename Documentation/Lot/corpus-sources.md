# Corpus source — analyse et programme d'exploitation {#corpus-sources}

`Documentation/SourceBook/` rassemble le **matériel de référence** ayant servi à construire le monde
et les règles : huit PDF, environ 1 200 pages, 280 Mo. Cette page dit ce qu'ils contiennent, ce que
l'extraction a appris techniquement, et découpe le travail en lots.

---

## 1. Le corpus n'est pas versionné

`Documentation/SourceBook/` est exclu **en entier** par le `.gitignore`. La raison est désormais la
taille : 280 Mo de binaires, dont un fichier de 83 Mo, que git versionne mal — chaque clone les
traîne, et aucune de leurs révisions ne se compresse.

Deux conséquences pratiques, à retenir avant d'écrire quoi que ce soit dans ce dossier :

- **Rien de ce qu'on y dépose ne sera versionné**, pas même un fichier texte. Le manifeste du
  `LOT-30` vit donc dans `scripts/sourcebook/corpus.toml`, avec l'outil qui le lit, et non à côté
  des PDF qu'il décrit.
- Le corpus texte intermédiaire produit par l'extraction n'est pas versionné non plus. Il se
  régénère à la demande depuis les PDF locaux ; ce sont les **données finales**, elles, qui entrent
  dans le dépôt.

---

## 2. Inventaire du corpus

| Document | Nature | Pages PDF | Langue | Texte | Images |
|---|---|---|---|---|---|
| `Tanares_Sourcebook.pdf` | Univers de Tanares : monde, factions, panthéon, régions, organisations, histoire, bestiaire, objets magiques | 179 | EN | natif, **double page** | **2 029** (1 806 ≥ 512²) |
| `Players_Guide_to_Tanares_Version_20231218.pdf` | Espèces, classes, sous-classes, historiques, dons, sorts, règles optionnelles | 165 | EN | natif, **double page** | **1 710** (1 476 ≥ 512²) |
| `Manuel-Des-Monstres.pdf` | *Monster Manual* D&D 5 en français | 354 | FR | **scan OCR bruité** | 1 424 (633 ≥ 512²) |
| `Manuel-Des-Joueurs.pdf` | *Player's Handbook* D&D 5 en français | 320 | FR | **scan OCR bruité** | 631 (453 ≥ 512²) |
| `Basic-Rules-FR.pdf` | Règles de base D&D 5 en français (aidedd.org) | 137 | FR | natif **propre** | 175, surtout des fonds de page |
| `Animaux.pdf` | Bêtes du SRD traduites (aidedd.org) | 32 | FR | natif **très propre** | 38, uniquement des fonds |
| `Glossaire.pdf` | Lexique de traduction anglais → français (aidedd.org) | 22 | EN/FR | natif, 2 colonnes | 5 pictogrammes |
| `Character_Sheets_Tanares.pdf` | Feuilles de personnage | 5 | EN | **aucun** (vectoriel) | 6, jusqu'à 2 668 × 3 418 |

### Ce que chaque document apporte, chiffré

- **`Animaux.pdf`** — **94 blocs de statistiques** complets (CA, PV, vitesse, six caractéristiques,
  compétences, sens, langues, facteur de puissance, traits, actions), au format le plus régulier de
  tout le corpus. C'est le gisement le plus rentable : traduction française déjà faite, mise en page
  mécanique. Aucune illustration de créature à en tirer en revanche — ses 38 images sont les fonds
  de parchemin des pages.
- **`Basic-Rules-FR.pdf`** — le socle jouable complet : 4 races, 4 classes, 6 historiques, et
  surtout **les tables d'équipement** (armes p. 48, armures p. 50, matériel d'aventurier p. 51,
  outils p. 55, montures et véhicules p. 56, marchandises p. 56, dépenses p. 57, babioles p. 59),
  les sorts, et l'appendice des états. C'est aussi la **référence terminologique française** du
  projet.
- **`Glossaire.pdf`** — environ **1 200 paires `anglais = français ; catégorie`**, catégorie
  comprise (`; classe`, `; sort`, `; état`, `; type de dégâts`, `; capacité (paladin)`…). C'est une
  table d'autorité de traduction directement exploitable, et la réponse à « comment dit-on *saving
  throw* partout pareil dans le jeu ».
- **`Tanares_Sourcebook.pdf`** — **10 régions**, **5 factions**, **7 lieux du plan pénombral**,
  **18 divinités**, **11 organisations**, une chronologie, une économie de minerais rares, puis
  **17 familles de créatures pour 82 blocs de statistiques** (ch. 9) et un chapitre d'objets
  magiques (ch. 10). Chaque région y a ses lieux, ses factions présentes, ses PNJ notables : c'est
  un monde de jeu prêt à l'emploi, pas seulement un exemple de structure. Et **2 029 images**, dont
  les cartes de régions.
- **`Players_Guide_to_Tanares.pdf`** — 13 espèces, 4 classes inédites, 4 classes simplifiées, une
  vingtaine de sous-classes (**31 tables de progression** détectées), 7 historiques, plus **1 710
  images** — portraits d'espèces, illustrations de classes, objets.
- **`Manuel-Des-Monstres.pdf`** — **416 blocs de statistiques** en français. Le plus gros bestiaire
  du corpus, mais le plus coûteux à exploiter : voir §4.
- **`Manuel-Des-Joueurs.pdf`** — le manuel complet en français ; utile surtout comme référence de
  formulation et de vocabulaire, redondant avec les *Basic Rules* sur les données.
- **`Character_Sheets_Tanares.pdf`** — 5 feuilles, **zéro caractère de texte** hors filigrane. Les
  planches sont en revanche disponibles en image à 2 668 × 3 418, soit environ 300 ppp : de quoi
  servir de maquette précise pour l'écran de fiche.

---

## 3. Licences : contrainte en sommeil

Le projet est **privé, sans diffusion ni acte commercial**, et le dépôt est passé en privé. Les
licences du corpus ne contraignent donc **pas** l'usage : noms propres, textes, blocs de
statistiques et illustrations sont utilisables tels quels dans le jeu.

Ce paragraphe existe quand même, parce que la contrainte n'est pas supprimée mais **endormie** : elle
se réveille intégralement le jour où le projet serait publié, diffusé ou vendu. Autant savoir dès
maintenant ce qu'il faudrait alors reprendre, plutôt que de le découvrir à ce moment-là.

| Source | Statut | Ce qu'il faudrait faire en cas de publication |
|---|---|---|
| `Basic-Rules-FR`, `Animaux`, `Glossaire` | **OGL 1.0a**, diffusion libre (aidedd.org) | Rien, hors ajout de la notice OGL et de l'attribution SRD 5.1 à `THIRD-PARTY-NOTICES.md` |
| `Tanares_Sourcebook`, `Players_Guide` | Mécanique SRD ouverte ; lore, noms et art **réservés** (*Product Identity* : « Tanares », « Penumbral Plane », « madwalker », « taii'maku », « Golgöggoth », « Isendden », « emogum ») | Renommer le monde et ses entités, réécrire les textes, remplacer les illustrations |
| `Manuel-Des-Joueurs`, `Manuel-Des-Monstres` | Copyright plein (Wizards of the Coast / Black Book Éditions) | Retirer intégralement les données qui en sont issues |

En pratique, cela suggère une seule discipline, peu coûteuse et qui garde la porte ouverte :
**tracer la provenance**. Chaque donnée produite porte un champ `"source"` (`srd`, `tanares`,
`mm-fr`, `original`). Un jour de publication, la question « qu'est-ce qui doit sauter ? » se répond
par une requête plutôt que par une relecture de 600 fichiers — et le `LOT-32` peut l'imposer par
schéma sans effort supplémentaire.

Cela ne change rien au cadrage acté avant le [LOT-01](@ref lot-01) — **règles d20 maison,
compatibles SRD dans leur structure, sans en dépendre** : le moteur reste indépendant du SRD, ce
sont les *données* qui viennent du corpus.

---

## 4. Ce que l'extraction a appris

Six constats techniques, tous vérifiés sur le corpus, qui déterminent la faisabilité des lots.

**PyMuPDF fait tout, et mieux que l'outillage en ligne de commande.** Le poste dispose de
`pdftotext` version Xpdf 4.00, mais ni `pdfimages` ni `pdftoppm`. **PyMuPDF 1.28.2** (installable
par `pip`, testé) couvre en une seule bibliothèque le texte, les coordonnées de mots, les images et
le rendu de page — c'est la dépendance à retenir pour le `LOT-30`, plutôt qu'un assemblage
d'exécutables partiellement présents.

**Les livres Tanares sont paginés en double page.** 179 pages PDF pour un livre qui numérote jusqu'à
358 ; une page PDF porte deux pages du livre côte à côte, ce qu'un rendu confirme visuellement.
Toute correspondance « page du sommaire → page PDF » doit intégrer ce facteur 2, sinon on cible
systématiquement le mauvais chapitre. C'est la première chose que le manifeste du `LOT-30` doit
enregistrer.

**`pdftotext -layout` mélange les colonnes des tableaux.** Sur la table des armes des *Basic Rules*,
il produit des lignes où le poids et le prix appartiennent à l'arme d'en dessous — une donnée
fausse, et fausse **silencieusement**, ce qui est pire qu'une extraction qui échoue. Le mode
`-table` d'Xpdf les restitue correctement (`Bâton | 1d6 contondant | 2 kg | 2 pa | Polyvalente
(1d8)`), et PyMuPDF permet mieux encore en regroupant les mots par coordonnée x. Dans tous les cas :
**jamais `-layout` sur un tableau**.

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
contrôle. Les 82 blocs du Sourcebook suivent un gabarit analogue en anglais.

**Les deux manuels français sont des OCR bruités.** Leurs pages de crédits donnent « Cordeil » pour
« Cordell », « )on Schindehette » pour « Jon Schindehette », « Chefde projet », « W'àyne Reynolds ».
Sur un nom d'auteur c'est anecdotique ; sur un jet de dégâts, c'est un `1d8` devenu `ld8` que rien
ne rattrape. Leurs 416 blocs restent exploitables, mais **jamais en confiance** : toute valeur
numérique qui en sort doit passer un contrôle de plausibilité, et le champ `"source": "mm-fr"` sert
aussi à savoir quoi relire en priorité.

**L'extraction du flux brut des images est corrompue ; il faut passer par le rendu.** Tirer un objet
image par son `xref` produit sur ce corpus des zones de bruit vert et cyan — décodage raté d'un flux
JPX ou d'un masque alpha. En revanche, **rendre la page** (ou une région clippée de la page) donne
un résultat parfait, parce que le rendu passe par la composition complète. Avec une réserve qui
compte : rendre une région composite **tout ce qui y est dessiné, texte compris**. Sur ces livres où
l'art court sous les colonnes, l'extraction d'illustrations propres est donc **semi-automatique** —
l'outil propose les régions candidates, l'humain garde et recadre. Les éléments d'habillage
autonomes (panneaux de parchemin, cadres, bordures) sortent en revanche parfaitement seuls, et ce
sont eux qui alimenteront l'interface.

---

## 5. Programme de lots proposé — phase F, contenu du monde

Les lots [LOT-13](@ref lot-13) (fiche de personnage), [LOT-14](@ref lot-14) (inventaire),
[LOT-25](@ref lot-25) (sorts) et [LOT-26](@ref lot-26) (économie) exigent tous un « catalogue en
**JSON** », conformément à [`EX-VIS-007`](@ref EX-VIS-007). Aucun ne dit **d'où sortent ces JSON**.
C'est exactement le trou que ce corpus comble, et c'est le périmètre de la phase F.

Dix lots, `LOT-30` à `LOT-39`. Les numéros sont, comme toujours, des identifiants stables : ils
viennent après [LOT-29](@ref lot-29) dans la numérotation, mais plusieurs s'exécutent **avant** les
lots qui les consomment (voir §6). Une nouvelle famille d'exigences `EX-CNT-*` les couvre, à écrire
dans un `Documentation/Specification/contenu.md`.

Trois formes de sortie, et le choix entre elles n'est pas cosmétique. **JSON** pour tout ce qui est
structuré et imbriqué — une créature a des actions, une classe a une progression par niveau — et
parce que le projet lit déjà du JSON (`skins.json`, `sounds.json`, `palettes.json`). **CSV** pour la
seule donnée réellement tabulaire et plate du corpus, le lexique du `LOT-31` : 1 200 lignes de trois
colonnes, qu'on veut pouvoir trier, comparer et corriger dans un tableur sans passer par un éditeur
de code. **PNG** enfin, pour les textures du `LOT-39`.

### `LOT-30` — Chaîne d'extraction du corpus

*Aucun prérequis. Prérequis de tous les autres.*

L'outillage, et rien d'autre. `scripts/sourcebook/`, sur **PyMuPDF** : extraction de texte, de
tableaux par coordonnées, d'images par rendu clippé ; gestion de la pagination en double page ;
cache disque. Un manifeste `scripts/sourcebook/corpus.toml` enregistre par document son empreinte
SHA-256, son nombre de pages, son décalage de pagination et sa **provenance** (`srd`, `tanares`,
`mm-fr`) — celle-là même que chaque donnée produite reportera (§3).

Le manifeste est versionné ; le corpus intermédiaire ne l'est pas, et la CI ne l'exécute pas : les
PDF ne sont pas sur le runner. Seules les **données produites** sont validées en CI (`LOT-32`).

*Acceptation* — deux exécutions successives produisent des empreintes identiques ; une empreinte qui
ne correspond plus au PDF présent fait échouer l'extraction plutôt que de produire des données
silencieusement décalées.

### `LOT-31` — Lexique bilingue et autorité de traduction

*Prérequis : `LOT-30`.*

Les ~1 200 paires du `Glossaire.pdf` deviennent `Source/Elements/Localization/rpg.glossary.csv`
(`anglais;français;catégorie`), complétées par le vocabulaire des *Basic Rules* (états, écoles de
magie, types de dégâts, propriétés d'armes).

L'intérêt n'est pas de traduire l'interface — `fr.lang` et `en.lang` font déjà cela. Il est
d'imposer **une seule traduction par terme de règle dans tout le jeu**. Sans table d'autorité,
*saving throw* devient « jet de sauvegarde » dans la fiche, « JdS » dans le journal de combat et
« sauvegarde » dans l'infobulle, et le joueur croit à trois mécaniques différentes. Le lexique sert
en outre à **traduire les données anglaises** du Sourcebook et du *Player's Guide* de façon
cohérente avec celles venues du français.

*Acceptation* — `scripts/check_glossary.py` échoue si une clé de règle de `fr.lang` emploie un terme
absent du lexique ou en contredit la traduction.

### `LOT-32` — Schémas de données RPG

*Prérequis : `LOT-30`. Prérequis de `LOT-33` à `LOT-37`.*

Le contrat avant les données. Un JSON Schema par famille — créature, objet, arme, armure, sort,
espèce, classe, historique, état, type de dégâts — sous `Source/Elements/Rpg/schema/`, et
`scripts/check_rpg_data.py` qui valide tout `Source/Elements/Rpg/**.json` en CI. Le champ
`"source"` du §3 y est **obligatoire** : c'est une ligne de schéma, et elle rend la question de la
provenance définitivement réglée.

Un test vérifie en plus que les énumérations C++ (`DamageType`, `Condition`…) et celles des schémas
**coïncident**. C'est le point où une donnée et un moteur divergent en silence : le JSON déclare
`"psychique"`, le C++ ne connaît que `Psychic`, la valeur tombe dans le cas par défaut, et le sort
ne fait plus de dégâts sans que rien ne l'annonce.

*Acceptation* — une donnée invalide fait échouer la CI avec le fichier et la ligne fautifs ; ajouter
une valeur d'énumération d'un seul côté fait échouer un test.

### `LOT-33` — Bestiaire

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13), [LOT-21](@ref lot-21),
[LOT-23](@ref lot-23).*

Trois gisements, par ordre de coût croissant, vers `Source/Elements/Rpg/creatures/*.json` :

1. les **94 bêtes** d'`Animaux.pdf` — gabarit régulier, français, automatisable de bout en bout ;
2. les **82 blocs de Tanares** (ch. 9, 17 familles) — gabarit analogue en anglais, traduits via le
   lexique du `LOT-31` ;
3. les **416 blocs du *Manuel des Monstres*** — OCR bruité, donc contrôle de plausibilité
   obligatoire sur chaque valeur numérique et relecture par lots.

Soit près de **600 profils** à terme. Le premier gisement suffit largement à démarrer : loup, ours,
araignée géante et sanglier peuplent le donjon du [LOT-27](@ref lot-27) sans attendre.

L'ordre compte. Livrer les 94 d'abord fait tourner tout l'aval — attaques, IA, rencontres — sur des
données réelles pendant que les 500 autres arrivent. L'inverse, tout extraire avant de brancher quoi
que ce soit, revient à découvrir 600 erreurs d'un coup.

*Acceptation* — les profils du gisement traité chargent ; un test rejoue dix d'entre eux (CA, PV,
vitesse, facteur de puissance, dégâts de l'attaque principale) contre des valeurs recopiées à la
main. Aucune valeur issue de `mm-fr` n'entre sans avoir passé le contrôle de plausibilité.

### `LOT-34` — Équipement, monnaie, objets magiques

*Prérequis : `LOT-32`. Alimente [LOT-14](@ref lot-14), [LOT-26](@ref lot-26).*

Les tables d'équipement des *Basic Rules* vers `Source/Elements/Rpg/items/` : armes (dégâts, poids,
prix, propriétés), armures (CA, exigence de Force, discrétion), matériel, outils, montures,
marchandises, services. Puis le **chapitre 10 du Sourcebook** — les objets magiques — qui donne au
[LOT-26](@ref lot-26) de quoi remplir des coffres autrement qu'avec de l'or.

**Jamais `-layout` sur ces tables** (§4), et relecture humaine par-dessus : c'est le lot où le §4 se
paie. Une valeur de prix décalée d'une ligne ne casse rien, ne lève aucune alerte, et déséquilibre
l'économie sans que personne ne comprenne pourquoi.

*Acceptation* — la CA calculée par le moteur pour chacune des 13 armures du catalogue égale la
colonne CA de la table, vérifié par un test ; le poids total d'un inventaire de départ correspond au
calcul manuel.

### `LOT-35` — Sorts et états

*Prérequis : `LOT-32`. Alimente [LOT-25](@ref lot-25).*

Les sorts et l'appendice des états des *Basic Rules* vers `spells/` et `conditions/`, complétés par
les sorts propres au *Player's Guide* (liste de l'élémentaliste, descriptions du ch. 4).

La règle qui compte est celle que le [LOT-25](@ref lot-25) énonce déjà : le C++ ne porte que des
*mécanismes* (dégâts de zone, jet de sauvegarde, condition appliquée, durée), la donnée les compose.
Un sort dont l'effet n'entre pas dans ces mécanismes se déclare **explicitement**
`"effet": "narratif"` — un sort tu, qu'on croit implémenté et qui ne fait rien, coûte bien plus cher
à diagnostiquer qu'un sort déclaré non joué.

*Acceptation* — chaque sort porte école, niveau, portée, durée, composantes, effet ; aucun sort ne
tombe silencieusement dans un cas par défaut, un test énumère le catalogue et l'exige.

### `LOT-36` — Espèces, classes, historiques

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13).*

Deux niveaux, et le second n'attend que le premier.

Le **socle** : les 4 races, 4 classes et 6 historiques des *Basic Rules*, en français, suffisants
pour jouer.

L'**extension Tanares** : les 13 espèces du *Player's Guide* (dont cirrus, gloomfolk, soulborn,
taii'maku), ses 4 classes inédites (dragonblade, élémentaliste, madwalker, redeemer), ses 4 classes
simplifiées, sa vingtaine de sous-classes et ses 7 historiques. Les **31 tables de progression**
détectées sont le vrai travail : chacune est un tableau de 20 lignes à extraire sans décalage.

*Acceptation* — celle du [LOT-13](@ref lot-13), tenue par les données : trois classes chargent et
donnent les bons modificateurs, et la progression du niveau 1 au niveau 5 ne fait intervenir aucune
valeur codée en C++. Une table de progression extraite est comparée ligne à ligne au PDF sur au
moins trois classes.

### `LOT-37` — Atlas du monde et graphe de cartes

*Prérequis : `LOT-32`. Alimente [LOT-09](@ref lot-09), [LOT-27](@ref lot-27).*

Le monde de Tanares devient l'atlas du jeu, tel quel : **10 régions**, **5 factions**, **18
divinités**, **11 organisations**, **7 lieux du plan pénombral**, une chronologie et une économie de
minerais rares, vers `Source/Elements/World/`.

Sa vertu est d'être déjà **cohérent** : une région porte des lieux, un lieu porte des factions
présentes et des PNJ notables, une faction porte des objectifs qui se traduisent en quêtes, un
panthéon porte des domaines qui se traduisent en capacités. C'est précisément ce qu'un monde inventé
au fil de l'eau n'a jamais, et ce qui rend le [LOT-16](@ref lot-16) (quêtes) écrivable plutôt
qu'improvisable.

Le branchement est direct sur le [LOT-09](@ref lot-09) : une région est un nœud du graphe de cartes,
un lieu est une carte à créer. Les cartes de régions du Sourcebook servent de référence de tracé au
`LOT-39`.

Le périmètre du *vertical slice* n'a pas besoin des dix régions : **une seule**, avec son village et
son donjon, suffit au [LOT-27](@ref lot-27). Les neuf autres sont de la donnée qui attend, pas du
travail à faire tout de suite.

*Acceptation* — `check_world_graph.py` (prévu au [LOT-27](@ref lot-27)) valide l'atlas ; aucun lieu
sans région, aucune faction référencée qui n'existe pas, aucune région inatteignable.

### `LOT-38` — Fiche de personnage : maquette et interface

*Prérequis : [LOT-13](@ref lot-13). Alimente [LOT-24](@ref lot-24).*

`Character_Sheets_Tanares.pdf` fournit ses 5 planches en image à environ 300 ppp (2 668 × 3 418) :
une maquette précise de l'écran de fiche — quels champs, groupés en quels blocs (identité,
caractéristiques, compétences, combat, équipement, sorts), dans quel ordre de lecture. L'écran se
construit sur le système de design existant, avec les panneaux de parchemin extraits au `LOT-39`.

*Acceptation* — chaque champ de la maquette est soit affiché à l'écran, soit inscrit dans une liste
explicite de champs hors périmètre. Un champ simplement oublié n'est pas un arbitrage.

### `LOT-39` — Textures, habillage et catalogue d'assets

*Prérequis : `LOT-33`, `LOT-34`, `LOT-37`. Alimente [LOT-11](@ref lot-11),
[LOT-27](@ref lot-27), `LOT-38`.*

Le corpus contient environ **6 000 images**, dont plus de 4 400 au-delà de 512 × 512. Trois usages,
de difficulté croissante — et la méthode est celle du §4 : **rendu clippé, jamais extraction du flux
brut**.

- **L'habillage d'interface**, immédiat et le plus rentable : panneaux de parchemin, cadres,
  bordures, pictogrammes. Ces éléments sont autonomes dans la page, sortent proprement, et habillent
  la fiche du `LOT-38`, les boîtes de dialogue du [LOT-15](@ref lot-15) et l'IHM de combat du
  [LOT-24](@ref lot-24).
- **Les portraits et illustrations** — créatures, espèces, classes, PNJ. Semi-automatique : l'outil
  propose les régions candidates triées par taille, l'humain garde et recadre, parce que l'art court
  sous les colonnes de texte et qu'un rendu clippé embarque ce texte.
- **Les cartes de régions**, référence de tracé pour les cartes jouables du `LOT-37` — à redessiner
  à l'échelle de la tuile, pas à afficher telles quelles.

Le lot pose par ailleurs la plomberie qui manque : une **clé d'asset** portée par chaque donnée
(`"asset": "beast/wolf"`), jamais un chemin de fichier — un chemin dans une donnée de règle lie le
catalogue à l'arborescence du disque, et tout déplacement de dossier casse alors des créatures. Un
**manifeste** dérivé des catalogues énumère les clés attendues, leur famille et leur taille. Et
toute clé sans image obtient un **marqueur généré** sur l'`ProceduralAtlas` existant, de sorte que
le jeu tourne complet — 600 créatures affichables — avant que la découpe manuelle ne soit finie. La
production graphique devient un remplacement progressif de marqueurs, jamais un préalable bloquant.

La distinction avec le [LOT-08](@ref lot-08) est nette : le `LOT-08` a fixé le vocabulaire des
**tuiles de terrain**, celui-ci fixe celui des **entités**.

*Acceptation* — aucune clé d'asset orpheline dans les catalogues ; toute clé sans image obtient un
marqueur généré, et la CI **liste** les clés sans art définitif sans échouer : c'est un état
d'avancement, pas un défaut.

---

## 6. Ordre d'exécution recommandé

Les numéros suivent [LOT-29](@ref lot-29), l'exécution non. Les lots de données doivent précéder les
lots qui les consomment, sans quoi ces derniers se construisent sur des catalogues fictifs — et un
catalogue fictif finit toujours par se figer en valeurs codées en dur, exactement ce que
[`EX-VIS-007`](@ref EX-VIS-007) interdit.

| Quand | Lots | Pourquoi là |
|---|---|---|
| Tout de suite après [LOT-08](@ref lot-08) | `LOT-30`, `LOT-31`, `LOT-32` | Outillage et contrats, sans dépendance ; le plus tôt est le mieux |
| Avec [LOT-09](@ref lot-09) | `LOT-37` (une région) | L'atlas donne au graphe de cartes de vrais nœuds à relier |
| Avant [LOT-13](@ref lot-13) | `LOT-36` (socle) | La fiche a besoin de vraies classes pour valider sa progression |
| Avant [LOT-14](@ref lot-14) | `LOT-34` | L'inventaire a besoin d'un vrai catalogue pour prouver l'absence de dérive de CA |
| Avant [LOT-21](@ref lot-21) / [LOT-23](@ref lot-23) | `LOT-33` (les 94) | Attaques et IA ont besoin de vraies créatures |
| Avant [LOT-25](@ref lot-25) | `LOT-35` | Les sorts sont des données avant d'être un système |
| Après [LOT-13](@ref lot-13) | `LOT-38`, puis `LOT-39` (habillage) | La maquette suppose la fiche existante |
| En fond, sans jalon | `LOT-33` (Tanares, MM), `LOT-36` (extension), `LOT-39` (portraits) | Volume long, sans blocage : se remplit par lots successifs |

La dernière ligne est la plus importante du tableau. Près de 600 créatures, 31 tables de
progression et 6 000 images ne se traitent pas d'un bloc, et rien n'y oblige : chaque gisement est
incrémental, et le jeu tourne dès le premier.

---

## 7. Arborescence cible

```
Documentation/SourceBook/   ← les PDF, exclus du dépôt (taille)

scripts/sourcebook/
  corpus.toml               ← manifeste : empreintes, pagination, provenance (LOT-30)
  *.py                      ← chaîne d'extraction, sur PyMuPDF (LOT-30)
scripts/check_rpg_data.py, check_glossary.py

Source/Elements/Rpg/
  schema/*.schema.json      ← contrats (LOT-32)
  creatures/*.json          ← ~600 profils à terme (LOT-33)
  items/*.json              ← équipement, monnaie, objets magiques (LOT-34)
  spells/, conditions/      ← sorts et états (LOT-35)
  species/, classes/, backgrounds/   ← création de personnage (LOT-36)

Source/Elements/World/      ← régions, lieux, factions, panthéon (LOT-37)
Source/Elements/Assets/
  rpg.assets.json           ← manifeste des clés d'assets (LOT-39)
  Ui/, Portraits/, Creatures/        ← textures extraites (LOT-39)
Source/Elements/Localization/rpg.glossary.csv   ← lexique (LOT-31)
```

---

## 8. Ce qui reste à trancher

- **Périmètre du socle jouable.** Les 4 classes des *Basic Rules* suffisent-elles au *vertical
  slice*, ou faut-il embarquer d'emblée l'extension Tanares du `LOT-36` ?
- **Région du *slice*.** Laquelle des dix sert de terrain au [LOT-27](@ref lot-27) ? Le choix décide
  des cartes à dessiner et des créatures à extraire en priorité.
- **Nouvelle famille d'exigences.** `EX-CNT-*` dans un `Documentation/Specification/contenu.md`
  reste à écrire ; les lots ci-dessus la référencent par anticipation, comme le
  [LOT-13](@ref lot-13) référence déjà `EX-DND-*` qui n'existe pas encore.
