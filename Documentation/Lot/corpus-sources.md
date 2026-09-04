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
| `Manuel-Des-Joueurs.pdf` | *Player's Handbook* D&D 5 en français — **5 races et 8 classes** absentes des *Basic Rules* | 320 | FR | scan OCR, **tables récupérables en `-table`** | 631 (453 ≥ 512²) |
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
- **`Manuel-Des-Monstres.pdf`** — **416 blocs de statistiques** en français, le plus gros bestiaire
  du corpus. **Hors périmètre** (§8) : son OCR bruité en fait aussi le plus coûteux, et les 176
  créatures des deux autres gisements suffisent à peupler dix régions. Il reste disponible si le
  besoin s'en fait sentir plus tard.
- **`Manuel-Des-Joueurs.pdf`** — **la source française des races et classes manquantes**, et à ce
  titre indispensable. Les *Basic Rules* ne portent que 4 races et 4 classes ; ce manuel apporte les
  **5 races** restantes (drakéide, gnome, demi-elfe, demi-orc, tieffelin) et les **8 classes**
  restantes (barbare, barde, druide, moine, paladin, rôdeur, ensorceleur, sorcier) — toutes
  vérifiées présentes. Sans lui, les sous-classes Tanares pour barbare, barde, druide, moine,
  paladin, rôdeur et ensorceleur n'auraient **aucune classe de base sur laquelle se greffer**. Son
  OCR est bruité mais gérable : voir §4.
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
| `Manuel-Des-Joueurs` (dans le périmètre), `Manuel-Des-Monstres` (hors périmètre) | Copyright plein (Wizards of the Coast / Black Book Éditions) | Retirer les données marquées `phb-fr` — soit 5 races et 8 classes, à remplacer par des créations propres |

En pratique, cela suggère une seule discipline, peu coûteuse et qui garde la porte ouverte :
**tracer la provenance**. Chaque donnée produite porte un champ `"source"` (`srd`, `tanares`,
`phb-fr`, `original`). Un jour de publication, la question « qu'est-ce qui doit sauter ? » se répond
par une requête plutôt que par une relecture de tout le catalogue — et le `LOT-32` peut l'imposer par
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

**Les deux manuels français sont des OCR bruités — mais moins gravement qu'il n'y paraît.** Leurs
pages de crédits donnent « Cordeil » pour « Cordell », « )on Schindehette », « Chefde projet »,
« W'àyne Reynolds ». Il serait tentant d'en conclure que ces fichiers sont inexploitables ; ce
serait généraliser depuis la page la plus bruitée du livre, et ce serait faux. Le bruit se répartit
en **trois niveaux, de gravité très inégale** :

- **La structure des lignes** — le plus dangereux, et le plus facile à corriger. En `-layout`, la
  table du barbare désynchronise ses colonnes : le niveau 5 y reçoit « Amélioration de
  caractéristiques » au lieu d'« Attaque supplémentaire », toute la table décalée d'un cran. En
  `-table`, elle est **exacte**, niveau par niveau. Même règle qu'au paragraphe précédent, cette
  fois démontrée sur un scan : **jamais `-layout` sur un tableau**.
- **Le bruit au niveau du mot** — « Voire primitive » pour « Voie primitive », « sup plémentaire »,
  « unjet ». Visible, non numérique, corrigé par une relecture ordinaire. Désagréable, pas
  dangereux.
- **Les valeurs numériques** — le risque résiduel, celui qu'aucune relecture rapide n'attrape : un
  `1d8` devenu `ld8`, un `+3` devenu `+8`. C'est précisément la raison d'être du `LOT-49`.

Conclusion : le `Manuel-Des-Joueurs.pdf` est **exploitable** avec le bon outil et une passe de
contrôle — et il est indispensable, puisqu'il est la seule source française des 5 races et 8 classes
absentes des *Basic Rules*. Le `Manuel-Des-Monstres.pdf` reste hors périmètre, non par impossibilité
mais par arbitrage (§8) : 416 blocs à contrôler pour un bestiaire déjà suffisant.

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

## 5. Programme de lots proposé — la filière contenu

Les lots [LOT-13](@ref lot-13) (fiche de personnage), [LOT-14](@ref lot-14) (inventaire),
[LOT-25](@ref lot-25) (sorts) et [LOT-26](@ref lot-26) (économie) exigent tous un « catalogue en
**JSON** », conformément à [`EX-VIS-007`](@ref EX-VIS-007). Aucun ne dit **d'où sortent ces JSON**.
C'est exactement le trou que ce corpus comble, et c'est le périmètre de cette filière.

Vingt lots, `LOT-30` à `LOT-49`. Les numéros sont, comme toujours, des identifiants stables : ils
viennent après [LOT-29](@ref lot-29) dans la numérotation, mais plusieurs s'exécutent **avant** les
lots qui les consomment (voir §6). Une nouvelle famille d'exigences `EX-CNT-*` les couvre, à écrire
dans un `Documentation/Specification/contenu.md`.

Trois formes de sortie, et le choix entre elles n'est pas cosmétique. **JSON** pour tout ce qui est
structuré et imbriqué — une créature a des actions, une classe a une progression par niveau — et
parce que le projet lit déjà du JSON (`skins.json`, `sounds.json`, `palettes.json`). **CSV** pour la
seule donnée réellement tabulaire et plate du corpus, le lexique du `LOT-31` : 1 200 lignes de trois
colonnes, qu'on veut pouvoir trier, comparer et corriger dans un tableur sans passer par un éditeur
de code. **PNG** enfin, pour les textures du `LOT-39`.

### Importer tout, honorer progressivement

Le périmètre est **le monde complet** : les 10 régions et leurs lieux, les 13 espèces, les 8 classes
et leurs sous-classes, les historiques, les factions, le panthéon, le bestiaire entier. Pas de
socle réduit, pas de tri préalable.

C'est le bon choix, et il est peu coûteux : extraire dix régions ne demande pas dix fois le travail
d'en extraire une, puisque c'est le **parseur** qui coûte, pas les données qu'il avale. Mais il
introduit un écart qu'il faut nommer tout de suite, sous peine de le découvrir en jeu :

> **Une donnée importée n'est pas une mécanique implémentée.** L'élémentaliste a sa propre liste de
> sorts, le madwalker ses *trails*, le redeemer ses *blessings*. Importer ces classes prend une
> après-midi ; faire que le moteur les joue correctement est un programme entier.

La règle qui évite le piège est celle que le [LOT-25](@ref lot-25) pose déjà pour les sorts, étendue
à toute la filière : **chaque donnée déclare les mécanismes dont elle a besoin**, et le moteur
**refuse en le disant** ce qu'il ne sait pas honorer. Une classe dont le moteur ignore la ressource
propre se signale au chargement, elle ne se joue pas en silence comme une classe ordinaire amputée
de ce qui la définit.

Ainsi le catalogue peut être complet longtemps avant le moteur, sans jamais mentir sur ce qui est
jouable — et l'écart entre les deux devient une liste consultable plutôt qu'une surprise.

### La cible : un bac à sable dans l'univers entier

Le jeu visé dans un premier temps est un **bac à sable** — le personnage parcourt les dix régions
librement, sans intrigue directrice. Ce cadrage n'ajoute pas une contrainte à la filière, il en
change la nature, et pour le mieux.

Un bac à sable se nourrit de contenu **systémique** : les rencontres naissent d'une table et d'un
lieu, pas d'un script ; le stock d'un marchand se déduit de sa région ; une quête se compose à
partir des objectifs d'une faction et de l'état du monde. Rien de tout cela ne s'écrit à la main
région par région — et c'est précisément ce que le corpus permet d'éviter, parce que les
**`Regional Statistics` sont déjà des réglages de bac à sable**. `Monster Presence`, `Magic Access`,
`Economic Prosperity`, `Crime and Violence` : quatre axes notés, dix régions, et le monde se
différencie mécaniquement sans qu'une seule valeur soit inventée.

D'où trois lots supplémentaires, `LOT-40` à `LOT-42`, qui transforment l'atlas en monde parcourable :
le terrain, son peuplement, et le voyage entre les régions.

Cela ne remplace pas le [LOT-27](@ref lot-27), qui garde sa fonction : prouver que la boucle
complète tourne — explorer, parler, déclencher, combattre, gagner — sur **un** village et **un**
donjon. Un bac à sable est cette boucle répétée sur cent lieux ; la construire sur cent lieux avant
de l'avoir validée sur un seul multiplierait simplement par cent le coût de chaque correction.

### `LOT-30` — Chaîne d'extraction du corpus

*Aucun prérequis. Prérequis de tous les autres.*

L'outillage, et rien d'autre. `scripts/sourcebook/`, sur **PyMuPDF** : extraction de texte, de
tableaux par coordonnées, d'images par rendu clippé ; gestion de la pagination en double page ;
cache disque. Un manifeste `scripts/sourcebook/corpus.toml` enregistre par document son empreinte
SHA-256, son nombre de pages, son décalage de pagination et sa **provenance** (`srd`, `tanares`,
`phb-fr`) — celle-là même que chaque donnée produite reportera (§3).

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

### `LOT-33` — Bestiaire de base

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13), [LOT-21](@ref lot-21),
[LOT-23](@ref lot-23).*

Les **94 bêtes** d'`Animaux.pdf` vers `Source/Elements/Rpg/creatures/*.json`. Gabarit régulier,
français, automatisable de bout en bout : c'est le seul gisement du corpus qui ne demande aucun
jugement humain, et c'est pour cela qu'il est seul dans ce lot.

Les 82 blocs de Tanares relèvent du `LOT-46` : mêmes gabarits, mais en anglais, donc une étape de
traduction qui n'a rien à faire ici.

Livrer les 94 d'abord fait tourner tout l'aval — attaques, IA, rencontres — sur des données réelles
pendant que le reste arrive. Loup, ours, araignée géante et sanglier peuplent le donjon du
[LOT-27](@ref lot-27) sans attendre.

*Acceptation* — les 94 profils chargent ; un test rejoue dix d'entre eux (CA, PV, vitesse, facteur
de puissance, dégâts de l'attaque principale) contre des valeurs recopiées à la main du PDF.

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

### `LOT-36` — Espèces, historiques et classes provisoires

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13).*

De quoi construire un personnage jouable au plus tôt, en trois apports :

- **Les espèces** — les 4 races des *Basic Rules* et les 5 du *Manuel des Joueurs* (drakéide, gnome,
  demi-elfe, demi-orc, tieffelin) pour le vocabulaire français de référence, puis les **13 espèces**
  du *Player's Guide* (dont cirrus, gloomfolk, soulborn, taii'maku). Là où les deux se recouvrent —
  nain, elfe, halfelin, humain, gnome, drakéide, tieffelin — **la version de Tanares fait foi** sur
  le fond, le manuel français fournissant la formulation.
- **Les 7 historiques** du *Player's Guide* et les 6 des *Basic Rules*.
- **Les 4 classes simplifiées** de Tanares — brawler, mage, priest, scoundrel — comme **socle
  provisoire du premier modèle de combat**. Leur intérêt est d'être simples : elles font tourner
  attaques, dégâts et tours (`LOT-21`, `LOT-20`) sans exiger d'abord le système complet de
  ressources de classe.

> **Ces quatre classes sont temporaires et doivent le déclarer.** Elles seront retirées du jeu
> final, remplacées par les 12 classes complètes du `LOT-47`. Une donnée provisoire non marquée
> devient permanente par accident — c'est la façon la plus banale dont un échafaudage finit en mur
> porteur. D'où un champ `"statut": "provisoire"` au schéma, la CI qui **liste** ce qui le porte, et
> un critère de retrait écrit d'avance : le jour où les 12 classes sont livrées, supprimer ces
> quatre fichiers ne doit casser aucune référence.

Les 12 classes complètes et les 31 tables de progression relèvent du `LOT-47` : une espèce se décrit
en une douzaine de champs, une classe en un tableau de 20 lignes à relire ligne à ligne. Mélanger
les deux ferait dépendre l'arrivée des espèces d'un travail dix fois plus long.

Chaque classe et chaque espèce déclare les **mécanismes** qu'elle exige (emplacements de sorts,
ressource propre, liste de sorts dédiée, choix de sous-classe au niveau *n*). C'est cette
déclaration qui permet au moteur de dire ce qu'il ne sait pas encore jouer, au lieu de servir une
classe amputée.

*Acceptation* — celle du [LOT-13](@ref lot-13), tenue par les données : trois classes chargent et
donnent les bons modificateurs, et la progression du niveau 1 au niveau 5 ne fait intervenir aucune
valeur codée en C++. Toute espèce ou classe exigeant un mécanisme absent du moteur est **listée au
chargement**, jamais jouée en silence. Les quatre classes provisoires portent leur marque, et un
test vérifie qu'aucune donnée **définitive** ne les référence.

### `LOT-37` — Atlas du monde et graphe de cartes

*Prérequis : `LOT-32`. Alimente [LOT-09](@ref lot-09), [LOT-27](@ref lot-27).*

Le monde de Tanares devient l'atlas du jeu, en entier : **10 régions** et leurs lieux, **5
factions**, **18 divinités**, **11 organisations**, **7 lieux du plan pénombral**, une chronologie
et une économie de minerais rares, vers `Source/Elements/World/`.

Sa vertu est d'être déjà **cohérent** : une région porte des lieux, un lieu porte des factions
présentes et des PNJ notables, une faction porte des objectifs qui se traduisent en quêtes, un
panthéon porte des domaines qui se traduisent en capacités. C'est précisément ce qu'un monde inventé
au fil de l'eau n'a jamais, et ce qui rend le [LOT-16](@ref lot-16) (quêtes) écrivable plutôt
qu'improvisable.

Chaque région suit un gabarit régulier, donc parsable : `Government`, `Faction`, `Population` (avec
répartition par espèce en pourcentage), puis les sections `Geography`, `Politics`, `Economy`,
`Culture and Society`, puis ses lieux nommés. Certains lieux portent même des **effets mécaniques**
directement implémentables — les Sacred Ponds du Bak Forest doublent les PV rendus par dé de vie
lors d'un repos court et annulent un niveau d'épuisement.

Et surtout un encart **`Regional Statistics`** : sept axes notés de *Very Low* à *Very High* —
`Citizen Freedom`, `Crime and Violence`, `Economic Prosperity`, `Government Corruption`,
`Magic Access`, `Monster Presence`, `Political Stability`. Ce sont des **paramètres de gameplay déjà
tabulés**, et il serait dommage de les traiter comme de la couleur : `Monster Presence` règle le
taux de rencontre, `Magic Access` le stock d'un marchand, `Economic Prosperity` ses prix,
`Crime and Violence` la fréquence des PNJ hostiles. Dix régions livrent ainsi dix ambiances
mécaniquement distinctes sans qu'aucune valeur ne soit inventée.

Le branchement est direct sur le [LOT-09](@ref lot-09) : une région est un nœud du graphe de cartes,
un lieu est une carte à créer. Les cartes de régions du Sourcebook servent de référence de tracé au
`LOT-39`.

L'objectif étant l'**univers complet et jouable**, les dix régions doivent toutes exister en jeu, pas
seulement en données. C'est ce qui rend les `LOT-40` et `LOT-41` nécessaires : à raison d'une dizaine
de lieux nommés par région, on parle d'une **centaine de cartes**, un volume qui ne se dessine pas à
la main et qui doit donc se **générer depuis cet atlas**. Le présent lot est ce qui rend cette
génération possible : c'est lui qui fournit la matière que le `LOT-40` transformera en terrain.

*Acceptation* — `check_world_graph.py` (prévu au [LOT-27](@ref lot-27)) valide l'atlas ; aucun lieu
sans région, aucune faction référencée qui n'existe pas, aucune région inatteignable. Les sept
statistiques régionales sont typées, pas laissées en texte libre. Chaque région déclare un
**descripteur de terrain** exploitable par le `LOT-40` — proportions de biomes, présence d'eau, de
relief, de bâti — dérivé de sa section `Geography`.

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

Ce lot pose la **plomberie**, et livre le seul gisement d'images qui sort proprement sans arbitrage
humain. Les portraits et illustrations, qui demandent un recadrage pièce par pièce, relèvent du
`LOT-48` — mélanger les deux ferait dépendre une brique d'architecture d'un travail de découpe sans
terme.

La plomberie : une **clé d'asset** portée par chaque donnée (`"asset": "beast/wolf"`), jamais un
chemin de fichier — un chemin dans une donnée de règle lie le catalogue à l'arborescence du disque,
et tout déplacement de dossier casse alors des créatures. Un **manifeste** dérivé des catalogues
énumère les clés attendues, leur famille et leur taille. Et toute clé sans image obtient un
**marqueur généré** sur l'`ProceduralAtlas` existant, de sorte que le jeu tourne complet — 176
créatures affichables — avant que la moindre découpe ne soit faite. La production graphique devient
un remplacement progressif de marqueurs, jamais un préalable bloquant.

L'**habillage d'interface** ensuite : panneaux de parchemin, cadres, bordures, pictogrammes. Ces
éléments sont autonomes dans la page et sortent parfaitement par rendu clippé (méthode du §4 :
**jamais l'extraction du flux brut**). Ils habillent la fiche du `LOT-38`, les boîtes de dialogue du
[LOT-15](@ref lot-15) et l'IHM de combat du [LOT-24](@ref lot-24).

La distinction avec le [LOT-08](@ref lot-08) est nette : le `LOT-08` a fixé le vocabulaire des
**tuiles de terrain**, celui-ci fixe celui des **entités**.

*Acceptation* — aucune clé d'asset orpheline dans les catalogues ; toute clé sans image obtient un
marqueur généré, et la CI **liste** les clés sans art définitif sans échouer : c'est un état
d'avancement, pas un défaut.

### `LOT-40` — Génération de terrain pilotée par l'atlas

*Prérequis : `LOT-37`, [LOT-11](@ref lot-11). Alimente `LOT-41`.*

Dix régions, une dizaine de lieux nommés chacune : environ **cent cartes**. Dessinées à la main dans
l'éditeur, à raison de quelques heures pièce, c'est une année de travail pour un développeur seul —
et la certitude que le monde ne sera jamais fini. Elles doivent donc se **générer**.

La génération n'est pas un pis-aller ici, parce que la matière existe déjà. Le `LOT-37` livre pour
chaque région un descripteur de terrain dérivé de sa section `Geography`, et le vocabulaire de
tuiles est posé depuis le [LOT-08](@ref lot-08) : `Grass`, `Dirt`, `Sand`, `Water`, `DeepWater`,
`Wall`, `Cliff`, `Bridge`, `Stairs`. Le Central Empire annonce « vallées fluviales, vastes
prairies, forêt du Bak, marais, hauts plateaux du nord » ; le Freelands « landes et zones humides,
forêts tempérées, chaînes glacées, deux mers ». Ce sont des recettes de terrain, pas de la prose
d'ambiance.

Le lot produit un générateur qui, d'un descripteur de région et d'une **graine dérivée de l'identité
du lieu**, rend une carte au format du [LOT-04](@ref lot-04) : couches, collision, portails. La
graine dérive du lieu et non de l'horloge — même raison qu'au [LOT-26](@ref lot-26) pour les coffres :
une carte qui se re-tire différemment à chaque chargement n'est pas un monde, c'est un kaléidoscope.

Le rapport à l'éditeur est le point à ne pas manquer : la génération **produit un niveau ordinaire**,
que le [LOT-11](@ref lot-11) ouvre, corrige et enregistre comme n'importe quel autre. Un générateur
dont la sortie n'est pas éditable oblige à choisir entre tout générer et tout dessiner ; celui-ci
permet de générer les cent cartes et d'en finir dix à la main — les seules que le joueur regardera
de près.

*Acceptation* — une même graine et un même descripteur rendent deux fois la carte **identique** ;
toute carte générée est traversable de son entrée à sa sortie, vérifié par un parcours automatique ;
une carte générée s'ouvre dans l'éditeur, se modifie et se recharge sans perte.

### `LOT-41` — Peuplement systémique des régions

*Prérequis : `LOT-40`, `LOT-33`, `LOT-34`, [LOT-15](@ref lot-15), [LOT-26](@ref lot-26).*

Une carte vide n'est pas un lieu. Ce lot la peuple, et il le fait **par déduction depuis les données
de région**, jamais par placement manuel — c'est ce qui rend dix régions tenables.

Les sept `Regional Statistics` deviennent des réglages effectifs :

| Statistique | Ce qu'elle pilote |
|---|---|
| `Monster Presence` | Densité des rencontres, dangerosité des tables de la région |
| `Magic Access` | Présence d'objets magiques chez les marchands, PNJ lanceurs de sorts |
| `Economic Prosperity` | Prix pratiqués, richesse des étals, valeur du butin |
| `Crime and Violence` | Fréquence des PNJ hostiles, embuscades sur les routes |
| `Political Stability` | Présence de gardes, réaction aux actes du joueur |
| `Government Corruption` | Disponibilité des marchés noirs et des contrats douteux |
| `Citizen Freedom` | Ton des dialogues génériques, ce que les PNJ osent dire |

S'y ajoutent la **répartition des espèces** de la région (le Freelands est à 70 % humain, 11 % elfe
d'automne, 8 % elfe d'hiver : les PNJ générés suivent cette distribution) et les **factions
présentes**, qui décident de qui tient les lieux et de qui accueille mal le joueur.

Les quêtes se composent sur le même principe : un objectif de faction, une cible dans la région, une
récompense tirée de sa prospérité. Le [LOT-16](@ref lot-16) fournit les drapeaux, ce lot fournit les
gabarits qui s'en servent.

*Acceptation* — deux régions aux statistiques opposées produisent des peuplements **mesurablement**
différents (densité de rencontre, prix moyen, composition des espèces), vérifié par un test ; le
peuplement est reproductible à graine égale ; aucun PNJ ni aucune rencontre n'est placé en dur dans
le code.

### `LOT-42` — Voyage et carte du monde

*Prérequis : `LOT-41`, [LOT-09](@ref lot-09), [LOT-17](@ref lot-17).*

Un univers vaste ne vaut que si l'on peut le parcourir. Ce lot livre la **carte du monde** — les dix
régions, leurs lieux connus, la position du personnage — et le déplacement entre régions : routes,
voyage rapide vers un lieu déjà visité, et le coût que ce voyage représente (temps, ravitaillement,
risque de rencontre selon la `Crime and Violence` traversée).

Il porte aussi la **découverte** : un lieu se révèle en y arrivant ou en l'apprenant d'un PNJ, et
cet état de découverte entre dans la sauvegarde du [LOT-17](@ref lot-17). Sans cela, un monde de
cent lieux s'ouvre entièrement dès la première seconde et n'a plus rien à offrir.

L'IHM s'appuie sur les cartes de régions du Sourcebook, redessinées à l'échelle du jeu par le
`LOT-39`.

*Acceptation* — le joueur atteint les dix régions par le seul jeu, sans commande de débogage ; la
carte du monde n'affiche que ce qui est découvert ; l'état de découverte survit à une sauvegarde et
à un rechargement.

### `LOT-43` — Options de personnage : dons, multiclassage, compétences, langues

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13), `LOT-36`.*

Quatre catalogues oubliés du premier découpage, tous présents dans le corpus et tous exigés par la
fiche de personnage :

- les **dons** (*Basic Rules* p. 61, plus les *New Feats* du *Player's Guide* ch. 4) ;
- le **multiclassage** — ses conditions de caractéristique, ses maîtrises accordées, et surtout le
  cumul des emplacements de sorts, qui est la règle la plus facile à implémenter de travers ;
- les **compétences** et leur caractéristique associée — Acrobaties/DEX, Athlétisme/FOR… — que le
  [LOT-13](@ref lot-13) suppose sans jamais dire d'où elles viennent ;
- les **langues**, courantes et exotiques, avec leur écriture et leurs espèces typiques. Elles ne
  sont pas décoratives : chaque bloc de créature déclare ses `Langues`, chaque espèce en accorde, et
  le [LOT-15](@ref lot-15) doit pouvoir refuser un dialogue faute de langue commune.

*Acceptation* — un personnage multiclassé calcule ses emplacements de sorts conformément à la table
du multiclassage, vérifié sur trois combinaisons ; toute langue référencée par une créature ou une
espèce existe dans le catalogue.

### `LOT-44` — Noms, tables aléatoires et contenu d'ambiance

*Prérequis : `LOT-36`, `LOT-37`. Prérequis de `LOT-41`.*

Un bac à sable peuple des centaines de PNJ. Il lui faut donc des **noms**, et le corpus en fournit
par espèce et par culture : les *Basic Rules* listent noms masculins, féminins et de famille pour
chaque race, le *Player's Guide* fait de même pour les siennes (`Male Names: A'Kole, B'Abku…` chez
les taii'maku). Croisés avec la répartition d'espèces d'une région, ils donnent des habitants
plausibles plutôt qu'une suite de « Villageois 1 ».

S'y ajoutent les **tables aléatoires** déjà écrites : les traits, idéaux, liens et défauts en `d6`
de chaque historique — de quoi donner une personnalité à un PNJ sans en écrire une —, la table de
**babioles** (p. 59), et le contenu d'ambiance du chapitre 8 du Sourcebook : animaux de compagnie,
boissons, fêtes, nourriture, jeux de taverne, mesure du temps.

C'est le lot le moins spectaculaire de la filière et l'un des plus rentables : c'est lui qui fait la
différence entre un monde peuplé et une grille d'entités.

*Acceptation* — mille PNJ générés dans une région donnent une distribution d'espèces conforme à
celle déclarée par la région, et aucun doublon de nom complet en deçà d'un seuil annoncé ; la
génération est reproductible à graine égale.

### `LOT-45` — Guilde des Aventuriers : rangs, contrats et boucle de progression

*Prérequis : `LOT-41`, [LOT-16](@ref lot-16), [LOT-26](@ref lot-26).*

C'est la **colonne vertébrale du bac à sable**, et elle n'est pas à inventer : le Sourcebook la
décrit dans la République des Freelands. La Guilde enregistre les aventuriers, affiche les quêtes
déposées par les citoyens, et applique un **classement du fer au diamant** dont l'objet explicite est
d'« empêcher les aventuriers inexpérimentés de prendre des tâches au-dessus de leurs moyens ».

Chaque rang porte sa correspondance chiffrée :

| Rang | Niveau de personnage | FP des PNJ |
|---|---|---|
| Fer | 1 à 2 | ⅛ à ¼ |
| Cuivre | 3 à 4 | ½ à 1 |
| Bronze | 5 à 6 | 2 à 4 |
| Argent | 7 à 10 | 5 à 7 |
| Or et au-delà | 11 à 16 | 8 à 13 |

C'est exactement ce qui manque à un monde ouvert : un **tableau de quêtes** diégétique où déposer le
contenu généré par le `LOT-41`, une **échelle de difficulté** qui dit quelle créature opposer à quel
niveau, une **raison de progresser**, et un garde-fou qui évite au joueur de niveau 2 de se faire
étriller dans une région d'Argent sans avertissement.

Les halls de guilde étant « bâtis jusqu'au-delà des Freelands », le système s'étend naturellement aux
dix régions et donne au `LOT-42` des points d'ancrage pour le voyage.

*Acceptation* — un contrat n'est proposé qu'au rang correspondant ; monter de rang change
l'offre de contrats de façon observable ; la difficulté d'un contrat généré respecte la bande de FP
de son rang, vérifié sur les cinq rangs.

### `LOT-46` — Créatures de Tanares

*Prérequis : `LOT-33`, `LOT-31`.*

Les **82 blocs de Tanares** (ch. 9, 17 familles : akhu, emoguns, gloomfolk, ironhands, kemets,
kepesh, kikoku, dragons et drakes pénombraux, ninjas spectraux, constructs taii'makian…). Gabarit
analogue aux 94 du `LOT-33`, mais **en anglais** : c'est l'étape de traduction, via le lexique du
`LOT-31`, qui justifie un lot séparé plutôt qu'un pipeline supplémentaire dans le précédent.

Avec les 94 du SRD, cela porte le bestiaire à **176 créatures** — et c'est le total retenu. Les 416
blocs du *Manuel des Monstres* sont **écartés du périmètre** (voir §8) : 176 profils suffisent
largement à peupler dix régions, et ils épargnent le seul gisement du corpus dont chaque valeur
numérique aurait dû être relue.

Ce lot est donc **borné et achevable**, contrairement à ce qu'il était quand il portait le *Manuel
des Monstres* : 17 familles, une fin, un critère de fin.

*Acceptation* — les 82 profils chargent et sont jouables en combat, pas seulement chargés ; chaque
terme de règle traduit est conforme au lexique du `LOT-31` ; dix profils sont vérifiés à la main
contre le PDF.

### `LOT-47` — Le roster complet de classes

*Prérequis : `LOT-36`, `LOT-43`, `LOT-49`.*

Les classes définitives, celles qui remplacent l'échafaudage du `LOT-36`. Trois apports :

- les **12 classes de base** — 4 des *Basic Rules* (clerc, guerrier, magicien, roublard) et 8 du
  *Manuel des Joueurs* (barbare, barde, druide, moine, paladin, rôdeur, ensorceleur, sorcier) ;
- les **4 classes inédites** de Tanares (dragonblade, élémentaliste, madwalker, redeemer) ;
- la **vingtaine de sous-classes** du *Player's Guide*, qui se greffent sur les 12 précédentes — et
  qui sont la raison pour laquelle le *Manuel des Joueurs* est indispensable : sans druide, pas de
  Cercle de la Floraison.

Les **31 tables de progression** sont le travail réel : chacune est un tableau de 20 lignes où un
décalage d'une ligne donne à une classe les capacités du niveau voisin — faux, et faux sans rien
casser. Le §4 le démontre sur la table du barbare : illisible en `-layout`, exacte en `-table`.
L'extraction est donc faisable, mais le mode d'extraction n'est pas négociable, et la sortie passe
au contrôle du `LOT-49` parce que les valeurs viennent d'un scan.

**Le lot ne se clôt pas sur la livraison** mais sur le retrait : les quatre classes provisoires du
`LOT-36` sont supprimées, et rien ne doit s'en apercevoir.

*Acceptation* — une table de progression extraite est comparée ligne à ligne au PDF sur au moins
trois classes ; toute classe exigeant un mécanisme absent du moteur est listée au chargement ; les
fichiers marqués `"statut": "provisoire"` ont disparu et aucune référence ne pend.

### `LOT-48` — Portraits et illustrations

*Prérequis : `LOT-39`.*

La part semi-automatique de l'extraction d'images : portraits d'espèces, illustrations de classes,
créatures, PNJ notables, et les cartes de régions comme référence de tracé. L'outil propose les
régions candidates triées par taille, l'humain garde et recadre — parce que l'art court sous les
colonnes et qu'un rendu clippé embarque le texte posé dessus (§4).

Volume long, sans jalon, et sans risque : chaque image livrée remplace un marqueur généré du
`LOT-39`, jamais une absence.

*Acceptation* — chaque image livrée est référencée par une clé du manifeste ; le compte de clés
encore servies par un marqueur décroît, et la CI le publie.

### `LOT-49` — Contrôle de cohérence du contenu

*Prérequis : `LOT-33`, `LOT-34`. Alimente `LOT-47`.*

Le `LOT-32` valide la **structure** : un fichier bien formé, des champs présents, des énumérations
connues. Il ne dit rien de la **plausibilité**. Un loup à CA 47, une épée à 3 pièces d'or au lieu de
30, une créature de FP ⅛ avec 90 points de vie : tout cela franchit un schéma sans broncher.

Écarter le *Manuel des Monstres* retire le pire des risques — l'OCR bruité — mais **pas le risque
lui-même**, qui vient des tableaux et qui est démontré au §4 : sur la table des armes, `-layout`
attribue le poids et le prix à l'arme de la ligne suivante. Les 31 tables de progression du `LOT-47`
et les tables d'équipement du `LOT-34` courent exactement ce danger, et une valeur décalée d'une
ligne ne lève aucune alerte.

Le lot ajoute donc un contrôle **statistique** plutôt que syntaxique : pour chaque famille, les
bornes attendues d'une valeur au regard des autres champs — PV cohérents avec le dé de vie et la
Constitution, bonus d'attaque cohérent avec la caractéristique et le facteur de puissance, prix
cohérent avec la rareté. Ce qui sort des bornes est **signalé, pas rejeté** : une créature
volontairement hors norme existe, un OCR raté aussi, et seul un humain les distingue.

*Acceptation* — une valeur sciemment corrompue dans un profil de test est détectée ; le rapport
distingue l'anomalie confirmée de l'anomalie acceptée, et cette acceptation est **enregistrée dans
la donnée** pour ne pas être re-signalée à chaque exécution.

---

## 6. Ordre d'exécution recommandé

Les numéros suivent [LOT-29](@ref lot-29), l'exécution non. Les lots de données doivent précéder les
lots qui les consomment, sans quoi ces derniers se construisent sur des catalogues fictifs — et un
catalogue fictif finit toujours par se figer en valeurs codées en dur, exactement ce que
[`EX-VIS-007`](@ref EX-VIS-007) interdit.

Ces vingt lots ne forment **pas une phase** qui suivrait le [LOT-29](@ref lot-29) : ils s'entrelacent
avec les phases B à E, parce que chacun sert un lot existant qui, sans lui, se construirait sur un
catalogue fictif — et un catalogue fictif finit toujours par se figer en valeurs codées en dur,
exactement ce que [`EX-VIS-007`](@ref EX-VIS-007) interdit. D'où « filière » plutôt que « phase ».

| Quand | Lots | Pourquoi là |
|---|---|---|
| Tout de suite après [LOT-08](@ref lot-08) | `LOT-30`, `LOT-31`, `LOT-32` | Outillage et contrats, sans dépendance ; le plus tôt est le mieux |
| Avec [LOT-09](@ref lot-09) | `LOT-37` | L'atlas donne au graphe de cartes de vrais nœuds à relier |
| Avant [LOT-13](@ref lot-13) | `LOT-36`, `LOT-43` | La fiche a besoin d'espèces, de compétences et de langues, et le premier combat des 4 classes simplifiées |
| Avant [LOT-14](@ref lot-14) | `LOT-34`, puis `LOT-49` | Catalogue réel, puis contrôle de ses valeurs |
| Avant [LOT-21](@ref lot-21) / [LOT-23](@ref lot-23) | `LOT-33` | Attaques et IA ont besoin de vraies créatures |
| Avant [LOT-25](@ref lot-25) | `LOT-35` | Les sorts sont des données avant d'être un système |
| Après [LOT-13](@ref lot-13) | `LOT-38`, puis `LOT-39` | La maquette suppose la fiche ; la plomberie d'assets suit |
| Après [LOT-11](@ref lot-11) | `LOT-40` | La génération produit des niveaux que l'éditeur doit savoir rouvrir |
| **Après [LOT-27](@ref lot-27)** | `LOT-44`, `LOT-41`, `LOT-45`, `LOT-42` | Le bac à sable généralise une boucle ; il faut l'avoir validée une fois |
| Avant `LOT-41` | `LOT-46` | Peupler dix régions demande plus que les 94 bêtes du SRD |
| Avant [LOT-25](@ref lot-25), après [LOT-21](@ref lot-21) | `LOT-47` | Le roster définitif remplace l'échafaudage une fois le combat éprouvé |
| En fond, sans jalon | `LOT-48` | Volume long, sans blocage : se remplit par lots successifs |

Trois lignes méritent qu'on s'y arrête.

**La dernière** : 6 000 images ne se traitent pas d'un bloc, et rien n'y oblige — le `LOT-39`
affiche un marqueur pour tout ce qui n'a pas encore d'image, et chaque illustration livrée en
remplace un.

Deux lots ont d'ailleurs **quitté** cette catégorie au fil des décisions, et c'est bon signe : le
`LOT-46` depuis que le *Manuel des Monstres* est hors périmètre — 82 créatures, cela se termine — et
le `LOT-47`, qui a désormais une échéance nette puisqu'il doit retirer l'échafaudage du `LOT-36`. Un
lot sans date de fin est un lot qu'on ne finit pas.

**L'avant-dernière** : les quatre lots qui font le bac à sable sont les seuls que je placerais
résolument **après** le [LOT-27](@ref lot-27). Peupler dix régions revient à appliquer cent fois la
même recette ; si la recette est mauvaise — rencontres mal dosées, marchands inutiles, quêtes vides
— on la découvre cent fois. Le `LOT-27` coûte un village et un donjon, et c'est le prix pour ne pas
payer cette erreur au centuple. Leur ordre interne compte aussi : `LOT-44` (les noms) avant
`LOT-41` (le peuplement), sinon on peuple avec des « Villageois 1 » ; `LOT-45` (la Guilde) après,
parce qu'elle a besoin de quêtes à afficher.

**Et une précondition hors tableau** : la filière référence une famille d'exigences `EX-CNT-*` qui
n'existe pas. Vingt lots qui pointent vers un document absent, c'est une dette qui grossit à chaque
lot livré. Écrire `Documentation/Specification/contenu.md` **avant** le `LOT-30` coûte une
demi-journée et l'éteint.

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
  creatures/*.json          ← 94 betes (LOT-33), puis 82 de Tanares (LOT-46)
  items/*.json              ← équipement, monnaie, objets magiques (LOT-34)
  spells/, conditions/      ← sorts et états (LOT-35)
  species/, backgrounds/    ← especes et historiques (LOT-36)
  classes/*.json            ← socle (LOT-36), puis Tanares et progressions (LOT-47)
  feats/, skills/, languages/        ← options de personnage (LOT-43)
  tables/*.json             ← noms, personnalite, babioles (LOT-44)
  guild/*.json              ← rangs et gabarits de contrat (LOT-45)

Source/Elements/World/
  regions/*.json            ← 10 regions : statistiques, especes, factions, terrain (LOT-37)
  locations/*.json          ← lieux nommes, avec leurs effets mecaniques (LOT-37)
  factions/, pantheon/, organizations/            ← (LOT-37)
  populate/*.json           ← gabarits de peuplement et de quete (LOT-41)
Source/Elements/Assets/
  rpg.assets.json           ← manifeste des clés d'assets (LOT-39)
  Ui/, Portraits/, Creatures/        ← textures extraites (LOT-39)
Source/Elements/Localization/rpg.glossary.csv   ← lexique (LOT-31)

Source/Core/World/
  TerrainGenerator.{h,cpp}  ← generation pilotee par descripteur de region (LOT-40)
  RegionPopulator.{h,cpp}   ← peuplement systemique (LOT-41)
  WorldMap.{h,cpp}          ← voyage et decouverte (LOT-42)
```

---

## 8. Décisions prises et questions restantes

### Tranché

- **Périmètre : l'univers complet.** Les 10 régions et leurs lieux, les 13 espèces, les 8 classes et
  leurs sous-classes, les historiques, les factions, le panthéon, le bestiaire entier. Pas de socle
  réduit.
- **Les 10 régions sont jouables**, pas seulement importées — d'où la génération de terrain du
  `LOT-40`, qui seule rend une centaine de cartes atteignable.
- **Le jeu est un bac à sable** dans un premier temps : contenu systémique déduit des données de
  région, pas d'intrigue directrice.
- **Licences en sommeil** (§3) : projet privé, dépôt privé, aucune contrainte d'usage.

- **Bestiaire arrêté à 176 créatures** : les 94 bêtes du SRD (`LOT-33`) et les 82 de Tanares
  (`LOT-46`). Le *Manuel des Monstres* est **hors périmètre** — 416 blocs dont chaque valeur
  numérique demanderait un contrôle, pour un bestiaire déjà largement suffisant à un monde ouvert
  de dix régions. Rien n'est fermé : le `LOT-30` sait le lire, il pourra être repris plus tard.
- **Le *Manuel des Joueurs* est dans le périmètre**, lui, et il est indispensable : il porte les
  5 races et 8 classes que les *Basic Rules* n'ont pas, donc les classes de base sur lesquelles se
  greffent les sous-classes de Tanares. Son OCR se traite (§4).
- **Les 4 classes simplifiées de Tanares sont un échafaudage.** Elles servent de socle au premier
  modèle de combat (`LOT-36`) et **seront retirées** au profit des 12 classes complètes
  (`LOT-47`). Elles portent pour cela un `"statut": "provisoire"` en donnée, et le `LOT-47` ne se
  clôt que sur leur suppression effective.
- **Région de départ : la République des Freelands.** Ses propres statistiques l'argumentent :
  `Monster Presence` haute — les rencontres se justifient d'elles-mêmes, ce que le Central Empire,
  noté *Low*, ne permet pas ; économie fondée sur les mercenaires et les parts de monstres — le
  marchand et le donneur de quête n'ont rien à inventer ; villes de frontière — le village à
  construire existe dans la fiction ; faction Allied Forces, cadre héroïque plutôt qu'oppressif. Et
  surtout : c'est là que siège la **Guilde des Aventuriers**, donc la boucle du `LOT-45`.

### À trancher

- **Écrire `contenu.md` d'abord.** La famille `EX-CNT-*` n'existe pas et vingt lots la référencent.
  C'est une précondition, pas une question ouverte — mais elle demande une décision : quelles
  exigences la filière contenu porte-t-elle exactement ?
- **Profondeur du bac à sable.** Le `LOT-41` compose des quêtes par gabarit et le `LOT-45` les
  distribue par rang. Faut-il en rester à ce contenu déduit, ou écrire par-dessus quelques quêtes à
  la main dans les lieux notables ? La réponse change le poids du [LOT-16](@ref lot-16), pas
  l'architecture.
*(Le sort du *Manuel des Monstres* est tranché : voir ci-dessus.)*
