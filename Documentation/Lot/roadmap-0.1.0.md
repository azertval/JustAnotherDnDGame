# Feuille de route 0.1.0 {#roadmap-010}

Le programme complet de la version `0.1.0`, et **l'unique source de vérité** des lots à venir : les
lots `LOT-08` à `LOT-84`, ce que le corpus `Documentation/SourceBook/` permet d'en tirer, et l'audit
qui a confronté le tout à l'état réel du dépôt.

Les sept lots **livrés** (`LOT-01` à `LOT-07`) gardent leur dossier et leur `epic.md` : ils portent
ce que leur réalisation a tranché, qui est de l'histoire, pas du programme. Tout le reste vit ici,
et un dossier se crée **au démarrage** d'un lot.

`Documentation/SourceBook/` rassemble le **matériel de référence** ayant servi à construire le monde
et les règles : huit PDF, environ 1 200 pages, 280 Mo. Les sections 1 à 4 disent ce qu'ils
contiennent et ce que l'extraction a appris ; la section 5 découpe le travail et la section 6 en
donne l'ordre d'exécution, chemin critique et graphe compris ; la section 7 dessine l'arborescence
cible et la section 8 récapitule ce qui est tranché ; les sections 9 et 10 rapportent l'audit et ce
qu'il faut anticiper ; la section 11 porte les lots `LOT-08` à `LOT-29`.

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

Cinquante et un lots, `LOT-30` à `LOT-84`, quatre numéros ayant été **retirés** par fusion
(`LOT-31`, `LOT-48`, `LOT-71`, `LOT-73` : voir l'encart en fin de section). Les numéros sont, comme
toujours, des identifiants stables : ils viennent après [LOT-29](@ref lot-29) dans la numérotation,
mais plusieurs s'exécutent **avant** les lots qui les consomment (voir §6). Une nouvelle famille d'exigences `EX-CNT-*` les couvre, à écrire
dans un `Documentation/Specification/contenu.md`.

> **État de l'audit.** Cette page a été confrontée à l'état réel du dépôt, puis à elle-même. Six
> lots dépassaient l'étalon mesuré sur les **sept lots livrés** — `LOT-01` à `LOT-07`, soit 16 à 34
> fichiers et 450 à 1340 lignes, le `LOT-04` marquant la borne haute à 19 fichiers pour 1337 lignes.
> Ils ont été **découpés** : `LOT-37`/`LOT-80`, `LOT-40`/`LOT-81`, `LOT-41`/`LOT-82`,
> `LOT-45`/`LOT-83`, `LOT-47`/`LOT-84`, et le `LOT-69` réduit à la suppression qu'il est réellement.
> Le principe de coupe est le même partout : **le code d'un côté, la donnée de l'autre**, chaque
> moitié ayant son consommateur et son critère d'acceptation.
>
> Les epics réels font 33 à 89 lignes ; les sections ci-dessous en font environ 27, donc
> **sous-spécifiées comme epics** — elles seront reprises au démarrage de chaque lot. Aucune ne
> porte encore de rubrique « Exigences couvertes », faute de familles `EX-*` existantes : c'est
> l'objet du `LOT-77`.

Trois formes de sortie, et le choix entre elles n'est pas cosmétique. **JSON** pour tout ce qui est
structuré et imbriqué — une créature a des actions, une classe a une progression par niveau — et
parce que le projet lit déjà du JSON (`skins.json`, `sounds.json`, `palettes.json`). **CSV** pour la
seule donnée réellement tabulaire et plate du corpus, le lexique du `LOT-30` : 1 200 lignes de trois
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

### `LOT-30` — Chaîne d'extraction du corpus et lexique bilingue {#lot-30}

*Prérequis : `LOT-77`. Prérequis de tous les autres lots de la filière.*

> **Fusionné à l'audit.** Ce lot a absorbé l'ancien `LOT-31` (lexique bilingue). Celui-ci livrait
> deux fichiers — un CSV et un script — soit un ordre de grandeur sous l'étalon, et le lexique est
> la **première sortie** de la chaîne d'extraction : il la valide autant qu'il en dépend. Le numéro
> `LOT-31` est retiré.

L'outillage, puis sa première sortie. `scripts/sourcebook/`, sur **PyMuPDF** : extraction de texte, de
tableaux par coordonnées, d'images par rendu clippé ; gestion de la pagination en double page ;
cache disque. Un manifeste `scripts/sourcebook/corpus.toml` enregistre par document son empreinte
SHA-256, son nombre de pages, son décalage de pagination et sa **provenance** (`srd`, `tanares`,
`phb-fr`) — celle-là même que chaque donnée produite reportera (§3).

Le manifeste est versionné ; le corpus intermédiaire ne l'est pas, et la CI ne l'exécute pas : les
PDF ne sont pas sur le runner. Seules les **données produites** sont validées en CI (`LOT-32`).

**Puis le lexique, qui éprouve la chaîne sur le gisement le plus simple.** Les ~1 200 paires du
`Glossaire.pdf` deviennent `Source/Elements/Localization/rpg.glossary.csv`
(`anglais;français;catégorie`), complétées par le vocabulaire des *Basic Rules* (états, écoles de
magie, types de dégâts, propriétés d'armes).

L'intérêt n'est pas de traduire l'interface — `fr.lang` et `en.lang` font déjà cela. Il est
d'imposer **une seule traduction par terme de règle dans tout le jeu**. Sans table d'autorité,
*saving throw* devient « jet de sauvegarde » dans la fiche, « JdS » dans le journal de combat et
« sauvegarde » dans l'infobulle, et le joueur croit à trois mécaniques différentes. Le lexique sert
en outre à **traduire les données anglaises** du Sourcebook et du *Player's Guide* de façon
cohérente avec celles venues du français.

*Acceptation* — deux exécutions successives produisent des empreintes identiques ; une empreinte
qui ne correspond plus au PDF présent fait échouer l'extraction plutôt que de produire des données
silencieusement décalées ; `scripts/check_glossary.py` échoue si une clé de règle de `fr.lang`
emploie un terme absent du lexique ou en contredit la traduction.

### `LOT-32` — Schémas de données RPG {#lot-32}

*Prérequis : `LOT-30`, `LOT-79`. Prérequis de `LOT-33` à `LOT-37`, `LOT-43`, `LOT-84`.*

Le contrat avant les données. Un JSON Schema par famille — créature, objet, arme, armure, sort,
espèce, classe, historique, état, type de dégâts — sous `Source/Elements/Rpg/schema/`, et
`scripts/check_rpg_data.py` qui valide tout `Source/Elements/Rpg/**.json` en CI. Le champ
`"source"` du §3 y est **obligatoire** : c'est une ligne de schéma, et elle rend la question de la
provenance définitivement réglée.

Un test vérifie en plus que les énumérations C++ (`DamageType`, `Condition`…) et celles des schémas
**coïncident**. C'est le point où une donnée et un moteur divergent en silence : le JSON déclare
`"psychique"`, le C++ ne connaît que `Psychic`, la valeur tombe dans le cas par défaut, et le sort
ne fait plus de dégâts sans que rien ne l'annonce.

Les schémas se branchent sur la **brique de chargement** du `LOT-79` plutôt que sur une septième
réimplémentation de `loadFromFile` : ce lot écrit des contrats, il n'écrit pas de lecteur.

*Acceptation* — une donnée invalide fait échouer la CI avec le fichier et la ligne fautifs ; ajouter
une valeur d'énumération d'un seul côté fait échouer un test ; aucun catalogue RPG ne porte sa
propre routine de lecture.

### `LOT-33` — Bestiaire de base {#lot-33}

*Prérequis : `LOT-32`. Alimente [LOT-13](@ref lot-13), [LOT-21](@ref lot-21),
[LOT-23](@ref lot-23), [LOT-27](@ref lot-27).*

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

### `LOT-34` — Équipement, monnaie, objets magiques {#lot-34}

*Prérequis : `LOT-32`. Alimente [LOT-14](@ref lot-14), [LOT-26](@ref lot-26),
[LOT-27](@ref lot-27).*

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

### `LOT-35` — Sorts et états {#lot-35}

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

### `LOT-36` — Espèces, historiques et classes provisoires {#lot-36}

*Prérequis : `LOT-32`, `LOT-43`. Alimente [LOT-13](@ref lot-13), [LOT-27](@ref lot-27).*

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
> final, remplacées par les **seize** classes complètes que livrent le `LOT-47` (le socle et le
> guerrier) puis les `LOT-51` à `LOT-65` (une classe par lot). Une donnée provisoire non marquée
> devient permanente par accident — c'est la façon la plus banale dont un échafaudage finit en mur
> porteur. D'où un champ `"statut": "provisoire"` au schéma, la CI qui **liste** ce qui le porte, et
> un critère de retrait écrit d'avance : le jour où les seize classes sont livrées, supprimer ces
> quatre fichiers ne doit casser aucune référence.

Les seize classes complètes relèvent du `LOT-47` et des `LOT-51`→`LOT-65`, leurs 31 tables de
progression du `LOT-84` : une espèce se décrit en une douzaine de champs, une classe en un tableau
de 20 lignes à relire ligne à ligne. Mélanger les deux ferait dépendre l'arrivée des espèces d'un
travail dix fois plus long.

Chaque classe et chaque espèce déclare les **mécanismes** qu'elle exige (emplacements de sorts,
ressource propre, liste de sorts dédiée, choix de sous-classe au niveau *n*). C'est cette
déclaration qui permet au moteur de dire ce qu'il ne sait pas encore jouer, au lieu de servir une
classe amputée.

*Acceptation* — celle du [LOT-13](@ref lot-13), tenue par les données : trois classes chargent et
donnent les bons modificateurs, et la progression du niveau 1 au niveau 5 ne fait intervenir aucune
valeur codée en C++. Toute espèce ou classe exigeant un mécanisme absent du moteur est **listée au
chargement**, jamais jouée en silence. Les quatre classes provisoires portent leur marque, et un
test vérifie qu'aucune donnée **définitive** ne les référence.

### `LOT-37` — Atlas des régions et graphe de cartes {#lot-37}

*Prérequis : `LOT-32`. Alimente [LOT-09](@ref lot-09), [LOT-27](@ref lot-27), `LOT-80`, `LOT-81`.*

> **Découpé à l'audit.** Ce lot portait l'atlas entier — régions, factions, panthéon, organisations,
> chronologie, économie — soit largement au-delà de l'étalon. Il ne garde que la **géographie
> politique** : les régions, leurs lieux et leurs statistiques. Les entités transverses partent au
> `LOT-80`, qui sert un autre consommateur (le `LOT-16`, quêtes) à un autre moment.

Les **10 régions** de Tanares et leurs lieux deviennent l'atlas du jeu, vers
`Source/Elements/World/regions/` et `locations/`.

Leur vertu est d'être déjà **cohérentes** : une région porte des lieux, un lieu porte des factions
présentes et des PNJ notables. C'est précisément ce qu'un monde inventé au fil de l'eau n'a jamais.

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
seulement en données. C'est ce qui rend les `LOT-81`, `LOT-40` et `LOT-41` nécessaires : à raison
d'une dizaine de lieux nommés par région, on parle d'une **centaine de cartes**, un volume qui ne se
dessine pas à la main et qui doit donc se **générer depuis cet atlas**. Le présent lot est ce qui
rend cette génération possible : c'est lui qui fournit la matière que le `LOT-81` traduira en
descripteurs de terrain.

*Acceptation* — `check_world_graph.py` (prévu au [LOT-27](@ref lot-27)) valide l'atlas ; aucun lieu
sans région, aucune faction référencée qui n'existe pas, aucune région inatteignable. Les sept
statistiques régionales sont typées, pas laissées en texte libre.

### `LOT-38` — Fiche de personnage : maquette et interface {#lot-38}

*Prérequis : [LOT-13](@ref lot-13), `LOT-68`, `LOT-39`, `LOT-76`. Alimente
[LOT-24](@ref lot-24).*

`Character_Sheets_Tanares.pdf` fournit ses 5 planches en image à environ 300 ppp (2 668 × 3 418) :
une maquette précise de l'écran de fiche — quels champs, groupés en quels blocs (identité,
caractéristiques, compétences, combat, équipement, sorts), dans quel ordre de lecture. L'écran se
construit sur le système de design existant, avec les panneaux de parchemin extraits au `LOT-39`.

*Acceptation* — chaque champ de la maquette est soit affiché à l'écran, soit inscrit dans une liste
explicite de champs hors périmètre. Un champ simplement oublié n'est pas un arbitrage.

### `LOT-39` — Plomberie des clés d'assets {#lot-39}

*Prérequis : `LOT-33`, `LOT-34`, `LOT-37`. Alimente [LOT-11](@ref lot-11),
[LOT-27](@ref lot-27), `LOT-38`.*

> **Scindé à l'audit.** Ce lot portait aussi l'extraction de l'habillage d'interface, ce qui faisait
> dépendre la charte visuelle (`LOT-66`) de tout le pipeline de données. Or l'habillage n'a besoin
> que des PDF, disponibles aujourd'hui : il part au `LOT-76` et peut démarrer immédiatement. Ne
> reste ici que la plomberie, qui a réellement besoin des catalogues.

La plomberie : une **clé d'asset** portée par chaque donnée (`"asset": "beast/wolf"`), jamais un
chemin de fichier — un chemin dans une donnée de règle lie le catalogue à l'arborescence du disque,
et tout déplacement de dossier casse alors des créatures. Un **manifeste** dérivé des catalogues
énumère les clés attendues, leur famille et leur taille. Et toute clé sans image obtient un
**marqueur généré** sur l'`ProceduralAtlas` existant, de sorte que le jeu tourne complet — 176
créatures affichables — avant que la moindre découpe ne soit faite. La production graphique devient
un remplacement progressif de marqueurs, jamais un préalable bloquant.

La distinction avec le [LOT-08](@ref lot-08) est nette : le `LOT-08` a fixé le vocabulaire des
**tuiles de terrain**, celui-ci fixe celui des **entités**.

*Acceptation* — aucune clé d'asset orpheline dans les catalogues ; toute clé sans image obtient un
marqueur généré, et la CI **liste** les clés sans art définitif sans échouer : c'est un état
d'avancement, pas un défaut.

> **Le `LOT-48` a été dissous ici.** Il portait « portraits et illustrations », explicitement
> « volume long, sans jalon ». Or cette page écrit par ailleurs qu'« un lot sans date de fin est un
> lot qu'on ne finit pas » : en garder un contredisait sa propre règle. L'extraction
> semi-automatique d'images devient donc une **obligation de chaque lot de catalogue** — le `LOT-33`
> livre ses bêtes avec leurs illustrations, le `LOT-46` ses créatures de Tanares, le `LOT-36` ses
> portraits d'espèces — et le marqueur généré ci-dessus couvre ce qui n'est pas encore fait. Le
> numéro `LOT-48` est retiré.

### `LOT-40` — Générateur de terrain {#lot-40}

*Prérequis : `LOT-81`, [LOT-11](@ref lot-11). Alimente `LOT-41`.*

> **Découpé à l'audit.** Ce lot mêlait un **générateur** (du C++ déterministe, testable sans aucune
> donnée de région) et les **descripteurs des dix régions** (de la donnée extraite et relue). Deux
> métiers, deux critères d'acceptation : les descripteurs partent au `LOT-81`. Ne reste ici que le
> moteur, qui se valide sur un descripteur de test.

Dix régions, une dizaine de lieux nommés chacune : environ **cent cartes**. Dessinées à la main dans
l'éditeur, à raison de quelques heures pièce, c'est une année de travail pour un développeur seul —
et la certitude que le monde ne sera jamais fini. Elles doivent donc se **générer**.

La génération n'est pas un pis-aller ici, parce que la matière existe déjà. Le `LOT-81` livre pour
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
une carte générée s'ouvre dans l'éditeur, se modifie et se recharge sans perte. Le générateur se
teste sur un descripteur **de fixture**, sans dépendre des dix régions réelles.

### `LOT-41` — Peuplement : rencontres et créatures {#lot-41}

*Prérequis : `LOT-40`, `LOT-33`, `LOT-46`, `LOT-44`, [LOT-27](@ref lot-27).
Alimente `LOT-82`.*

> **Découpé à l'audit.** Ce lot dérivait **deux peuplements sans rapport** des mêmes statistiques :
> le danger (créatures, rencontres, embuscades) et le civil (PNJ, marchands, prix, quêtes). Deux
> dérivations indépendantes, deux jeux de données d'entrée, deux mesures d'acceptation. Le civil
> part au `LOT-82`.

Une carte vide n'est pas un lieu. Ce lot y met ce qui est **hostile**, et il le fait **par déduction
depuis les données de région**, jamais par placement manuel — c'est ce qui rend dix régions tenables.

Trois des sept `Regional Statistics` pilotent ce versant :

| Statistique | Ce qu'elle pilote |
|---|---|
| `Monster Presence` | Densité des rencontres, dangerosité des tables de la région |
| `Crime and Violence` | Embuscades sur les routes, fréquence des rencontres hostiles |
| `Political Stability` | Présence de gardes, donc de rencontres qui tournent mal |

Le bestiaire des 176 créatures y est filtré par région : une table de rencontre se compose de ce qui
vit là, à la dangerosité que la région annonce, et non d'un tirage uniforme dans le catalogue.

*Acceptation* — deux régions aux statistiques opposées produisent des **densités et des tables de
rencontre mesurablement différentes**, vérifié par un test ; le peuplement est reproductible à
graine égale ; aucune rencontre n'est placée en dur dans le code ; aucune créature n'apparaît dans
une région dont le biome ne la porte pas.

### `LOT-42` — Voyage et carte du monde {#lot-42}

*Prérequis : `LOT-82`, `LOT-68`, `LOT-70`, [LOT-09](@ref lot-09), [LOT-17](@ref lot-17).*

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

### `LOT-43` — Options de personnage : dons, multiclassage, compétences, langues {#lot-43}

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

### `LOT-44` — Noms, tables aléatoires et contenu d'ambiance {#lot-44}

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

### `LOT-45` — Guilde des Aventuriers : rangs et contrats {#lot-45}

*Prérequis : `LOT-82`, `LOT-68`, [LOT-16](@ref lot-16), [LOT-26](@ref lot-26).
Alimente `LOT-83`.*

> **Découpé à l'audit.** Ce lot portait à la fois le **catalogue** (rangs, bandes de facteur de
> puissance, gabarits de contrat, tableau d'affichage) et la **boucle de progression** (monter de
> rang, ce que cela change, ce que cela débloque). Le premier est de la donnée et un écran ; le
> second est une mécanique qui suppose l'expérience du `LOT-74`. La boucle part au `LOT-83`.

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

*Acceptation* — un contrat n'est proposé qu'au rang correspondant ; la difficulté d'un contrat
généré respecte la bande de FP de son rang, vérifié sur les cinq rangs ; le tableau de quêtes
affiche ce que le `LOT-82` a composé, sans qu'aucun contrat ne soit écrit à la main.

### `LOT-46` — Créatures de Tanares {#lot-46}

*Prérequis : `LOT-33`, `LOT-30`.*

Les **82 blocs de Tanares** (ch. 9, 17 familles : akhu, emoguns, gloomfolk, ironhands, kemets,
kepesh, kikoku, dragons et drakes pénombraux, ninjas spectraux, constructs taii'makian…). Gabarit
analogue aux 94 du `LOT-33`, mais **en anglais** : c'est l'étape de traduction, via le lexique du
`LOT-30`, qui justifie un lot séparé plutôt qu'un pipeline supplémentaire dans le précédent.

Avec les 94 du SRD, cela porte le bestiaire à **176 créatures** — et c'est le total retenu. Les 416
blocs du *Manuel des Monstres* sont **écartés du périmètre** (voir §8) : 176 profils suffisent
largement à peupler dix régions, et ils épargnent le seul gisement du corpus dont chaque valeur
numérique aurait dû être relue.

Ce lot est donc **borné et achevable**, contrairement à ce qu'il était quand il portait le *Manuel
des Monstres* : 17 familles, une fin, un critère de fin.

*Acceptation* — les 82 profils chargent et sont jouables en combat, pas seulement chargés ; chaque
terme de règle traduit est conforme au lexique du `LOT-30` ; dix profils sont vérifiés à la main
contre le PDF.

### `LOT-47` — Socle de classe, et le guerrier comme preuve {#lot-47}

*Prérequis : `LOT-36`, `LOT-43`, `LOT-84`.*

> **Découpé à l'audit, et allégé d'une dépendance.** Ce lot portait aussi l'**extraction des 31
> tables de progression** — de la donnée, relue ligne à ligne, qui appartient à la filière contenu
> et n'a aucune raison d'attendre le socle : elle part au `LOT-84`. Et il déclarait le `LOT-49` en
> prérequis, ce qui faisait attendre au socle de classe un contrôleur de cohérence numérique qui
> attend lui-même le catalogue d'objets. Le lien est passé en « alimente » : le `LOT-49` contrôle
> les tables quand elles arrivent, il ne les précède pas.

Non pas les seize classes, mais **ce qui leur est commun** — et une seule classe pour le démontrer.

Grouper seize classes dans un lot serait cacher une phase entière derrière un numéro. Une classe
n'apporte pas des données, elle apporte une **mécanique** : la rage n'est pas la magie de pacte, la
forme sauvage n'est pas l'attaque sournoise. Chacune se code, se teste et se règle séparément. D'où
le découpage en `LOT-51` à `LOT-65`, **une classe par lot**.

Ce lot-ci pose donc le contrat commun : ce qu'une classe **déclare** (dé de vie, maîtrises,
caractéristiques de sauvegarde, table de progression sur 20 niveaux, niveau de choix de sous-classe,
ressources propres), comment une **ressource de classe** se décrit génériquement — quantité, cadence
de récupération (repos court, repos long, à volonté), et ce qu'elle alimente — et comment une
**sous-classe** se greffe sans dupliquer sa classe mère.

Il le prouve sur le **guerrier**, la classe la plus simple : pas d'incantation, deux ressources
seulement (Second souffle, Fougue), et l'Attaque supplémentaire qui suffit à valider la boucle
d'attaque du [LOT-21](@ref lot-21). Si le socle ne tient pas pour le guerrier, il ne tiendra pour
personne.

Les **31 tables de progression** que le `LOT-84` a extraites sont ici **consommées** : le socle les
lit, il ne les produit pas.

*Acceptation* — le guerrier est jouable du niveau 1 au niveau 5, ses deux ressources se consomment
et se récupèrent au bon repos ; ajouter une classe ne demande de toucher à **aucun** fichier C++
existant hors l'ajout de sa mécanique propre.

### `LOT-49` — Contrôle de cohérence du contenu {#lot-49}

*Prérequis : `LOT-33`, `LOT-34`.*

Le `LOT-32` valide la **structure** : un fichier bien formé, des champs présents, des énumérations
connues. Il ne dit rien de la **plausibilité**. Un loup à CA 47, une épée à 3 pièces d'or au lieu de
30, une créature de FP ⅛ avec 90 points de vie : tout cela franchit un schéma sans broncher.

Écarter le *Manuel des Monstres* retire le pire des risques — l'OCR bruité — mais **pas le risque
lui-même**, qui vient des tableaux et qui est démontré au §4 : sur la table des armes, `-layout`
attribue le poids et le prix à l'arme de la ligne suivante. Les 31 tables de progression du `LOT-84`
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
### `LOT-50` — Le Colisée : bac à sable de combat {#lot-50}

*Prérequis : [LOT-19](@ref lot-19), [LOT-20](@ref lot-20), [LOT-21](@ref lot-21), `LOT-47`.
Prérequis de `LOT-51` à `LOT-65`.*

Un lieu pour éprouver le combat, encore et encore, sans monter une partie complète à chaque essai.
Et le corpus en fournit un qui n'est pas un outil de développement déguisé : **les Arènes de
Tanares**, mentionnées cent fois dans le Sourcebook, sont une institution centrale du monde.

Ce qu'elles apportent est presque taillé pour l'usage :

- **Le combat y est non létal.** « Les Arènes opposent deux camps d'un conflit, représentés par
  leurs Héros, pour résoudre les impasses sans recourir à la guerre ni aux morts. » La sécurité des
  combattants tient à un rituel de **Marque Héroïque** de l'Église évolutionniste. Un affrontement
  se rejoue donc indéfiniment, ce qui est exactement ce qu'on demande à un banc d'essai — et ici
  c'est la fiction qui l'explique, pas une entorse aux règles.
- **N'importe quel appariement est justifié.** La *Law of the Arena* est le mode de résolution des
  litiges de l'Empire : la Malédiction malrokienne interdisant la guerre ouverte, les conflits entre
  citoyens ou entre régions se tranchent en arène. Opposer un moine à trois gobelins n'a donc pas
  besoin de prétexte.
- **Huit rôles tabulés.** Les Marques Héroïques se déclinent en Bruiser, Brute, Commander,
  Controller, Healer, Shooter, Tactician, Tank — une taxonomie toute faite pour vérifier que chaque
  classe livrée remplit bien le sien.
- **Les feats d'équipe** du *Player's Guide* (Coordinated Attack, Group Opportunity, Phalanx
  Tactics…) s'y branchent directement, et anticipent le [LOT-29](@ref lot-29).

Le lot livre l'arène comme **carte jouable** — une grille tactique dégagée, des gradins, des points
d'entrée — plus un écran de mise en place : choisir les combattants de chaque camp, leur niveau,
leur équipement, et lancer. Rejouable à graine fixée pour comparer deux versions d'une mécanique.

Sa vertu la plus durable est de n'être **jamais du code jetable** : c'est une zone du jeu final,
avec ses combats de gloire, sa Marque Héroïque à gagner et son statut social — le monde de Tanares
est né du jeu de plateau *Arena: The Contest*, et l'arène y est chez elle.

*Acceptation* — un affrontement se met en place, se joue et se rejoue **sans quitter le jeu** ; à
graine et composition égales, deux exécutions donnent le même déroulé ; aucun combattant n'y meurt
définitivement ; l'arène s'ouvre aussi comme une carte ordinaire depuis le monde
(`LOT-42`).

### `LOT-51` à `LOT-65` — une classe par lot {#lot-51}

*Prérequis de chacun : `LOT-47`, `LOT-50`, `LOT-70`.*

Quinze lots sur le même patron, un par classe restante. Chacun livre : la classe complète du niveau
1 au niveau 20, **sa mécanique propre implémentée dans le moteur**, au moins une sous-classe, et ses
tests.

Le critère d'acceptation est commun et tient en une phrase : **la classe se joue dans le Colisée
contre un adversaire de son niveau, et sa mécanique propre s'y observe** — la rage réduit
effectivement les dégâts subis, le châtiment divin consomme bien un emplacement, la forme sauvage
change réellement le profil. Une classe dont la mécanique ne se voit pas en combat n'est pas livrée.

| Lot | Classe | Mécanique propre | Ce qu'elle exige du moteur |
|---|---|---|---|
| `LOT-51` | Barbare | Rage, défense sans armure | Ressource par repos long, résistance conditionnelle, CA calculée autrement |
| `LOT-52` | Roublard | Attaque sournoise, Ruse | Condition d'avantage ou d'allié adjacent, action bonus de déplacement |
| `LOT-53` | Clerc | Conduit divin, sorts préparés | Emplacements, distinction préparés / connus, canal à usage limité |
| `LOT-54` | Magicien | Grimoire, restauration arcanique | Préparation depuis un grimoire, récupération partielle d'emplacements |
| `LOT-55` | Barde | Inspiration bardique | Dé confié à un **autre** personnage, dépensé plus tard, à distance |
| `LOT-56` | Moine | Ki, arts martiaux | Ressource par repos **court**, progression des attaques à mains nues |
| `LOT-57` | Paladin | Châtiment divin, auras | Conversion d'emplacement en dégâts, effet de zone permanent autour du porteur |
| `LOT-58` | Rôdeur | Ennemi juré, explorateur | Bonus conditionnés au **type** de créature et au terrain |
| `LOT-59` | Ensorceleur | Points de sorcellerie, métamagie | Conversion ressource ↔ emplacement, altération d'un sort à l'incantation |
| `LOT-60` | Sorcier | Magie de pacte, manifestations occultes | **Un second système d'emplacements** : peu nombreux, toujours au niveau maximal, récupérés au repos court |
| `LOT-61` | Druide | Forme sauvage | **Substitution complète du profil de créature** — le plus lourd du lot |
| `LOT-62` | Élémentaliste | Éléments primordiaux, sorts de fusion | Liste de sorts dédiée, combinaison de deux sorts en un |
| `LOT-63` | Dragonblade | Voies draconiques | Ressource liée à un dragon choisi, effets par couleur |
| `LOT-64` | Madwalker | *Trails* aberrantes | Progression par piste plutôt que par sous-classe unique |
| `LOT-65` | Redeemer | Bénédictions | Liste de bénédictions accordées à autrui, distincte des sorts |

Deux d'entre eux méritent d'être vus venir. Le **druide** (`LOT-61`) remplace le profil entier du
personnage par celui d'une bête pendant sa forme sauvage : c'est le seul cas où une classe touche à
l'agrégat que le [LOT-13](@ref lot-13) suppose stable, et il vaut mieux l'aborder tard, une fois le
reste éprouvé. Le **sorcier** (`LOT-60`) introduit un système d'emplacements parallèle : si le
`LOT-25` a codé « les emplacements » au singulier, il faudra y revenir — autant le savoir avant.

L'ordre du tableau n'est pas alphabétique : il va du plus simple au plus intrusif, pour que chaque
lot bénéficie du précédent.

**Le dernier lot livré retire l'échafaudage** : les quatre classes provisoires du `LOT-36` sont
supprimées, et rien ne doit s'en apercevoir.

### `LOT-66` — Charte visuelle : sortir de l'identité pixel art {#lot-66}

*Prérequis : **aucun** — ses références sont disponibles aujourd'hui. Prérequis de `LOT-67`,
`LOT-68`, `LOT-69`, `LOT-76`.*

> **Corrigé à l'audit, sur deux points.** Ce lot déclarait « Prérequis : `LOT-39` », ce qui le
> plaçait derrière tout le pipeline de données — alors que **la charte n'a rien à voir avec les
> créatures** : ses entrées sont la feuille de personnage et les artworks, déjà là. Et il ne visait
> qu'`EX-IHM-070` alors que **onze exigences** imposent le pixel art.

L'interface actuelle est celle du jeu de plateforme dont ce dépôt est issu, et elle **contredit
désormais explicitement** le reste des décisions. Ce n'est pas une impression, c'est écrit dans la
spécification :

> `EX-IHM-070` — « Les écrans du **jeu** doivent porter une identité **pixel art** assumée : police
> bitmap embarquée et cadres à bordure franche, rendus **sans lissage** à un facteur
> d'agrandissement **entier**. »

Cette exigence est justifiée dans le texte par « le menu d'un jeu de plateforme en pixel art ». Or le
`LOT-38` part d'une maquette de fiche à **300 ppp** et le `LOT-76` de **panneaux de parchemin
peints** extraits des livres. Une police bitmap non lissée et une illustration peinte à 300 ppp ne
cohabitent pas : il faut trancher, et le trancher **avant** que cinq écrans n'inventent chacun leur
langage visuel.

**Mais `EX-IHM-070` n'est que la feuille ; la racine est `EX-ARCH-022`** (« rendu pixel art :
échantillonnage *nearest-neighbor*, zoom caméra de préférence en facteurs entiers »), que toutes les
autres citent. Le périmètre réel est de **onze exigences** — la liste en annonçait dix et n'en
donnait que neuf ; `EX-DEC-032` (l'image pixel art enregistrée comme fichier) et `EX-REN-032` (la
police **bitmap**) manquaient :

`EX-ARCH-022` (racine) · `EX-DEC-003` · `EX-DEC-032` · `EX-DEC-043` · `EX-REN-032` · `EX-REN-041` ·
`EX-IHM-053` · `EX-IHM-070` · `EX-IHM-073` · `EX-EDIT-041` · `EX-EDIT-045`.

Une difficulté à regarder en face : le [LOT-01](@ref lot-01) a **délibérément conservé** l'atelier
pixel art lors de la purge, et le [LOT-11](@ref lot-11) en fait un acquis à ne pas régresser. Ce lot
revient sur cette décision ; il doit le dire, pas le faire en passant.

Le lot livre donc :

- la **nouvelle identité**, prise sur les références que tu as désignées — les feuilles de
  personnage (`Character_Sheets_Tanares.pdf`, 2 668 × 3 418) pour la structure et la hiérarchie
  typographique, les livres Tanares pour l'habillage : parchemin, cadres ornés, titrage à
  empattements, illustration peinte ;
- la **réécriture des tokens** de la portée identité (`DesignTokens`), dont les rôles « cadre pixel
  art » disparaissent au profit de rôles de panneau, de bordure ornée et de parchemin ;
- la **refonte des onze exigences** ci-dessus, à commencer par `EX-ARCH-022` ;
- la **suppression** des ~640 lignes de widgets pixel art de `Source/HMI/Interface/` (`PixelArtScale`,
  `PixelFocusCaret`, `PixelFrameGeometry`, `PixelFrameWidget`, `PixelMenuButton`), devenues sans
  objet — l'allègement se fait ici, pas dans un lot de ménage ultérieur.

Deux garde-fous à ne pas perdre au passage, parce qu'ils sont bons et indépendants de l'esthétique :
la marque de focus explicite d'`EX-IHM-071` — une teinte seule ne se suit pas à la manette, et ne se
voit pas du tout pour qui distingue mal les couleurs — et l'interdiction des réglages inopérants
d'`EX-IHM-072`. La **portée éditeur** n'est pas concernée : un outil de travail garde son apparence
d'outil de travail, et aucun parchemin ne doit se répandre dans ses tables denses.

> **La collision de numéros n'est pas traitée ici.** Les titres de sections d'`interface-ihm.md`
> encore intitulés « (LOT-56) », « (LOT-57) », « (LOT-68) », « (LOT-73) » renvoient au programme
> **hérité** de `ProjectGaming`. Ils ne sont que 26 des 208 renvois ambigus recensés au §9.5 : les
> désambiguïser fichier par fichier au fil des lots produirait exactement la moitié de convention
> qu'on cherche à éviter. Tout part au `LOT-78`, en une passe.

*Acceptation* — un écran du jeu et un écran de l'éditeur placés côte à côte se distinguent
immédiatement ; `scripts/check_design_tokens.py` passe sur les nouveaux rôles ; le focus reste
signalé par une marque, pas par une teinte.

### `LOT-67` — Menus et vocabulaire d'un RPG {#lot-67}

*Prérequis : `LOT-66`.*

Les menus existants décrivent un autre jeu, littéralement. `fr.lang` porte encore
`menu.select_level = Choisir un niveau`, `pause.restart = Recommencer le niveau`, et un
`pause.quit_confirm_text` qui parle de « la progression du **tableau** en cours ». Un bac à sable
ouvert n'a ni niveau à choisir, ni tableau à recommencer.

> **Élargi à l'audit.** Ce lot ne prévoyait que la migration des clés `.lang` — soit un dixième du
> travail. **Seize exigences** imposent encore une séquence de tableaux discrets, et ce sont elles
> le vrai chantier.

Le lot refond le **menu principal** (continuer, nouvelle partie, créer un personnage, options,
crédits), l'**écran de pause** (reprendre, fiche, inventaire, journal, carte, options, quitter) et
migre la famille de clés `level.*` vers le vocabulaire du monde — sans laisser de clé morte.

Mais il doit surtout **retirer la notion de niveau discret** de la spécification :

- `EX-GP-030`/`031`/`032` — atteindre la sortie termine le niveau en succès, le danger provoque
  l'échec, le niveau redémarre à son état initial ;
- `EX-GP-040` — l'état `NiveauTermine` de la machine à états ;
- `EX-LVL-010` → `EX-LVL-015` — ordre défini, enchaînement automatique au niveau suivant, **vingt-deux
  tableaux**, progression à la granularité du tableau ;
- `EX-IHM-003`/`004`/`005` — nom du tableau au HUD, écran de fin de niveau, sélection de niveau ;
- `EX-NFR-021` — test système de **franchissabilité** des niveaux du MVP.

D'où la suppression des écrans correspondants (`LevelSelectScreen`, `LevelCompleteScreen`, ~460
lignes) et des deux maquettes `SelectionNiveau.dc.html` et `FinDeNiveau.dc.html`. Accessoirement,
`Documentation/Doxyfile:10` annonce encore
`PROJECT_BRIEF = "Jeu 2D de plateforme/puzzle en C++/DirectX"`.

C'est le lot le moins spectaculaire de la refonte et celui qui se voit le plus : un joueur qui lit
« Recommencer le niveau » dans un monde ouvert comprend en une seconde que le jeu n'a pas été
terminé.

*Acceptation* — plus aucune clé de traduction n'emploie « niveau » ou « tableau » au sens du jeu de
plateforme ; les deux catalogues `fr.lang` et `en.lang` restent synchrones ; chaque entrée de menu
mène à un écran qui existe (`EX-IHM-072`) ; **aucune exigence en vigueur ne suppose plus une
séquence ordonnée de tableaux**.

### `LOT-68` — Le châssis des écrans RPG {#lot-68}

*Prérequis : `LOT-66`. Alimente `LOT-38`, `LOT-42`, `LOT-45`, [LOT-24](@ref lot-24).*

Huit écrans manquent, et aucun n'existe même en ébauche : fiche de personnage, inventaire et
équipement, journal de quêtes, carte du monde, dialogue, marchand, tableau de la Guilde, ATH de
combat.

Ce lot ne les **remplit** pas — c'est le travail du `LOT-38` pour la fiche, du `LOT-42` pour la
carte, du `LOT-45` pour la guilde, du [LOT-24](@ref lot-24) pour le combat. Il livre ce qu'ils ont
en commun et qu'aucun ne doit réinventer : le **cadre** (panneau, titre, zone de contenu, pied
d'actions), la **navigation** (ouverture, fermeture, retour, passage d'un écran à l'autre sans
repasser par le menu), le **parcours de focus** à la manette, et la règle de superposition — ce qui
met le jeu en pause et ce qui se consulte en marchant.

Sans ce châssis, les quatre lots qui suivent produisent quatre écrans qui s'ouvrent différemment,
se ferment différemment et se naviguent différemment. Avec lui, ils ne produisent que du contenu.

*Acceptation* — les huit écrans existent, même vides, et se naviguent entièrement à la manette ; un
écran s'ouvre et se ferme depuis n'importe quel autre sans repasser par le menu principal ; ajouter
un neuvième écran ne demande de toucher à aucun des huit.

### `LOT-69` — Retrait de l'atelier pixel art {#lot-69}

*Prérequis : [LOT-11](@ref lot-11), `LOT-66`.*

> **Deux fois corrigé à l'audit.** D'abord la prémisse : ce lot affirmait que l'éditeur est « un
> outil Qt séparé », ce qui est **faux** — `EX-EDIT-030` impose déjà « un éditeur intégré à
> l'application (mode éditeur), et **non un outil séparé** », et `EX-ARCH-090` prévoit un état
> `Éditeur`.
>
> Ensuite le périmètre, et c'est le changement qui compte. Ce lot était écrit comme la **refonte**
> de l'éditeur en mode édition dans la scène, et déclarait `LOT-50` en prérequis — ce qui plaçait le
> socle de classe et le Colisée **avant le vertical slice du [LOT-27](@ref lot-27)**, à rebours de
> l'argument du §6. Or le §10 conclut que le `LOT-11` n'étant pas commencé, il doit **viser d'emblée
> l'édition dans la scène**. Cette conclusion est ici prise au mot : la refonte est devenue le
> cahier des charges du `LOT-11`, et il ne reste à ce lot que la **suppression** de l'atelier de
> dessin — qui n'a besoin ni du Colisée ni des classes.

Le `LOT-66` supprime l'identité pixel art — et avec elle la raison d'être de l'**atelier pixel
art** que l'`EX-IHM-073` impose aujourd'hui comme l'un des deux espaces de travail exclusifs. Un
atelier de dessin de sprites au pixel n'a plus d'objet dans un jeu habillé de parchemin peint et
d'illustrations à 300 ppp. C'est tout le sujet de ce lot.

> **Ce qui a été transféré au `LOT-11`.** L'édition **dans la scène** — entrer en mode édition
> depuis le jeu, poser librement décors, objets, coffres, PNJ et ennemis sur la grille, sortir, et
> jouer immédiatement ce qu'on vient de poser. La boucle « modifier → constater » tombe de
> plusieurs minutes à quelques secondes, et c'est cette boucle qui décide si un outil sert vraiment.
> Sept exigences en portent le sens inverse et sont à réviser **là-bas** : `EX-IHM-001` range
> l'éditeur dans l'interface hors-jeu, `EX-IHM-010` en fait une fenêtre à panneaux dockables
> détachables, `EX-IHM-050`/`054`/`082` lui donnent une portée de style disjointe de celle du jeu,
> `EX-EDIT-020` en fait un « outil exécutable » pour non-codeurs, et `EX-EDIT-008` fait aller le flux
> **de l'éditeur vers le jeu**. `EX-VIS-006` — « un éditeur permettant à des membres
> non-développeurs de créer du contenu sans coder » — s'en trouve mieux servi, pas abandonné.
>
> Le construire d'emblée coûte ce que coûte le `LOT-11` ; le construire puis le refondre coûte le
> double. C'est la seule raison de ce transfert.

Ce qui est **conservé** : le format de niveau du [LOT-04](@ref lot-04), les couches, la couche de
collision qui fait foi, les portails, la validation du graphe — et le panneau de **textures**
(`TexturePanel`), qui affecte des habillages aux types de tuiles et reste utile.

Ce qui est **retiré** : l'atelier de dessin proprement dit — `PixelCanvas`, `PixelOperations`,
`PixelHistory`, `PixelPalette`, `PixelAssetIO`, `PixelAutotilePreview` et leurs deux panneaux, soit
**~2 975 lignes** — avec la révision d'`EX-IHM-073`, qui n'a plus deux espaces à rendre exclusifs,
et d'`EX-EDIT-045` qui le prescrit. Le guide `Documentation/Guide/guide-atelier-pixel-art.md` et les
huit fichiers de tests associés disparaissent avec.

> **Ce retrait est un revirement, pas un oubli.** Le [LOT-01](@ref lot-01) a conservé l'atelier lors
> de la purge, et le [LOT-11](@ref lot-11) en fait un acquis à ne pas régresser. La raison de
> revenir dessus est que le `LOT-66` retire l'identité pixel art : un atelier de dessin au pixel
> n'a plus de sujet. Mais c'est une décision à assumer explicitement, pas à laisser passer.

*Acceptation* — aucun panneau, aucun fichier ni aucun test de l'atelier pixel art ne subsiste ;
`EX-IHM-073` et `EX-EDIT-045` sont révisées ; l'éditeur du `LOT-11` continue de fonctionner à
l'identique, panneau de textures compris.

### `LOT-70` — Horloge de partie et cycle jour/nuit {#lot-70}

*Prérequis : [LOT-13](@ref lot-13). **Débloque `LOT-51`→`LOT-65`, [LOT-25](@ref lot-25),
`LOT-75` et `LOT-42`.***

> **Fusionné à l'audit.** Ce lot a absorbé l'ancien `LOT-71` (repos court et long). L'horloge n'a
> **aucun consommateur** hors du repos et du campement : livrée seule, elle ne produit rien
> d'observable, et le §6 les traitait déjà comme une paire indissociable. Le numéro `LOT-71` est
> retiré.

`core::GameClock` : un temps **de jeu**, distinct du temps réel, qui avance en exploration et se
**gèle en combat** — un combat se compte en tours, pas en minutes. Cycle jour/nuit, calendrier
simple.

**Le piège.** Le temps de jeu ne doit jamais dériver de l'horloge système. Le [LOT-26](@ref lot-26)
interdit déjà les graines liées à l'horloge, et pour la même raison : une sauvegarde rechargée
décalerait tout. Le temps avance par pas de simulation, et par rien d'autre.

**Et ce qu'un repos restaure.** Un repos court (1 h de jeu) et un repos long (8 h) : points de vie
via les dés de vie, ressources de classe selon **leur** cadence, emplacements de sorts. Interruption
par une rencontre.

**Le second piège.** Chaque ressource déclare sa propre cadence — repos court, repos long, à
volonté — et `Rest` ne connaît **aucune** classe. Sans cela, chacun des quinze lots de classes à
venir modifierait le code du repos, et la quinzième modification casserait la première.

*Acceptation* — 24 h de jeu s'écoulent en un nombre déterministe de pas ; une ressource « repos
court » se restaure au repos court et pas avant ; un repos interrompu ne restaure rien ; les quatre
classes provisoires du `LOT-36` récupèrent correctement sans que `Rest` les connaisse ; l'horloge ne
bouge pas
pendant un combat ; elle survit à une sauvegarde et à un rechargement.

### `LOT-72` — Conditions, agonie et mort {#lot-72}

*Prérequis : [LOT-12](@ref lot-12), [LOT-21](@ref lot-21), `LOT-35`.*

> **Fusionné à l'audit.** Ce lot a absorbé l'ancien `LOT-73` (agonie et mort). Les jets de
> sauvegarde contre la mort sont une **application** du système de conditions — inconscient,
> stabilisé — et les traiter à part faisait rouvrir par le second les fichiers du premier. Le numéro
> `LOT-73` est retiré.

Le moteur d'application : poser, empiler, expirer — en tours ou en temps de jeu — et l'effet sur les
jets : avantage, désavantage, incapacité d'agir.

**Le piège.** Une condition n'est pas un booléen sur la fiche. Deux sources peuvent poser
« empoisonné » avec deux durées différentes ; retirer l'une ne doit pas retirer l'autre. C'est le
piège de la classe d'armure du [LOT-14](@ref lot-14), transposé : on **recalcule depuis les
sources**, on n'accumule jamais.

**Et l'agonie.** Points de vie à 0, inconscience, jets de sauvegarde contre la mort (trois succès
ou trois échecs), stabilisation, critique à 0 PV, dégâts massifs. Et la mort **hors combat**,
aujourd'hui absente de tout document.

Ce lot **ne touche pas** à `EX-GP-030`/`031`/`032`, qui décrivent encore la mort du jeu de
plateforme : leur retrait appartient au `LOT-67`, qui retire la notion de niveau discret dans son
ensemble. Le `LOT-72` en dépend pour son vocabulaire, mais ne se les approprie pas — une exigence
retirée deux fois par deux lots est une exigence dont personne ne répond.

*Acceptation* — deux sources de la même condition, retrait de l'une, l'autre tient ; une condition
expire au bon tour ; chaque état du catalogue `LOT-35` a un effet observable, ou est explicitement
déclaré narratif ; trois échecs tuent, trois succès stabilisent, un soin au-dessus de 0 réinitialise
le compteur ; mourir en exploration a un effet défini, et ce n'est pas « redémarrer le niveau ».

### `LOT-74` — Expérience et progression {#lot-74}

*Prérequis : [LOT-13](@ref lot-13), [LOT-20](@ref lot-20), [LOT-16](@ref lot-16).*

**Les sources** d'expérience, qui manquent entièrement : victoire au combat selon le facteur de
puissance des adversaires, achèvement de quête, découverte de lieu — cette dernière propre au bac à
sable. Puis les seuils, la montée de niveau, et la répartition dans un groupe
([LOT-29](@ref lot-29)).

**Le piège.** Trois lots consomment l'expérience et aucun n'en produit : le `LOT-13` fait monter de
niveau, le `LOT-28` règle des seuils, le `LOT-29` la répartit, mais rien n'en attribue jamais. Et
`niveaux.md` fait encore progresser **par tableau franchi** (`EX-LVL-014`), une notion qui disparaît
avec le `LOT-67`.

*Acceptation* — tuer une créature de facteur de puissance connu donne l'expérience attendue ;
franchir un seuil monte d'un niveau, un dépassement multiple monte de plusieurs ; aucune progression
liée au franchissement d'un tableau ne subsiste.

### `LOT-75` — Campement et repos dans le monde {#lot-75}

*Prérequis : `LOT-70`, `LOT-42`, `LOT-41`.*

Où et quand on peut se reposer : le campement comme action en monde ouvert, le risque de rencontre
nocturne, l'auberge comme lieu sûr — que le [LOT-09](@ref lot-09) cite déjà comme nœud de graphe
sans lui donner de fonction.

**Le piège.** Un repos long disponible partout et sans risque annule toute gestion de ressources —
or c'est elle qui rend le repos intéressant. La contrainte doit venir des **données de région**
(`Crime and Violence`, `Monster Presence` du `LOT-41`), jamais d'une règle codée : c'est ainsi que
dix régions donnent dix rapports au repos sans qu'une ligne de C++ les distingue.

*Acceptation* — se reposer en zone dangereuse déclenche des rencontres à une fréquence dérivée de la
région ; une auberge garantit un repos non interrompu ; l'horloge avance du montant attendu.

### `LOT-76` — Habillage d'interface extrait des livres {#lot-76}

*Prérequis : `LOT-30`, `LOT-66`. Alimente `LOT-38`, [LOT-15](@ref lot-15),
[LOT-24](@ref lot-24).*

Détaché du `LOT-39` à l'audit, parce qu'il n'a besoin que des PDF : panneaux de parchemin, cadres,
bordures, pictogrammes. Ces éléments sont autonomes dans la page et sortent parfaitement par **rendu
clippé** (méthode du §4 — jamais l'extraction du flux brut).

Ils habillent la fiche du `LOT-38`, les boîtes de dialogue du [LOT-15](@ref lot-15) et l'IHM de
combat du [LOT-24](@ref lot-24). C'est le premier gisement d'images exploitable, et le seul qui ne
demande aucun arbitrage humain.

*Acceptation* — les éléments extraits s'intègrent aux jetons de la charte du `LOT-66` ;
`scripts/check_design_tokens.py` reste vert ; aucune image n'est tirée par extraction de flux brut.

### `LOT-77` — La moitié RPG de la spécification {#lot-77}

*Prérequis : aucun. **Prérequis de toute la filière**, à commencer par le `LOT-30`.*

> **Créé à l'audit.** Cette précondition était signalée trois fois — aux §5, §6 et §8 — et rangée
> dans « à trancher », c'est-à-dire nulle part. Vingt lots pointent vers des familles d'exigences
> qui n'existent pas ; tant que personne n'en répond, la dette grossit à chaque lot livré.

Cinq familles d'exigences sont **fantômes** : `EX-CNT-*`, `EX-DND-*`, `EX-RPG-*`, `EX-CBT-*` et
`EX-INV-*`. Les quatre documents censés les porter n'existent pas. Ce lot les écrit :

| Document | Famille | Ce qu'il fixe |
|---|---|---|
| `Specification/contenu.md` | `EX-CNT-*` | Le contrat de la filière données : provenance, schémas, clés d'assets, marquage du provisoire, refus explicite d'un mécanisme non honoré |
| `Specification/regles-dnd.md` | `EX-DND-*` | Les règles d20 maison : jets, avantage, maîtrise, caractéristiques, repos, conditions, facteur de puissance |
| `Specification/combat.md` | `EX-CBT-*` | Le combat tactique : initiative, action/action bonus/réaction, portée, ligne de vue, dégâts, mort |
| `Specification/rpg.md` | `EX-RPG-*` | Le personnage : espèces, classes, ressources de classe, progression, expérience |
| `Specification/inventaire.md` | `EX-INV-*` | Inventaire, encombrement, équipement, monnaie, commerce |

Le travail n'est pas d'inventer : le cadrage acté avant le [LOT-01](@ref lot-01) — règles d20 maison,
compatibles SRD dans leur structure, sans en dépendre — et le corpus donnent la matière. Il est de
**décider ce que le moteur promet**, en identifiants stables, pour que les vingt lots qui les citent
cessent de pointer dans le vide.

C'est aussi ce lot qui **éteint l'échec volontaire du lint** (§9.1) : tant qu'il n'est pas livré,
`scripts/lint_exigences.py` doit continuer d'échouer sur les cinq familles. Un lint désarmé « en
attendant » ne se réarme jamais.

*Acceptation* — `python scripts/lint_exigences.py` passe au vert sans liste d'exceptions ajoutée ;
chaque lot de la filière peut renseigner sa rubrique « Exigences couvertes » ; aucune exigence
nouvelle ne décrit un mécanisme que le §5 déclare hors périmètre.

### `LOT-78` — Désambiguïsation des numéros de lots hérités {#lot-78}

*Prérequis : aucun. **Prérequis du démarrage de tout lot de la filière.***

> **Créé à l'audit, et bien plus gros qu'estimé.** Le §10 chiffrait ce travail à « une
> recherche-remplacement dans six specs ». Le décompte réel est de **208 renvois ambigus dans douze
> fichiers** (§9.5). Ce n'est pas une passe de relecture, c'est un lot.

Ce lot doit précéder la création du premier dossier de lot au-delà du `LOT-29`.

Le programme hérité de `ProjectGaming` allait jusqu'à `LOT-74` ; la filière atteint `LOT-84`. Tout
`LOT-NN` au-delà de `LOT-07` cité dans une spécification est donc **ambigu sans être cassé** : ni le
lint ni Doxygen ne le signalent, et un lecteur ne peut pas trancher.

Le lot classe les 208 renvois un par un — lot hérité ou lot courant — puis :

- préfixe les renvois hérités en **`LOT-H-XX`**, avec un renvoi explicite à
  `Documentation/Heritage/Lot/` ;
- laisse tels quels les renvois au programme courant ;
- ajoute au lint la règle qui empêche la reformation de l'ambiguïté : un `LOT-NN` cité dans une
  spécification doit désigner un lot **existant** de ce programme, sans quoi il s'écrit `LOT-H-NN`.

`architecture.md` illustre le piège mieux qu'un argument : son `EX-ARCH-031` dit « Concrétisé en
`LOT-33` » en parlant d'une interpolation de rendu **déjà livrée** — alors que le `LOT-33` de ce
programme est le bestiaire de base, et n'est pas commencé.

*Acceptation* — aucun `LOT-NN` d'une spécification ne désigne un lot hérité sans préfixe `LOT-H-` ;
le lint refuse un renvoi vers un numéro de lot inexistant ; la Doxygen ne produit aucune ancre en
double avec l'archive.

### `LOT-79` — Socle de chargement de données {#lot-79}

*Prérequis : aucun. Prérequis de `LOT-32`.*

> **Créé à l'audit.** Le §10 attribuait la brique de chargement JSON au `LOT-32` — mais son
> périmètre ne livre que des schémas, un script Python et un test d'énumérations : **personne ne
> portait le travail C++**. Les tests paramétrés n'étaient dans aucun lot du tout.

Le dépôt compte **six réimplémentations** de `loadFromFile` — `SkinCatalog`, `AnimationCatalog`,
`SoundCatalog`, `PixelPalette`, `LevelLoader`, `LevelSequence` — chacune avec sa validation écrite à
la main, champ par champ, et aucun schéma. La filière s'apprête à en ajouter une quinzaine.

Le lot livre donc, avant tout catalogue :

- une **brique de lecture** unique : ouverture, parsage, report d'erreur avec fichier et ligne,
  validation contre un schéma, conversion vers un agrégat typé ;
- la **migration des six lecteurs existants** dessus — c'est la migration qui prouve la brique, pas
  un septième lecteur écrit à côté ;
- la capacité de **test paramétré** absente de tout `Source/Test/` : ni `TEST_P`, ni parcours de
  dossier de fixtures. Les lots de données en vivent : un catalogue se teste en balayant un dossier,
  pas en écrivant un test par entrée.

C'est le lot dont l'absence coûte le plus cher, parce que son prix **double à chaque catalogue
livré** : le faire maintenant, c'est six lecteurs à reprendre ; le faire dans dix lots, c'est vingt.

*Acceptation* — les six lecteurs existants passent par la brique et leurs tests restent verts ; une
donnée invalide produit un message portant le fichier et la ligne ; un test paramétré balaie un
dossier de fixtures et échoue en nommant l'entrée fautive.

### `LOT-80` — Factions, panthéon et organisations {#lot-80}

*Prérequis : `LOT-37`. Alimente [LOT-16](@ref lot-16), `LOT-82`.*

Détaché du `LOT-37` à l'audit : les **5 factions**, **18 divinités**, **11 organisations**, **7 lieux
du plan pénombral**, la chronologie et l'économie de minerais rares, vers
`Source/Elements/World/factions/`, `pantheon/` et `organizations/`.

Ce sont les entités **transverses aux régions**, et elles servent un autre consommateur que l'atlas :
une faction porte des objectifs qui se traduisent en quêtes, un panthéon porte des domaines qui se
traduisent en capacités. C'est ce qui rend le [LOT-16](@ref lot-16) écrivable plutôt qu'improvisable
— et c'est aussi pourquoi ce lot peut arriver après le `LOT-37` sans retarder le graphe de cartes.

*Acceptation* — toute faction, divinité ou organisation citée par une région du `LOT-37` existe dans
le catalogue ; chaque faction déclare au moins un objectif exploitable comme gabarit de quête ;
aucune entité orpheline, vérifié par `check_world_graph.py`.

### `LOT-81` — Descripteurs de terrain des dix régions {#lot-81}

*Prérequis : `LOT-37`. Prérequis de `LOT-40`.*

Détaché du `LOT-40` à l'audit, pour séparer la donnée du moteur. Chaque région déclare son
**descripteur de terrain** — proportions de biomes, présence d'eau, de relief, de bâti — dérivé de sa
section `Geography`.

La matière est déjà écrite : le Central Empire annonce « vallées fluviales, vastes prairies, forêt
du Bak, marais, hauts plateaux du nord » ; le Freelands « landes et zones humides, forêts tempérées,
chaînes glacées, deux mers ». Ce sont des recettes de terrain, pas de la prose d'ambiance. Le
travail est de les **typer** dans le vocabulaire de tuiles du [LOT-08](@ref lot-08), et de les
relire.

*Acceptation* — les dix régions portent un descripteur validé par schéma ; chaque biome cité
correspond à un type de tuile existant ; deux régions distinctes ne portent pas le même descripteur.

### `LOT-82` — Peuplement civil : PNJ, marchands et quêtes {#lot-82}

*Prérequis : `LOT-41`, `LOT-34`, `LOT-44`, `LOT-80`, [LOT-15](@ref lot-15), [LOT-26](@ref lot-26).
Alimente `LOT-45`, `LOT-42`.*

Détaché du `LOT-41` à l'audit : le versant **non hostile** du peuplement, dérivé des quatre autres
`Regional Statistics`.

| Statistique | Ce qu'elle pilote |
|---|---|
| `Magic Access` | Présence d'objets magiques chez les marchands, PNJ lanceurs de sorts |
| `Economic Prosperity` | Prix pratiqués, richesse des étals, valeur du butin |
| `Government Corruption` | Disponibilité des marchés noirs et des contrats douteux |
| `Citizen Freedom` | Ton des dialogues génériques, ce que les PNJ osent dire |

S'y ajoute la **répartition des espèces** de la région — le Freelands est à 70 % humain, 11 % elfe
d'automne, 8 % elfe d'hiver : les PNJ générés suivent cette distribution, avec les noms du `LOT-44`.

Les quêtes se composent sur le même principe : un objectif de faction venu du `LOT-80`, une cible
dans la région, une récompense tirée de sa prospérité. Le [LOT-16](@ref lot-16) fournit les drapeaux,
ce lot fournit les gabarits qui s'en servent.

*Acceptation* — deux régions aux statistiques opposées produisent des **prix moyens et des stocks
mesurablement différents**, vérifié par un test ; mille PNJ générés suivent la distribution
d'espèces déclarée par leur région ; aucun PNJ ni aucun marchand n'est placé en dur dans le code.

### `LOT-83` — Boucle de progression de la Guilde {#lot-83}

*Prérequis : `LOT-45`, `LOT-74`.*

Détaché du `LOT-45` à l'audit : le catalogue des rangs et des contrats est une donnée et un écran,
la **progression** est une mécanique — et elle suppose l'expérience du `LOT-74`, que le `LOT-45`
n'exigeait pas.

Monter de rang : ce qui le déclenche (contrats accomplis, niveau atteint), ce que cela change dans
l'offre affichée, ce que cela ouvre — accès aux régions de rang supérieur, tarifs, réputation auprès
des factions du `LOT-80`. Et le garde-fou que la fiction impose déjà : « empêcher les aventuriers
inexpérimentés de prendre des tâches au-dessus de leurs moyens » — un joueur de niveau 2 doit être
**averti** avant d'entrer dans une région d'Argent, pas seulement puni.

*Acceptation* — accomplir les contrats d'un rang fait monter au suivant ; monter de rang change
l'offre de façon observable ; entrer dans une zone au-dessus de son rang produit un avertissement
diégétique, et non un écran de mort.

### `LOT-84` — Les 31 tables de progression de classe {#lot-84}

*Prérequis : `LOT-32`. Prérequis de `LOT-47`.*

Détaché du `LOT-47` à l'audit : c'est de la **donnée**, extraite et relue, et elle appartient à la
filière contenu aux côtés des `LOT-36` et `LOT-43`. Rien ne justifiait qu'elle attende le socle de
classe ; l'inverse, si.

Les **31 tables de progression** sont extraites pour les seize classes et leurs sous-classes, même
celles dont la mécanique viendra bien plus tard. Le §4 le démontre sur la table du barbare :
illisible en `-layout`, où le niveau 5 reçoit « Amélioration de caractéristiques » au lieu
d'« Attaque supplémentaire », **exacte** en `-table`. C'est le lot où le §4 se paie une seconde fois.

La donnée peut précéder le code — c'est même souhaitable, cela rend le travail restant visible : les
quinze lots de classes à venir lisent une table qui existe déjà, au lieu de l'extraire chacun pour
soi.

Le `LOT-49` **contrôle** ces tables comme il contrôle les autres catalogues — il ne les précède
pas et n'en dépend pas : c'est un filet, pas un maillon.

*Acceptation* — les 31 tables sont extraites et validées par schéma ; trois d'entre elles sont
comparées **ligne à ligne** au PDF ; le contrôle statistique du `LOT-49` ne signale aucune anomalie
non acceptée.

---

> **Quatre numéros retirés.** Les `LOT-31`, `LOT-48`, `LOT-71` et `LOT-73` ont été absorbés par
> fusion et **ne seront pas réattribués** : un identifiant de lot est stable, y compris quand il
> cesse de désigner du travail. Le tableau ci-dessous dit où leur contenu est parti.
>
> | Numéro retiré | Contenu | Absorbé par |
> |---|---|---|
> | `LOT-31` | Lexique bilingue et autorité de traduction | `LOT-30` |
> | `LOT-48` | Portraits et illustrations | Dissous dans chaque lot de catalogue |
> | `LOT-71` | Repos court et long | `LOT-70` |
> | `LOT-73` | Agonie et mort | `LOT-72` |


---

## 6. Ordre d'exécution recommandé

Les numéros suivent [LOT-29](@ref lot-29), l'exécution non. Les lots de données doivent précéder les
lots qui les consomment, sans quoi ces derniers se construisent sur des catalogues fictifs — et un
catalogue fictif finit toujours par se figer en valeurs codées en dur, exactement ce que
[`EX-VIS-007`](@ref EX-VIS-007) interdit.

Ces cinquante et un lots ne forment **pas une phase** qui suivrait le [LOT-29](@ref lot-29) : ils s'entrelacent
avec les phases B à E, parce que chacun sert un lot existant qui, sans lui, se construirait sur un
catalogue fictif — et un catalogue fictif finit toujours par se figer en valeurs codées en dur,
exactement ce que [`EX-VIS-007`](@ref EX-VIS-007) interdit. D'où « filière » plutôt que « phase ».

| Quand | Lots | Pourquoi là |
|---|---|---|
| **Avant tout le reste** | `LOT-77`, `LOT-78`, `LOT-79` | Trois préconditions sans aucun prérequis : les exigences, les numéros, la brique de chargement |
| Tout de suite après [LOT-08](@ref lot-08) | `LOT-30`, puis `LOT-32` | Outillage et contrats ; le plus tôt est le mieux |
| **Tout de suite après [LOT-08](@ref lot-08)** | `LOT-66`, `LOT-67`, `LOT-68`, `LOT-76` | La charte ne dépend que des PDF, et elle conditionne tous les écrans à venir |
| Avec [LOT-09](@ref lot-09) | `LOT-37`, puis `LOT-80` | L'atlas donne au graphe de cartes de vrais nœuds à relier |
| Avant [LOT-13](@ref lot-13) | `LOT-43`, puis `LOT-36` | Compétences et langues d'abord, car les espèces s'appuient dessus ; puis les 4 classes simplifiées pour le premier combat |
| Avant [LOT-14](@ref lot-14) | `LOT-34`, puis `LOT-49` | Catalogue réel, puis contrôle de ses valeurs |
| Avant [LOT-21](@ref lot-21) / [LOT-23](@ref lot-23) | `LOT-33` | Attaques et IA ont besoin de vraies créatures |
| Avant [LOT-25](@ref lot-25) | `LOT-35` | Les sorts sont des données avant d'être un système |
| Avant [LOT-25](@ref lot-25) et les lots de classes | `LOT-70` | Un petit lot qui en débloque quinze |
| Après [LOT-13](@ref lot-13) | `LOT-38`, puis `LOT-39` | La maquette suppose la fiche ; la plomberie d'assets suit |
| Avec [LOT-11](@ref lot-11) | `LOT-69` | Le `LOT-11` vise l'édition dans la scène ; ce lot retire l'atelier devenu sans objet |
| Après [LOT-11](@ref lot-11) | `LOT-81`, puis `LOT-40` | Les descripteurs, puis le générateur qui les consomme |
| Avec [LOT-21](@ref lot-21) | `LOT-72` | Conditions, agonie et mort appartiennent au combat |
| Avec [LOT-20](@ref lot-20) | `LOT-74` | L'expérience se gagne à la fin d'un combat |
| Avant `LOT-47` | `LOT-84` | Les 31 tables sont de la donnée : elles précèdent le socle qui les lit |
| Après [LOT-21](@ref lot-21) | `LOT-47`, puis `LOT-50` | Le socle de classe, puis le lieu où éprouver ce qu'on lui ajoute |
| **Après [LOT-27](@ref lot-27)** | `LOT-44`, `LOT-46`, `LOT-41`, `LOT-82`, `LOT-45`, `LOT-83`, `LOT-42` | Le bac à sable généralise une boucle ; il faut l'avoir validée une fois |
| Avec `LOT-42` | `LOT-75` | Le campement suppose le voyage et le peuplement |
| Un par un, après `LOT-50` | `LOT-51` → `LOT-65` | Une classe, une mécanique, un test dans l'arène ; le dernier retire l'échafaudage |

Quatre lignes méritent qu'on s'y arrête.

**La première.** Ces trois préconditions ne dépendent de rien et bloquent tout : le `LOT-77` écrit
les exigences que vingt lots citent sans qu'elles existent, le `LOT-78` désambiguïse 208 renvois de
numéros avant que le premier dossier au-delà de `LOT-29` ne fige l'ambiguïté, le `LOT-79` factorise
six lecteurs JSON avant que la filière n'en ajoute quinze. Aucune n'est un préalable de confort :
chacune coûte deux à trois fois plus cher un an plus tard.

**L'avant-avant-dernière** : les lots qui font le bac à sable sont les seuls que je placerais
résolument **après** le [LOT-27](@ref lot-27). Peupler dix régions revient à appliquer cent fois la
même recette ; si la recette est mauvaise — rencontres mal dosées, marchands inutiles, quêtes vides
— on la découvre cent fois. Le `LOT-27` coûte un village et un donjon, et c'est le prix pour ne pas
payer cette erreur au centuple. Leur ordre interne compte aussi : `LOT-44` (les noms) avant
`LOT-41` (le peuplement), sinon on peuple avec des « Villageois 1 » ; `LOT-45` (la Guilde) après le
`LOT-82`, parce qu'elle a besoin de quêtes à afficher.

**Le volume d'images n'est plus un lot.** 6 000 images ne se traitent pas d'un bloc, et rien n'y
oblige — le `LOT-39` affiche un marqueur pour tout ce qui n'a pas encore d'image, et chaque
illustration livrée en remplace un. Ce travail était le `LOT-48`, « volume long, sans jalon » ; il
est désormais **réparti dans chaque lot de catalogue**, qui livre ses propres illustrations. Un lot
sans date de fin est un lot qu'on ne finit pas — et en garder un pendant qu'on écrit cette phrase
était le plus visible des angles morts de cette page.

### Le chemin critique jusqu'au *vertical slice*

C'est la contrainte que cette feuille de route a le plus de mal à tenir, parce qu'elle se dégrade
sans que personne ne décide rien : chaque lot qu'on déclare « avant le [LOT-27](@ref lot-27) »
repousse d'autant le seul jalon qui prouve que le jeu **est un jeu**.

L'audit a trouvé le `LOT-69` placé « avec `LOT-50`, avant le `LOT-27` ». Comme le `LOT-50` exige le
socle de classe, cela plaçait `LOT-84 → LOT-47 → LOT-50 → LOT-69` — les seize classes, le Colisée et
le mode édition — **devant** le slice. Vingt-six des lots de la filière se retrouvaient ainsi en
amont d'un jalon censé arriver tôt. La correction est au `LOT-69`, réduit à une suppression.

Le chemin critique restant est celui des **catalogues, pas des systèmes** :

`LOT-77` → `LOT-79` → `LOT-30` → `LOT-32` → { `LOT-33`, `LOT-34`, `LOT-43` → `LOT-36`,
`LOT-37` } → [LOT-27](@ref lot-27)

Tout le reste — classes, Colisée, peuplement, voyage, guilde — vient **après**. La règle à tenir
tient en une phrase : *un lot n'entre dans ce chemin que si le slice ne peut pas se jouer sans lui.*

### Le graphe des dépendances

Les liens ci-dessous sont ceux que déclarent les lignes « Prérequis » de la section 5. Ils sont
**vérifiés en CI** par `scripts/lint_lots.py`, qui refuse un cycle, un lien déclaré d'un seul côté,
un lot absent du tableau d'ordre ci-dessus ou une section de lot sans ancre Doxygen.

Le graphe ci-dessous est donné en **source Graphviz**, et non en diagramme rendu : la chaîne
Doxygen du projet tourne sans `HAVE_DOT`, et ajouter Graphviz au runner pour une seule
illustration coûterait plus que de coller ces vingt lignes dans un visualiseur.

```dot
digraph filiere {
  rankdir=LR;
  node [shape=box, style=rounded, fontsize=10];
  subgraph cluster_pre { label="Préconditions"; style=dashed;
    L77 [label="LOT-77\nSpécification RPG"];
    L78 [label="LOT-78\nNuméros hérités"];
    L79 [label="LOT-79\nChargement JSON"];
  }
  subgraph cluster_slice { label="Chemin critique du vertical slice"; style=dashed;
    L30 [label="LOT-30\nExtraction + lexique"];
    L32 [label="LOT-32\nSchémas"];
    L33 [label="LOT-33\nBestiaire"];
    L34 [label="LOT-34\nÉquipement"];
    L43 [label="LOT-43\nOptions perso"];
    L36 [label="LOT-36\nEspèces"];
    L37 [label="LOT-37\nAtlas régions"];
    L27 [label="LOT-27\nVertical slice", shape=box, style="rounded,bold"];
  }
  L77 -> L30; L79 -> L32; L30 -> L32;
  L32 -> L33; L32 -> L34; L32 -> L43; L32 -> L37;
  L43 -> L36;
  L33 -> L27; L34 -> L27; L36 -> L27; L37 -> L27;
  L84 [label="LOT-84\n31 tables"]; L47 [label="LOT-47\nSocle de classe"];
  L50 [label="LOT-50\nColisée"]; L51 [label="LOT-51→65\nune classe par lot"];
  L32 -> L84 -> L47 -> L50 -> L51;
  L36 -> L47; L43 -> L47; L70 [label="LOT-70\nTemps et repos"] -> L51;
  L80 [label="LOT-80\nFactions"]; L81 [label="LOT-81\nDescripteurs"];
  L40 [label="LOT-40\nGénérateur"]; L41 [label="LOT-41\nRencontres"];
  L82 [label="LOT-82\nPeuplement civil"]; L45 [label="LOT-45\nGuilde"];
  L83 [label="LOT-83\nProgression guilde"]; L42 [label="LOT-42\nVoyage"];
  L37 -> L80; L37 -> L81 -> L40 -> L41 -> L82 -> L45 -> L83;
  L80 -> L82; L82 -> L42;
  L27 -> L41 [style=dotted, label="valide la recette"];
  L66 [label="LOT-66\nCharte visuelle"];
  L66 -> L67 [label=""]; L67 [label="LOT-67\nMenus RPG"];
  L66 -> L68 [label=""]; L68 [label="LOT-68\nChâssis écrans"];
  L66 -> L69 [label=""]; L69 [label="LOT-69\nRetrait atelier"];
}
```

### Récapitulatif : qui dépend de qui

Ce tableau est **généré depuis les lignes « Prérequis » ci-dessus** et vérifié en CI par
`scripts/lint_lots.py` : s'il diverge du texte, c'est le lint qui échoue, pas le lecteur qui s'en
aperçoit trois lots plus tard. La colonne « Alimente » est l'inverse calculé de la colonne
« Prérequis », augmentée des dépendances que les lots livrés et absorbés ne peuvent pas déclarer
eux-mêmes.

| Lot | Objet | Prérequis | Alimente |
|---|---|---|---|
| `LOT-30` | Chaîne d'extraction du corpus et lexique bilingue | `LOT-77` | `LOT-32`, `LOT-46`, `LOT-76` |
| `LOT-32` | Schémas de données RPG | `LOT-30`, `LOT-79` | `LOT-33`, `LOT-34`, `LOT-35`, `LOT-36`, `LOT-37`, `LOT-43`, `LOT-84` |
| `LOT-33` | Bestiaire de base | `LOT-32` | `LOT-13`, `LOT-21`, `LOT-23`, `LOT-27`, `LOT-39`, `LOT-41`, `LOT-46`, `LOT-49` |
| `LOT-34` | Équipement, monnaie, objets magiques | `LOT-32` | `LOT-14`, `LOT-26`, `LOT-27`, `LOT-39`, `LOT-49`, `LOT-82` |
| `LOT-35` | Sorts et états | `LOT-32` | `LOT-25`, `LOT-72` |
| `LOT-36` | Espèces, historiques et classes provisoires | `LOT-32`, `LOT-43` | `LOT-13`, `LOT-27`, `LOT-44`, `LOT-47` |
| `LOT-37` | Atlas des régions et graphe de cartes | `LOT-32` | `LOT-09`, `LOT-27`, `LOT-39`, `LOT-44`, `LOT-80`, `LOT-81` |
| `LOT-38` | Fiche de personnage : maquette et interface | `LOT-13`, `LOT-39`, `LOT-68`, `LOT-76` | `LOT-24` |
| `LOT-39` | Plomberie des clés d'assets | `LOT-33`, `LOT-34`, `LOT-37` | `LOT-11`, `LOT-27`, `LOT-38` |
| `LOT-40` | Générateur de terrain | `LOT-11`, `LOT-81` | `LOT-41` |
| `LOT-41` | Peuplement : rencontres et créatures | `LOT-27`, `LOT-33`, `LOT-40`, `LOT-44`, `LOT-46` | `LOT-75`, `LOT-82` |
| `LOT-42` | Voyage et carte du monde | `LOT-09`, `LOT-17`, `LOT-68`, `LOT-70`, `LOT-82` | `LOT-75` |
| `LOT-43` | Options de personnage : dons, multiclassage, compétences, langues | `LOT-32` | `LOT-13`, `LOT-36`, `LOT-47` |
| `LOT-44` | Noms, tables aléatoires et contenu d'ambiance | `LOT-36`, `LOT-37` | `LOT-41`, `LOT-82` |
| `LOT-45` | Guilde des Aventuriers : rangs et contrats | `LOT-16`, `LOT-26`, `LOT-68`, `LOT-82` | `LOT-83` |
| `LOT-46` | Créatures de Tanares | `LOT-30`, `LOT-33` | `LOT-41` |
| `LOT-47` | Socle de classe, et le guerrier comme preuve | `LOT-36`, `LOT-43`, `LOT-84` | `LOT-50`, `LOT-51` |
| `LOT-49` | Contrôle de cohérence du contenu | `LOT-33`, `LOT-34` | — |
| `LOT-50` | Le Colisée : bac à sable de combat | `LOT-19`, `LOT-20`, `LOT-21`, `LOT-47` | `LOT-51`, `LOT-65` |
| `LOT-51` | une classe par lot | `LOT-47`, `LOT-50`, `LOT-70` | — |
| `LOT-66` | Charte visuelle : sortir de l'identité pixel art | — | `LOT-67`, `LOT-68`, `LOT-69`, `LOT-76` |
| `LOT-67` | Menus et vocabulaire d'un RPG | `LOT-66` | — |
| `LOT-68` | Le châssis des écrans RPG | `LOT-66` | `LOT-24`, `LOT-38`, `LOT-42`, `LOT-45` |
| `LOT-69` | Retrait de l'atelier pixel art | `LOT-11`, `LOT-66` | — |
| `LOT-70` | Horloge de partie et cycle jour/nuit | `LOT-13` | `LOT-25`, `LOT-42`, `LOT-51`, `LOT-65`, `LOT-75` |
| `LOT-72` | Conditions, agonie et mort | `LOT-12`, `LOT-21`, `LOT-35` | — |
| `LOT-74` | Expérience et progression | `LOT-13`, `LOT-16`, `LOT-20` | `LOT-83` |
| `LOT-75` | Campement et repos dans le monde | `LOT-41`, `LOT-42`, `LOT-70` | — |
| `LOT-76` | Habillage d'interface extrait des livres | `LOT-30`, `LOT-66` | `LOT-15`, `LOT-24`, `LOT-38` |
| `LOT-77` | La moitié RPG de la spécification | — | `LOT-30` |
| `LOT-78` | Désambiguïsation des numéros de lots hérités | — | — |
| `LOT-79` | Socle de chargement de données | — | `LOT-32` |
| `LOT-80` | Factions, panthéon et organisations | `LOT-37` | `LOT-16`, `LOT-82` |
| `LOT-81` | Descripteurs de terrain des dix régions | `LOT-37` | `LOT-40` |
| `LOT-82` | Peuplement civil : PNJ, marchands et quêtes | `LOT-15`, `LOT-26`, `LOT-34`, `LOT-41`, `LOT-44`, `LOT-80` | `LOT-42`, `LOT-45` |
| `LOT-83` | Boucle de progression de la Guilde | `LOT-45`, `LOT-74` | — |
| `LOT-84` | Les 31 tables de progression de classe | `LOT-32` | `LOT-47` |


---

## 7. Arborescence cible

```
Documentation/SourceBook/   ← les PDF, exclus du dépôt (taille)

scripts/sourcebook/
  corpus.toml               ← manifeste : empreintes, pagination, provenance (LOT-30)
  *.py                      ← chaîne d'extraction, sur PyMuPDF (LOT-30)
scripts/check_rpg_data.py, check_glossary.py
scripts/lint_lots.py        ← controle du graphe de lots de cette page (LOT-78)

Source/Elements/Rpg/
  schema/*.schema.json      ← contrats (LOT-32)
  creatures/*.json          ← 94 betes (LOT-33), puis 82 de Tanares (LOT-46)
  items/*.json              ← équipement, monnaie, objets magiques (LOT-34)
  spells/, conditions/      ← sorts et états (LOT-35)
  species/, backgrounds/    ← especes et historiques (LOT-36)
  classes/*.json            ← 4 provisoires (LOT-36), 31 tables (LOT-84), socle (LOT-47),
                              puis une classe par lot (LOT-51 a LOT-65)
  feats/, skills/, languages/        ← options de personnage (LOT-43)
  tables/*.json             ← noms, personnalite, babioles (LOT-44)
  guild/*.json              ← rangs et gabarits de contrat (LOT-45), progression (LOT-83)

Source/Elements/World/
  arena/*.json              ← Colisee : appariements, marques heroiques, roles (LOT-50)
  regions/*.json            ← 10 regions : statistiques, especes, factions (LOT-37)
                              + descripteurs de terrain (LOT-81)
  locations/*.json          ← lieux nommes, avec leurs effets mecaniques (LOT-37)
  factions/, pantheon/, organizations/            ← (LOT-80)
  populate/*.json           ← gabarits de rencontre (LOT-41), civils et quetes (LOT-82)
Source/Elements/Assets/
  rpg.assets.json           ← manifeste des clés d'assets (LOT-39)
  Ui/, Portraits/, Creatures/        ← textures extraites (LOT-39)
Source/Elements/Localization/rpg.glossary.csv   ← lexique (LOT-30)

Source/Core/Data/
  JsonDocument.{h,cpp}      ← brique de lecture unique, validation par schema (LOT-79)
Source/Core/World/
  TerrainGenerator.{h,cpp}  ← generation pilotee par descripteur de region (LOT-40)
  RegionPopulator.{h,cpp}   ← peuplement systemique (LOT-41, LOT-82)
  WorldMap.{h,cpp}          ← voyage et decouverte (LOT-42)
Source/Core/Time/
  GameClock.{h,cpp}, Rest.{h,cpp}    ← horloge, repos court et long (LOT-70)

Documentation/Specification/
  contenu.md, regles-dnd.md, combat.md, rpg.md, inventaire.md   ← (LOT-77)
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
  modèle de combat (`LOT-36`) et **seront retirées** au profit des seize classes complètes livrées
  par le `LOT-47` puis du `LOT-51` au `LOT-65`. Elles portent pour cela un `"statut": "provisoire"` en donnée, et le
  **dernier lot de classe livré** ne se clôt que sur leur suppression effective.
- **Une classe par lot, pas un lot pour toutes.** Une classe apporte une mécanique — rage, forme
  sauvage, magie de pacte — qui se code, se teste et se règle séparément. Le `LOT-47` ne porte donc
  que le socle commun et le guerrier ; les quinze autres classes ont chacune leur lot.
- **Le Colisée est le bac à sable de combat** (`LOT-50`), et il n'est pas un outil jetable : les
  Arènes sont une institution centrale de Tanares, leurs combats sont non létaux par la fiction
  même, et la zone sert telle quelle dans le jeu final.
- **La charte IHM est refondue** (`LOT-66` → `LOT-68`). L'identité **pixel art** imposée par
  `EX-IHM-070` était celle du jeu de plateforme d'origine et contredit frontalement une maquette de
  fiche à 300 ppp et des panneaux de parchemin peints. Références retenues : les feuilles de
  personnage pour la structure, les livres Tanares pour l'habillage. Les menus suivent — un monde
  ouvert n'a ni « niveau à choisir » ni « tableau à recommencer ».
- **L'éditeur devient un mode de l'arène** (`LOT-69`) : placement libre de décors, objets et PNJ
  depuis le jeu, sans application séparée. L'atelier pixel art est retiré, faute d'objet une fois
  l'identité pixel art abandonnée. Réoriente le [LOT-11](@ref lot-11), qui n'est pas encore livré.
- **Région de départ : la République des Freelands.** Ses propres statistiques l'argumentent :
  `Monster Presence` haute — les rencontres se justifient d'elles-mêmes, ce que le Central Empire,
  noté *Low*, ne permet pas ; économie fondée sur les mercenaires et les parts de monstres — le
  marchand et le donneur de quête n'ont rien à inventer ; villes de frontière — le village à
  construire existe dans la fiction ; faction Allied Forces, cadre héroïque plutôt qu'oppressif. Et
  surtout : c'est là que siège la **Guilde des Aventuriers**, donc la boucle du `LOT-45`.

- **Trois préconditions sont devenues des lots.** Écrire la moitié RPG de la spécification
  (`LOT-77`), désambiguïser les numéros hérités (`LOT-78`) et factoriser le chargement de données
  (`LOT-79`) étaient signalés comme préalables et portés par personne. Un préalable sans porteur
  n'est pas un préalable, c'est une dette.
- **Le découpage suit une règle unique : le code d'un côté, la donnée de l'autre.** C'est ce qui a
  scindé `LOT-37`/`LOT-80`, `LOT-40`/`LOT-81`, `LOT-41`/`LOT-82`, `LOT-45`/`LOT-83` et
  `LOT-47`/`LOT-84`. Les deux moitiés n'ont ni le même métier, ni le même critère d'acceptation, ni
  le même moment.
- **Le `LOT-11` vise d'emblée l'édition dans la scène.** Le `LOT-69` n'est plus une refonte de
  l'éditeur, seulement le retrait de l'atelier pixel art. Construire puis remplacer coûtait le
  double, et le `LOT-11` n'est pas commencé.

### À trancher

- **Profondeur du bac à sable.** Le `LOT-41` compose des quêtes par gabarit et le `LOT-45` les
  distribue par rang. Faut-il en rester à ce contenu déduit, ou écrire par-dessus quelques quêtes à
  la main dans les lieux notables ? La réponse change le poids du [LOT-16](@ref lot-16), pas
  l'architecture.
- **Le sort de `Documentation/Heritage/`.** 520 fichiers, 3,7 Mo, hors Doxygen, conservés par
  l'historique git de toute façon. Le `LOT-78` rend l'archive *citable* sans ambiguïté ; il ne
  tranche pas s'il faut la garder dans l'arbre de travail.

*(Le sort du *Manuel des Monstres* est tranché : voir ci-dessus.)*

---

## 9. Ce que l'audit a révélé sur l'état du dépôt

Cette page a été confrontée à l'état réel du code et des spécifications. Cinq constats la
dépassent et conditionnent son exécution.

### 9.1 Cinq familles d'exigences sont fantômes — le lint le dit désormais

Le dépôt compte **13 familles réelles pour 269 exigences** (`EX-ARCH`, `EX-BUILD`, `EX-CTRL`,
`EX-DEC`, `EX-EDIT`, `EX-EXP`, `EX-GP`, `EX-IA`, `EX-IHM`, `EX-LVL`, `EX-NFR`, `EX-REN`, `EX-VIS`).
Cinq autres sont référencées par une vingtaine d'epics et **n'existent pas** : `EX-DND-*`,
`EX-RPG-*`, `EX-INV-*`, `EX-CBT-*` et `EX-CNT-*`. Les documents censés les porter — `regles-dnd.md`,
`combat.md`, `rpg.md`, `contenu.md` — sont absents.

**Le garde-fou, lui, est réparé.** Il ne l'était pas : `scripts/lint_exigences.py` ne filtrait que
sur `EX-[A-Z]+-[0-9]+`, si bien qu'un `EX-DND-*` n'était **ni** une déclaration **ni** une
référence, et que la CI passait au vert sur cinq familles inexistantes. Une seconde expression,
`FAMILY_REF_RE = re.compile(r'EX-([A-Z]+)-\*')`, capte désormais les références de famille entière
et le lint **échoue** sur les cinq.

Cet échec est **voulu et il doit le rester** jusqu'à ce que les documents existent : c'est le seul
mécanisme qui empêche la dette de grossir en silence. Il s'éteint au `LOT-77`, et pas avant.

Écrire la **moitié RPG de la spécification** est donc le vrai chantier de fond, celui dont dépend
tout le reste — d'où un lot dédié plutôt qu'une ligne dans « à trancher ».

### 9.2 Six mécaniques étaient consommées sans être produites

Le repos était **invoqué par six lots et livré par aucun**. Le temps de jeu et le campement
n'existaient nulle part. Les conditions, l'agonie et l'expérience étaient mentionnées dans un
périmètre sans être ni spécifiées ni testées — et **aucune source d'expérience** n'était définie,
alors que trois lots en consomment.

D'où les `LOT-70` à `LOT-75`. Les rattacher au périmètre de lots existants aurait reproduit
exactement la cause du trou.

### 9.3 L'allègement est le geste à plus fort levier du programme

Les décisions prises rendent mort un volume de code supérieur à ce que les cinq lots suivants
ajouteront :

| Élément | Poids | Retiré par |
|---|---|---|
| Atelier de dessin pixel art (`Source/HMI/Editor/Pixel*`) | ~2 975 lignes | `LOT-69` |
| Widgets pixel art (`Source/HMI/Interface/Pixel*`) | ~640 lignes | `LOT-66` |
| Écrans de plateforme (`LevelSelectScreen`, `LevelCompleteScreen`) | 622 lignes | `LOT-67` |
| Maquettes `SelectionNiveau`, `FinDeNiveau` + clés `level.*` | — | `LOT-67` |

Soit **plus de 4 200 lignes**. Chaque suppression est portée par le lot qui la rend possible,
**jamais reportée** à un lot de ménage : du code mort qu'on garde « pour plus tard » se remet à
coûter dès la première refactorisation qui le traverse.

À décider à part : `Documentation/Heritage/` pèse **520 fichiers et 3,7 Mo** — l'archive du
programme de lots d'un autre projet, hors Doxygen. L'historique git la conserve de toute façon.

### 9.4 Le socle technique tient, mais trois préconditions manquent

Ce qui est sain : **927 tests verts** sur 129 fichiers, une CI durcie (`/W4 /WX`, `clang-tidy` et
Doxygen épinglés), un cœur déjà purgé de 63 000 lignes au [LOT-01](@ref lot-01) et déjà réorienté
vers le RPG top-down par les `LOT-06` et `LOT-07` — le `LOT-08` reste à faire. Le format v3
**porte déjà les entités**
(`MapEntity`), et `GridDistanceField` — un parcours en largeur sur la grille — a été explicitement
sauvé de la purge comme « le calcul de portée de déplacement du futur combat tactique ».

Ce qui manque avant d'ajouter des dizaines de catalogues :

- **Aucune brique de chargement JSON partagée.** Six réimplémentations identiques de `loadFromFile`
  (`SkinCatalog`, `AnimationCatalog`, `SoundCatalog`, `PixelPalette`, `LevelLoader`,
  `LevelSequence`), validation écrite à la main champ par champ, **aucun schéma**. Il faut
  factoriser ce patron, sinon la filière le duplique dix fois de plus.
- **Aucun test paramétré** dans tout `Source/Test/` — ni `TEST_P`, ni parcours de dossier de
  fixtures. Les lots de données en ont besoin : c'est une capacité à créer, pas à réutiliser.

  Ces deux manques étaient attribués au `LOT-32` par le tableau du §10, mais **absents de son
  périmètre**, qui ne livre que des schémas, un script Python et un test d'énumérations : personne
  ne portait le travail C++. C'est l'objet du `LOT-79`, désormais prérequis du `LOT-32`.
- **`Source/Elements/Levels/` est vide.** Le jeu n'a aucune carte à charger ; `loadDeliveredLevel`
  (`test_render_budget.cpp`) est du code mort et la fixture `rejeu-test-deplacement.json` est
  orpheline depuis la purge.

### 9.5 La collision de numéros est bloquante, et bien plus large qu'estimé

Le programme hérité allait jusqu'à `LOT-74`. En atteignant `LOT-84`, la filière **recouvre
entièrement** cette plage. Or les spécifications citent encore des lots hérités dans le corps de
leurs exigences — `gameplay.md` (« Complété en `LOT-65` »), `niveaux.md` (« `LOT-25`, étendu en
`LOT-65` »), `editeur-niveaux.md` (« `LOT-54` introduit un éditeur de texture »), les titres de
sections d'`interface-ihm.md`, et jusqu'à `architecture.md` (« Concrétisé en `LOT-33` »).

**Le décompte a été fait, et il corrige l'estimation initiale.** Les spécifications portent
**222 renvois `LOT-NN`, dont 208 ambigus** — tout numéro au-delà de `LOT-07`, puisque seuls les sept
premiers désignent à coup sûr un lot livré de ce dépôt. Ils se répartissent sur **douze fichiers**,
et non quatre :

| Fichier | Renvois ambigus |
|---|---|
| `editeur-niveaux.md` | 42 |
| `rendu-technique.md` | 34 |
| `interface-ihm.md` | 26 |
| `decors.md` | 21 |
| `gameplay.md` | 20 |
| `exigences-non-fonctionnelles.md` | 17 |
| `niveaux.md` | 15 |
| `controles.md` | 13 |
| `architecture.md` | 9 |
| `exploration.md` | 6 |
| `conventions.md` | 4 |
| `vision.md` | 1 |

Ces renvois deviennent **ambigus sans être cassés** : ni le lint ni Doxygen ne les signaleront. Et
ce n'est pas « une recherche-remplacement dans six specs » comme l'estimait le §10 : c'est une passe
sur 208 références dont chacune doit être **classée** — lot hérité ou lot courant — avant d'être
préfixée `LOT-H-XX` ou laissée telle quelle. D'où un lot à part entière, le `LOT-78`, prérequis de
la création du premier dossier au-delà de `LOT-29`.

---

## 10. Ce qu'il faut anticiper, tant que rien n'est construit

**Sept lots sont livrés sur les quatre-vingts que compte le programme.** Presque tout ce que cette filière modifie n'existe pas
encore : ni la fiche de personnage, ni l'inventaire, ni le combat, ni l'éditeur multi-couches, ni le
graphe de cartes. C'est un avantage considérable et **temporaire** — chaque décision prise
maintenant coûte une écriture ; la même, prise dans dix lots, coûte une réécriture plus la migration
de ce qui s'est construit dessus.

Huit choses méritent donc d'être décidées avant, et non après.

| À anticiper | Coût aujourd'hui | Coût si on attend |
|---|---|---|
| **Retirer les exigences de plateforme** (`EX-GP-030/031/032`, `EX-LVL-010`→`015`, `EX-IHM-003/004/005`) — porté par le `LOT-67` | Une passe de rédaction : elles ne décrivent **rien de construit** pour le RPG | Les `LOT-09`→`17` se bâtissent dessus, puis il faut les défaire |
| **La charte visuelle** (`LOT-66`) | Seuls les écrans hérités existent | Chaque écran RPG livré entre-temps est à refaire |
| **Viser directement l'édition dans la scène** au `LOT-11` au lieu de la refondre au `LOT-69` | Le `LOT-11` n'est pas commencé : il suffit qu'il vise cette cible | Construire l'éditeur multi-couches, puis le remplacer |
| **La brique de chargement JSON** (`LOT-79`) | Six lecteurs à factoriser | Vingt et plus, chacun avec sa validation manuscrite |
| **Les tests paramétrés** (`LOT-79`) | Une capacité à créer, sur une suite encore petite | À créer quand même, mais avec des dizaines de tests déjà écrits autrement |
| **L'horloge et le repos** (`LOT-70`) | Le `LOT-13` n'existe pas : la fiche peut naître en déclarant ses ressources et leur cadence | Rétro-adapter la fiche, puis les quinze classes |
| **Les champs `"source"` et `"statut"` au schéma** (`LOT-32`) | Une ligne, avant le premier catalogue | Une migration de tous les catalogues livrés |
| **Désambiguïser les numéros hérités** (`LOT-78`) | 208 renvois à classer dans douze specs — un lot, pas une passe de relecture | Ambiguïté silencieuse, invisible au lint comme à Doxygen |

Deux d'entre elles sont plus que des économies.

**Le `LOT-11` ne doit pas être construit puis refondu.** Le `LOT-69` était écrit comme une refonte
parce qu'il a été conçu après ; mais l'éditeur multi-couches n'étant pas commencé, la bonne décision
est que le `LOT-11` **vise d'emblée** l'édition dans la scène. Le `LOT-69` a donc été **réécrit en
conséquence** : il ne reste que le retrait de l'atelier de dessin, qui est une suppression, pas une
réécriture — et il n'a plus besoin ni du Colisée ni du socle de classe pour cela.

**`MapEntity` existe déjà et n'alimente rien.** Le format v3 porte `{ type, position, properties }`
avec un contrat explicite — « `Core` ne connaît aucune sémantique de `type` » — mais aucun
consommateur hors des tests : ni le rendu, ni l'ECS, ni l'éditeur. Le `LOT-10` (entités et
interaction) est le premier à pouvoir s'y brancher, et il doit le faire plutôt que d'inventer un
second conteneur d'entités à côté.

---

## 11. Les lots `LOT-08` à `LOT-29`

Ces vingt-deux lots avaient chacun leur dossier `LOT-NN-nom/`. Ils sont absorbés ici pour que
la feuille de route ait **une seule source de vérité** : deux documents décrivant le même
programme divergent, et l'audit a montré qu'ils avaient déjà commencé à le faire.

Leurs **ancres Doxygen sont conservées** (`{#lot-09}`, `{#lot-13}`…), si bien que tous les
renvois `@ref lot-NN` des spécifications continuent de résoudre. Seuls les `@subpage` de
`lots.md` disparaissent, faute de pages séparées.

> **Contenu d'origine, corrections signalées.** Le texte de chaque epic est repris tel quel.
> Là où l'audit l'a contredit — l'éditeur du `LOT-11`, la mort du `LOT-21`, le repos du
> `LOT-25` — la correction figure dans les sections 5, 9 et 10 ci-dessus, qui font foi.

### LOT-08 — Vocabulaire de tuiles RPG {#lot-08}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (les couches donnent leur sens aux types).

#### Objectif

Donner à la palette les types de terrain d'un RPG en vue de dessus, là où le `LOT-01` a laissé le
strict minimum hérité (11 types).

#### Périmètre

Types à ajouter dans `core::TileType`, à peu près : `Grass`, `Dirt`, `Sand`, `Water`, `DeepWater`,
`Wall`, `Cliff`, `Bridge`, `Stairs`. La liste exacte se décide au contact du level design, pas
d'avance.

Pour **chaque** type ajouté, la chaîne complète — c'est la leçon la plus chère de l'héritage, où
ajouter un type touchait « exactement la même chaîne de huit fichiers » :

1. `TileType.h` (avant le dernier énumérateur, `TILE_TYPE_COUNT` suit tout seul) ;
2. `TileTypeName.cpp` (le `switch` est exhaustif et sans `default` : le compilateur désigne
   lui-même ce qu'il reste à faire) ;
3. `Editor/TileTaxonomy.cpp` (catégorie de palette) et `TaxonomyLabels.cpp` ;
4. libellés `fr.lang` **et** `en.lang` ;
5. `ProceduralAtlas` : **rendu de repli déterministe**, dans le même lot, jamais « plus tard » —
   c'est ce qui garde le jeu lançable sans aucun fichier d'image ;
6. `TileAutotile` si le type a des raccords ;
7. `TileSilhouette` si sa matière n'occupe pas toute la case (falaises, bords d'eau, ponts) — le
   `LOT-01` a **conservé ce mécanisme vidé** précisément pour ce lot ;
8. test d'aller-retour chargeur/écrivain.

#### Note de conception

Le `LOT-01` a supprimé les 25 types de plateforme mais gardé les 11 génériques (`Empty`, `Solid`,
`Danger`, `Entry`, `Exit`, `Switch`, `Door`, `PressurePlate`, `Block`, `Key`, `LockedDoor`) : le
vocabulaire de puzzle sert tel quel au RPG. Ce lot **ajoute**, il ne remplace pas.

#### Exigences couvertes

`EX-EXP-*`, `EX-EDIT-*`, `EX-REN-*`.

#### Critères d'acceptation

- Chaque nouveau type a : un libellé fr/en, une classe de palette, un rendu procédural de repli, et
  un test d'aller-retour.
- Le jeu se lance et affiche une carte **sans aucun fichier d'image** présent.
- `TILE_TYPE_COUNT` reste dérivé du dernier énumérateur — aucune borne recopiée à la main.

### LOT-09 — Graphe de cartes et transitions {#lot-09}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04), [LOT-06](@ref lot-06).

#### Objectif

Remplacer la **séquence linéaire** de tableaux héritée par un **monde de cartes connectées**, où
chaque carte a des sorties nommées vers d'autres cartes, et où l'on peut revenir sur ses pas.

#### Le problème

`core::LevelSequence` est une **liste ordonnée** : tableau 1, puis 2, puis 3. C'est la structure
d'un jeu de niveaux discrets, pas d'un monde. Un RPG a besoin d'un **graphe** : un village ouvre
sur une forêt et sur une auberge, la forêt ramène au village, le donjon a trois entrées.

#### Périmètre

- `Source/Core/World/WorldGraph.{h,cpp}` : cartes, portails, points d'arrivée **nommés**. Un
  portail référence `(carte cible, nom du point d'arrivée)` — jamais des coordonnées brutes, qui
  se désynchroniseraient au moindre redimensionnement de la carte cible.
- Chargement de carte **à chaud**, sans repasser par un écran de sélection.
- Transition visuelle (fondu) côté `hmi::ScreenFlow`.
- **Validation au chargement** : un portail dont la carte ou le point d'arrivée n'existe pas est
  une erreur explicite, pas un plantage à la traversée (`EX-NFR-040`).

`LevelSequence` disparaît ou devient un cas dégénéré du graphe — à trancher à l'implémentation.

#### Exigences couvertes

`EX-EXP-*` (portails, points d'arrivée nommés), `EX-LVL-*` (validation du graphe).

#### Critères d'acceptation

- A → B → A ramène le personnage **au bon point d'arrivée**, pas à l'entrée par défaut de A.
- Un portail orphelin (carte inconnue, point d'arrivée inconnu) est refusé au chargement avec un
  message exploitable.
- Test headless d'un parcours de cinq cartes, aller et retour.
- L'état de la carte quittée (coffres ouverts, ennemis vaincus) est **conservé** au retour — c'est
  ce qui distingue un monde d'une séquence de tableaux. Persisté au `LOT-17`.

### LOT-10 — Entités de carte et interaction {#lot-10}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (couche `objects`), [LOT-06](@ref lot-06) (orientation du
> personnage), [LOT-08](@ref lot-08).

#### Objectif

Peupler les cartes d'entités qui ne sont **pas des tuiles** — PNJ, coffres, panneaux, portails,
déclencheurs — et permettre au joueur d'interagir avec elles.

#### Pourquoi des entités et pas des tuiles

Une tuile, c'est un type par case et rien d'autre : la grille ne porte aucune métadonnée. Le
platformer avait contourné cela avec des vecteurs annexes indexés par position — le `LOT-01` en a
retiré quatre. Un PNJ a un nom, un dialogue, un inventaire ; un coffre a un contenu et un état
« déjà ouvert ». Cela demande une **liste d'objets à propriétés**, ce que la couche `objects` du
format v3 fournit.

#### Périmètre

- Instanciation ECS depuis la couche `objects` (`LevelScene`), une entité par objet, avec ses
  propriétés libres.
- **Cible d'interaction** : la case devant l'orientation du personnage. `core::PlayerInput` porte
  déjà `interactPressed/Held/Released` — rien à câbler côté entrée.
- Invite visuelle quand une cible est à portée (le joueur doit savoir qu'il *peut* interagir).
- Coffre : contenu, état consommé. Panneau : texte localisé.

#### Le piège

Un coffre ouvert deux fois ne doit donner son butin **qu'une fois**, et cet état doit survivre au
départ puis au retour sur la carte (`LOT-09`) et à la sauvegarde (`LOT-17`). C'est un drapeau de
monde, pas un booléen local à l'entité rechargée.

#### Exigences couvertes

`EX-EXP-*` (interaction, portée, priorité de cible), `EX-RPG-*` (coffre, panneau).

#### Critères d'acceptation

- Ouvrir un coffre deux fois ne donne le butin qu'une fois, y compris après aller-retour de carte.
- L'interaction **ne traverse pas un mur**.
- Quand deux cibles sont à portée, celle désignée est déterministe et prévisible (la plus proche du
  centre de la case visée).

### LOT-11 — Éditeur multi-couches et placement d'entités {#lot-11}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04), [LOT-08](@ref lot-08), [LOT-10](@ref lot-10).

#### Objectif

Rendre l'éditeur hérité capable d'éditer les **trois couches** et de poser des **entités** avec
leurs propriétés — c'est-à-dire de produire le contenu du RPG sans écrire de JSON à la main.

#### Ce qui est déjà là

L'éditeur du dépôt d'origine est un poste de travail complet, et il est **entièrement conservé** :
peinture à la souris, outils rectangle et sélection, liaison de mécanismes, undo/redo, essai
immédiat, bibliothèque d'assets à vignettes avec rechargement à chaud et détection des références
avant suppression, atelier pixel art intégré. Ce lot **retarge**, il ne reconstruit pas.

Le `LOT-01` a retiré l'outil « Parcours » et le panneau « Propriétés », qui ne pilotaient que des
réglages de plateforme.

#### Périmètre

- **Sélecteur de couche active** ; visibilité et opacité par couche (voir le sol sous le décor).
- Outil **« poser entité »** et panneau de propriétés d'entité (nouveau panneau, remplaçant celui
  retiré au `LOT-01`).
- Pose et édition des **portails** (carte cible, point d'arrivée) et des **points d'apparition**.
- Undo/redo étendu à tout cela — l'éditeur n'a jamais eu de mutation non annulable, cette règle
  tient.
- Vue **graphe du monde** dans le navigateur de cartes.

#### La contrainte à rendre visible ici

Le combat se déroulant **sur la carte d'exploration** (décision de cadrage, cf.
`Documentation/Specification/vision.md`), *toute carte doit être un terrain tactique valide*.
L'éditeur doit **avertir** quand une zone est trop étroite ou trop encombrée pour y poser une
rencontre. Découvrir la contrainte au `LOT-18`, une fois vingt cartes dessinées, coûterait leur
reprise.

#### Exigences couvertes

`EX-EDIT-*`.

#### Critères d'acceptation

- Édition des trois couches avec undo/redo complet.
- Pose d'un PNJ, d'un coffre et d'un portail, avec leurs propriétés, puis essai immédiat.
- Aucune régression sur l'édition existante (peinture, sélection, liaisons, atelier pixel art).
- Avertissement visible quand une zone de rencontre n'est pas un terrain tactique valide.

### LOT-12 — Dés, caractéristiques, jets {#lot-12}

> Statut : **à faire**.
> Prérequis : **aucun**. Pur `Core`, zéro dépendance — **parallélisable dès le `LOT-01`**.

#### Objectif

Implémenter le cœur chiffré du système : dés déterministes, six caractéristiques, modificateurs,
jet d20 avec avantage et désavantage contre un degré de difficulté ou une classe d'armure.

#### Pourquoi ce lot peut démarrer tout de suite

Il ne dépend ni du format de carte, ni du mode de jeu, ni du rendu. C'est du **calcul pur**, dans
`Core`, testable unitairement sans fenêtre ni GPU. C'est le meilleur candidat à une exécution en
parallèle du socle technique.

#### Périmètre

- `Source/Core/Rpg/Dice.{h,cpp}` : notation `2d6+3` (parseur et évaluation).
- `Ability.h` : les six caractéristiques, modificateur `(score - 10) / 2` **arrondi vers le bas**
  (attention aux scores impairs inférieurs à 10 : `7 → -2`, pas `-1`).
- `Check.{h,cpp}` : jet d20 contre difficulté, **avantage** (max de deux d20), **désavantage** (min
  de deux d20), détection des 1 et 20 naturels.
- Fige l'**échelle 1 case = 1,5 m** (décision de cadrage) comme constante nommée : seule source de
  vérité des portées et vitesses, consommée par les `LOT-19` et `LOT-22`.

#### Le socle déterministe

`Source/Core/Math/DeterministicRandom.h` fournit déjà splitMix64 et
`deriveSeed(baseSeed, step, entityId)`. **Il lui manque un `nextInt(min, max)`** — à ajouter ici,
sans biais modulo.

Le déterminisme n'est pas un confort : sans lui, **aucun test de combat n'est écrivable**. C'est
`EX-NFR-002`, qui prend avec ce lot une importance qu'il n'avait pas dans le platformer.

#### Exigences couvertes

Catégorie `EX-DND-*`, déclarée par ce lot dans `Documentation/Specification/regles-dnd.md`.

#### Critères d'acceptation

- Distribution vérifiée statistiquement sur 100 000 tirages à graine fixe (uniformité, bornes).
- **Rejouabilité stricte** : même graine → même séquence, y compris après sérialisation de l'état.
- Avantage = max de deux d20, désavantage = min ; les deux ensemble s'annulent.
- 1 et 20 naturels détectés et distingués d'un total de 1 ou 20.
- Aucune dépendance ECS, Qt ou rendu : `Core/Rpg/` compile seul.

### LOT-13 — Fiche de personnage {#lot-13}

> Statut : **à faire**.
> Prérequis : [LOT-12](@ref lot-12).

#### Objectif

Donner à toute créature — héros, PNJ, ennemi — une fiche complète : points de vie, classe
d'armure, classe, niveau, maîtrise, compétences, vitesse.

#### Périmètre

- `Source/Core/Rpg/CharacterSheet.{h,cpp}` : caractéristiques, PV courants et maximum, CA, niveau,
  bonus de maîtrise, jets de sauvegarde, vitesse (en cases, dérivée de l'échelle du `LOT-12`).
- `ClassDefinition.{h,cpp}` : **pilotée par JSON**, jamais codée en dur. Dé de vie, maîtrises,
  progression par niveau.
- `Skill.h` : compétences et leur caractéristique associée.
- Composant ECS `Source/Core/Ecs/Components/RpgActor.h` référençant la fiche.
- Expérience et montée de niveau.

#### Deux règles à tenir

**`CharacterSheet` est un objet autonome, jamais un singleton joueur.** La décision de cadrage est
« un héros au départ, quatre à terme » : le passage au groupe (`LOT-29`) ne doit **rien** changer à
cette classe. Si une fonction prend « le personnage » implicitement, elle est mal écrite.

**Aucune valeur de règle dans le C++** (`EX-VIS-007`). Dé de vie, seuils d'expérience, maîtrises :
tout en JSON. C'est ce qui rend l'équilibrage possible sans recompiler — et sans équilibrage, un
RPG n'est pas jouable.

#### Exigences couvertes

`EX-DND-*` : PV, CA, bonus de maîtrise par niveau, jets de sauvegarde, expérience, progression.

#### Critères d'acceptation

- Trois classes définies en JSON se chargent et donnent les bons modificateurs.
- Montée de niveau reproductible et testée aux bornes (seuil exact, dépassement, multi-niveaux).
- **Aucune valeur de règle codée en dur** dans le C++ — vérifiable par relecture du diff.
- Un test construit **quatre** fiches indépendantes : rien ne suppose l'unicité.

### LOT-14 — Inventaire et équipement {#lot-14}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13).

#### Objectif

Porter, équiper et consommer des objets, avec un effet **mesurable** sur la fiche.

#### Périmètre

- `Source/Core/Rpg/Item.{h,cpp}` : type, poids, valeur, effets. Catalogue en **JSON**.
- `Inventory.{h,cpp}` : contenu, capacité de charge.
- `Equipment.{h,cpp}` : emplacements (arme principale, arme secondaire, armure, accessoires).
- Statistiques **dérivées** : armure → CA, arme → dés de dégâts et caractéristique d'attaque.

#### Le piège

**Retirer un équipement doit annuler exactement son effet.** L'erreur classique est d'appliquer un
bonus en additionnant à la volée (`ca += 2`) : après trois équipements et deux retraits dans le
désordre, la CA a dérivé. La CA doit être **recalculée** depuis l'équipement porté, jamais
accumulée.

C'est un critère d'acceptation, pas une préférence de style.

#### Exigences couvertes

`EX-INV-*`, `EX-DND-*` (statistiques dérivées).

#### Critères d'acceptation

- Équiper une armure change la CA du montant attendu ; la retirer **restitue exactement** la valeur
  d'origine, quel que soit l'ordre des opérations.
- Un test équipe et retire dans plusieurs ordres et vérifie l'absence de dérive.
- Capacité de charge respectée, avec un comportement défini au dépassement.
- Catalogue d'objets entièrement en données.

### LOT-15 — PNJ et dialogues {#lot-15}

> Statut : **à faire**.
> Prérequis : [LOT-05](@ref lot-05) (mode « monde gelé »), [LOT-10](@ref lot-10) (entités et
> interaction), [LOT-12](@ref lot-12) (jets de compétence en dialogue).

#### Objectif

Parler à un PNJ via un arbre de dialogue scripté, avec choix, conditions et jets de compétence.

#### Périmètre

- `Source/Core/Rpg/Dialogue.{h,cpp}` : graphe de nœuds en **JSON** — réplique, choix, condition sur
  drapeau, action (donner un objet, poser un drapeau, démarrer une quête), **jet de compétence**
  (ex. Persuasion contre difficulté 15, avec deux suites possibles).
- `DialogueRunner` : machine à états **pure**, testable headless. Elle ne connaît ni Qt, ni le
  rendu, ni le monde — elle consomme un graphe et une source de drapeaux, elle produit un nœud
  courant et des choix.
- `DialogueMode` (`LOT-05`) : le monde est **gelé** pendant la conversation.
- Widget Qt sur `DesignTokens` / `PixelFrameWidget` / `BitmapFont`.
- Tout le texte via `hmi::Localization` — **aucun libellé en dur** (`EX-REN-033`).

#### Ce qui fait la qualité de ce lot

La séparation runner pur / widget. Un dialogue doit être **jouable en test headless**, nœud par
nœud, sans fenêtre : c'est ce qui permettra de vérifier des arbres de vingt nœuds sans les cliquer
à la main. Le widget ne fait qu'afficher ce que le runner décide.

#### Exigences couvertes

`EX-RPG-*` (dialogue, choix, conditions, actions), `EX-IHM-*`.

#### Critères d'acceptation

- Un dialogue de dix nœuds, deux conditions et un jet de Persuasion se parcourt **en headless**.
- Un graphe mal formé (nœud cible inconnu, cycle non intentionnel, choix vide) est **rejeté au
  chargement** avec un message exploitable, pas découvert en jeu.
- L'exploration est **gelée** pendant le dialogue : aucun pas de simulation d'exploration consommé.
- Traduction fr/en complète, aucun texte en dur.

### LOT-16 — Quêtes et drapeaux de monde {#lot-16}

> Statut : **à faire**.
> Prérequis : [LOT-15](@ref lot-15) (les dialogues déclenchent et font avancer les quêtes).

#### Objectif

Suivre l'avancement du joueur par des **drapeaux persistants**, et en donner une lecture dans un
journal de quêtes.

#### Périmètre

- `Source/Core/Rpg/WorldFlags.{h,cpp}` : ensemble de drapeaux nommés, posés et lus par les
  dialogues, les entités de carte et les rencontres. C'est le **substrat** : coffre déjà ouvert
  (`LOT-10`), ennemi vaincu (`LOT-18`), PNJ déjà rencontré.
- `Quest.{h,cpp}` : étapes, conditions d'avancement, récompenses. Défini en **JSON**.
- Journal de quêtes (contenu ; l'écran vient au `LOT-24` ou avec l'IHM RPG).
- Persistance dans `SaveGame` (`LOT-17`).

#### Note de conception

Les drapeaux de monde et les quêtes sont deux niveaux du même mécanisme : une quête **lit** des
drapeaux pour décider de son avancement, elle n'a pas d'état propre au-delà de son étape courante.
Cette séparation évite le piège classique où l'état du monde existe en double — une fois dans les
entités, une fois dans les quêtes — et diverge.

#### Exigences couvertes

`EX-RPG-*`.

#### Critères d'acceptation

- Une quête à trois étapes se déclenche, progresse et se termine.
- Elle **survit à une sauvegarde et un rechargement** à n'importe laquelle de ses étapes.
- Un drapeau posé par un dialogue est lu par une entité de carte, et inversement.

### LOT-17 — Sauvegarde riche {#lot-17}

> Statut : **à faire**.
> Prérequis : [LOT-09](@ref lot-09), [LOT-10](@ref lot-10), [LOT-13](@ref lot-13),
> [LOT-16](@ref lot-16).

#### Objectif

Remplacer la progression héritée — qui ne retenait qu'un **tableau atteint** — par un état de
partie complet et versionné.

#### Le problème

`hmi::Progression` stocke un identifiant de séquence, un nom de tableau courant et un ensemble de
tableaux terminés. C'était suffisant pour un jeu de niveaux discrets ; c'est sans rapport avec un
monde persistant où le joueur a une position, un inventaire, des quêtes en cours et un monde qui a
gardé la trace de ses actes.

#### Périmètre

`Source/Core/Rpg/SaveGame.{h,cpp}` : JSON **versionné**, portant

- la carte courante et la position exacte du personnage ;
- la **liste** des personnages (décision de cadrage : un héros au départ, quatre à terme — c'est
  une liste dès maintenant, pas un champ unique qu'on pluraliserait plus tard) ;
- l'inventaire et l'équipement ;
- les drapeaux de monde et les entités consommées (coffres, ennemis vaincus) ;
- les quêtes et leur étape.

`hmi::Progression` disparaît ou devient un cas dégénéré.

#### Règles de format

Mêmes règles que le format de carte, pour les mêmes raisons : **versionné**, **tolérant aux champs
inconnus**, migration ascendante. Une sauvegarde est la donnée que le joueur ne peut pas
reconstruire — un format qui casse lui fait perdre sa partie.

#### Exigences couvertes

`EX-RPG-*` (sauvegarde, chargement, versionnement, tolérance).

#### Critères d'acceptation

- Aller-retour sauvegarde → chargement **à l'identique**, sur un état riche (plusieurs quêtes en
  cours, inventaire garni, plusieurs cartes visitées).
- Une sauvegarde d'une version antérieure se charge avec des valeurs par défaut sensées.
- Un champ inconnu est ignoré **et préservé** à la réécriture.
- Testable headless.

### LOT-18 — Bascule exploration ↔ combat {#lot-18}

> Statut : **à faire**.
> Prérequis : [LOT-05](@ref lot-05) (modes de jeu), [LOT-10](@ref lot-10) (déclencheurs),
> [LOT-13](@ref lot-13) (fiches des combattants).

#### Objectif

Déclencher une rencontre depuis l'exploration, geler le monde, monter la grille de combat, et en
revenir — sans que le joueur perde quoi que ce soit au passage.

#### La décision qui structure ce lot

Le combat se déroule **sur la carte d'exploration**, jamais dans une arène chargée à part
(décision de cadrage, cf. `Documentation/Specification/vision.md`). La grille tactique est
**dérivée de la couche collision** du format v3 (`LOT-04`).

C'est ce qui donne la continuité visuelle d'un Chrono Trigger plutôt que la rupture d'un JRPG
classique — et c'est ce qui justifie rétroactivement le soin mis au format de carte en phase B.
En contrepartie, *toute carte doit être un terrain tactique valide* : contrainte de level design
signalée dans l'éditeur dès le `LOT-11`.

#### Périmètre

- `CombatMode` : le second mode de `LOT-05`.
- `Source/Core/Combat/Encounter.{h,cpp}` : définition **JSON** — ennemis, positions de départ
  (relatives au déclencheur), conditions.
- Déclencheurs : contact avec un ennemi de carte, zone de rencontre, action de dialogue.
- **Sauvegarde et restauration de l'état d'exploration** : position, orientation, caméra, entités.

#### Le critère qui compte

Entrer puis sortir d'un combat doit restituer **exactement** l'état d'exploration — aux PV près,
qui eux ont changé. Un ennemi vaincu est retiré de la carte **durablement** : c'est un drapeau de
monde (`LOT-16`), persisté (`LOT-17`), pas un booléen local perdu au rechargement de la carte.

#### Exigences couvertes

Catégorie `EX-CBT-*`, déclarée par ce lot dans `Documentation/Specification/combat.md`.

#### Critères d'acceptation

- Aller-retour exploration → combat → exploration restituant l'état, PV mis à jour.
- Un ennemi vaincu ne réapparaît pas, y compris après avoir quitté et rechargé la carte.
- Une fuite ramène à l'exploration sans que l'ennemi soit marqué vaincu.
- Testable headless : le montage et le démontage d'une rencontre ne demandent ni fenêtre ni GPU.

### LOT-19 — Grille tactique et déplacement {#lot-19}

> Statut : **à faire**.
> Prérequis : [LOT-18](@ref lot-18).

#### Objectif

Poser la grille de combat, l'occupation des cases, et le calcul du déplacement par budget.

#### Ce qui existe déjà — à ne pas réécrire

**`core::GridDistanceField`** (`Source/Core/World/GridDistanceField.h`) : champ de distances de
plus court chemin sur la grille, BFS 4-voisins **multi-source**, lectures `O(1)`, avec sentinelle
d'inatteignabilité. Il vient du solveur d'IA du dépôt d'origine ; le `LOT-01` l'a **délibérément
sauvé** de la purge et déplacé dans `Core/World/` **pour ce lot précis**.

C'est exactement le calcul « quelles cases puis-je atteindre avec ce budget de déplacement ».

#### Périmètre

- `Source/Core/Combat/BattleGrid.{h,cpp}` : occupation des cases, terrain difficile, obstacles
  **issus de la couche collision** du format v3 (`LOT-04`).
- Portée de déplacement via `GridDistanceField`, budget dérivé de la vitesse de la fiche
  (`LOT-13`), à l'échelle 1 case = 1,5 m figée au `LOT-12`.
- `Pathfinding.{h,cpp}` : A* **déterministe**, avec départage stable des égalités.

#### Pourquoi le déterminisme du chemin n'est pas négociable

Deux chemins de même coût existent presque toujours sur une grille. Si le départage dépend de
l'ordre d'itération d'un conteneur non ordonné, l'IA (`LOT-23`) devient irreproductible et
**aucun test de combat ne tient**. Le départage doit être une règle explicite (par exemple : plus
petit indice de case), pas un hasard d'implémentation.

#### Exigences couvertes

`EX-CBT-*` (grille, portée de déplacement, chemin déterministe).

#### Critères d'acceptation

- Même entrée → **même chemin**, systématiquement.
- L'ensemble des cases atteignables correspond **exactement** au budget de déplacement, ni une de
  plus ni une de moins (tests aux bornes).
- Deux créatures ne partagent **jamais** une case.
- Le terrain difficile double le coût, et la portée s'en trouve réduite en conséquence.

### LOT-20 — Initiative et tour par tour {#lot-20}

> Statut : **à faire**.
> Prérequis : [LOT-12](@ref lot-12) (jet d'initiative), [LOT-19](@ref lot-19).

#### Objectif

Ordonnancer les combattants par initiative et structurer le tour : mouvement, action, action bonus,
réaction.

#### Périmètre

- `Source/Core/Combat/TurnOrder.{h,cpp}` : ordre d'initiative, **multi-alliés dès maintenant**
  (décision de cadrage : un héros au départ, quatre à terme — l'ordre n'a aucune raison de
  supposer un seul allié, même si le contenu n'en propose qu'un).
- `CombatState.{h,cpp}` : machine à états **explicite** — début de round, tour actif, fin de tour,
  fin de combat. Pas de drapeaux épars : un état nommé, des transitions nommées.
- Égalités d'initiative : départage déterministe et documenté.
- Entrée et sortie d'un combattant **en cours** de combat (renfort, fuite).

#### Les trois fins

Un combat se termine de trois façons, et les trois doivent être couvertes :

- **victoire** : tous les ennemis à 0 PV ;
- **défaite** : tous les alliés à 0 PV ;
- **fuite** : les alliés quittent la zone.

Une machine à états qui n'en couvre que deux laisse un combat qui ne finit jamais — le pire des
défauts, parce qu'il bloque le joueur sans message d'erreur.

#### Exigences couvertes

`EX-CBT-*` (initiative, structure du tour, conditions de fin).

#### Critères d'acceptation

- Un combat à cinq combattants se déroule **en headless** du premier round à une condition de fin.
- Les **trois** conditions de fin sont couvertes par un test chacune.
- Un test monte **quatre alliés** : rien dans l'ordre d'initiative ni dans la machine à états ne
  suppose un héros unique.
- Rejeu à graine fixe strictement reproductible, ordre d'initiative compris.

### LOT-21 — Attaques, dégâts et états {#lot-21}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13), [LOT-14](@ref lot-14), [LOT-20](@ref lot-20).

#### Objectif

Résoudre une attaque au d20 contre la classe d'armure, appliquer les dégâts, gérer critiques,
inconscience et mort.

#### Périmètre

- `Source/Core/Combat/Attack.{h,cpp}` : jet d'attaque = d20 + modificateur de caractéristique +
  bonus de maîtrise, contre la CA de la cible.
- `Damage.{h,cpp}` : types de dégâts, résistances, vulnérabilités, immunités.
- `Condition.{h,cpp}` : à terre, entravé, empoisonné, inconscient — et leur effet sur les jets.
- Jets de sauvegarde contre la mort.

#### Les règles à ne pas se tromper

- **Un 20 naturel double les dés de dégâts, pas le modificateur.** L'erreur inverse est la plus
  répandue et fausse tout l'équilibrage.
- **Un 1 naturel rate toujours**, quel que soit le total.
- Les PV sont **bornés à 0** par le bas : pas de PV négatifs qui rendraient la réanimation
  arbitraire.

#### Le journal de combat

Chaque jet doit être **auditable** : jet brut, modificateurs détaillés, cible, total, résultat.
« Tu as raté » n'est pas une information ; « 7 + 3 = 10 contre CA 15 : raté » en est une. C'est ce
qui permet au joueur de comprendre le système, et au développeur de déboguer l'équilibrage sans
attacher un débogueur.

#### Exigences couvertes

`EX-DND-*` (résolution chiffrée) et `EX-CBT-*` (application en combat).

#### Critères d'acceptation

- Un 20 naturel double **les dés** et non le modificateur, vérifié par test.
- Un 1 naturel rate, même avec un total supérieur à la CA.
- PV bornés à 0 ; inconscience et jets de sauvegarde contre la mort testés aux bornes.
- Résistances, vulnérabilités et immunités appliquées dans le bon ordre.
- Chaque jet produit une entrée de journal complète et lisible.

### LOT-22 — Portée, ligne de vue et zones d'effet {#lot-22}

> Statut : **à faire**.
> Prérequis : [LOT-19](@ref lot-19).

#### Objectif

Rendre la **géométrie** tactique signifiante : portée d'arme, ligne de vue bloquée, couverture,
gabarits d'effet de zone.

C'est ce lot qui fait qu'un combat se joue avec le terrain plutôt que sur une grille vide.

#### Périmètre

- `Source/Core/Combat/LineOfSight.{h,cpp}` : tracé sur grille, obstacles issus de la **couche
  collision** du format v3 (`LOT-04`).
- **Couverture** partielle et totale, traduite en bonus de CA.
- `AreaOfEffect.{h,cpp}` : gabarits cercle, cône, ligne, carré — à l'échelle 1 case = 1,5 m figée
  au `LOT-12`.
- Portée d'arme et portée de sort, distinguées de la portée de déplacement.

#### L'invariant à tester exhaustivement

**La ligne de vue doit être symétrique** : A voit B si et seulement si B voit A. C'est le défaut
classique des tracés sur grille — un algorithme qui part de A et s'arrête au premier obstacle ne
donne pas le même résultat en partant de B, et le joueur découvre qu'il peut tirer sur un ennemi
qui ne peut pas riposter (ou l'inverse, plus rageant encore).

À vérifier **exhaustivement sur des grilles générées**, pas sur trois cas choisis à la main.

#### Exigences couvertes

`EX-CBT-*` (portée, ligne de vue, couverture, gabarits).

#### Critères d'acceptation

- **Symétrie de la ligne de vue** vérifiée exhaustivement sur des grilles générées.
- Les gabarits couvrent exactement les cases attendues, figées par des cas de référence.
- La couverture modifie la CA du montant prévu, et ne s'applique jamais deux fois.
- Testable headless.

### LOT-23 — IA tactique ennemie {#lot-23}

> Statut : **à faire**.
> Prérequis : [LOT-20](@ref lot-20), [LOT-21](@ref lot-21), [LOT-22](@ref lot-22).

#### Objectif

Donner aux ennemis un comportement de combat crédible, **déterministe** et testable sans GPU.

#### La décision de conception

**Heuristiques pondérées, pas de réseau de neurones.**

Le dépôt d'origine embarquait 12 000 lignes de solveur RL (tenseurs, autodiff, réseaux,
optimiseurs, quatre familles d'algorithmes), retirées au `LOT-01`. La tentation serait de les
ressusciter ici : c'est le seul endroit du programme où elles auraient pu resservir.

Il ne faut pas. Une IA tactique par heuristiques est **plus prévisible, plus déboguable et bien
moins coûteuse** ; surtout, le déterminisme est ici un **critère d'acceptation**, pas une
préférence — sans lui, aucun test de combat ne tient. Un agent entraîné rendrait chaque
régression irreproductible.

#### Périmètre

- `Source/Core/Combat/EnemyAi.{h,cpp}` : évaluation de cibles pondérée (distance, PV restants,
  menace, portée), choix de position, choix d'action.
- **Profils de comportement en JSON** : agressif, prudent, soutien, archer. Les poids sont des
  données, pas des constantes C++ (`EX-VIS-007`) — c'est ce qui permet d'ajuster un ennemi sans
  recompiler.

#### Les deux défauts à prévenir

- **Le blocage.** Une IA qui ne trouve aucune action valide et passe son tour indéfiniment gèle le
  combat. Un test IA contre IA doit toujours **terminer**.
- **Le suicide.** Une IA qui ne pondère que la distance finit son tour au milieu de trois ennemis à
  bas PV. La menace subie doit peser dans le choix de position.

#### Exigences couvertes

`EX-CBT-*` (choix de cible, de position et d'action).

#### Critères d'acceptation

- Un combat IA contre IA **se termine toujours**, sur un échantillon de configurations générées.
- Rejeu à graine fixe strictement reproductible.
- L'IA ne finit pas son tour à portée de trois ennemis quand une position sûre existait.
- Tests **headless** intégralement automatisables — aucune vérification manuelle.

### LOT-24 — IHM de combat {#lot-24}

> Statut : **à faire**.
> Prérequis : [LOT-21](@ref lot-21), [LOT-23](@ref lot-23).

#### Objectif

Rendre le combat **lisible et jouable** : bandeau d'initiative, portées surlignées, curseur de
ciblage, prévisualisations, journal.

C'est le lot qui transforme une machine à états correcte en un combat qu'on a envie de jouer.

#### Périmètre

- `Source/HMI/Game/CombatHud.{h,cpp}` : bandeau d'ordre d'initiative, PV, conditions actives.
- **Surbrillance de grille** via `SpriteBatch` / `ComposedScene` — le calque `EditorOverlay` hérité
  se réemploie tel quel pour les cases atteignables et les portées.
- **Curseur de ciblage** clavier et manette (pas seulement souris : cf. critère ci-dessous).
- **Prévisualisations** avant validation : chemin emprunté, cases atteintes par un gabarit,
  probabilité de toucher.
- **Journal de combat** défilant, alimenté par les entrées auditables du `LOT-21`.
- Animations d'attaque via `AnimationCatalog`.

#### Le critère qui est souvent oublié

Un combat tactique doit se jouer **entièrement au clavier et entièrement à la manette**, pas
seulement à la souris. Le projet hérite d'un système de remappage complet (clavier, manette XInput)
et d'une navigation à la manette dans tous les écrans : il serait incohérent que le combat, cœur du
jeu, soit le seul écran à exiger une souris.

#### Exigences couvertes

`EX-IHM-*`, `EX-REN-*`.

#### Critères d'acceptation

- Un combat complet se joue **entièrement au clavier** et **entièrement à la manette**.
- Chaque jet affiché est traçable au journal.
- `check_design_tokens.py` vert (la palette d'identité reste cohérente entre maquettes et code).
- Traduction fr/en complète.
- Vérification IHM manuelle par l'utilisateur, comme pour tout lot de rendu.

### LOT-25 — Sorts et capacités de classe {#lot-25}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13), [LOT-21](@ref lot-21), [LOT-22](@ref lot-22).

#### Objectif

Ajouter les sorts — emplacements, incantation, concentration — et les capacités propres aux
classes.

#### Périmètre

- `Source/Core/Rpg/Spell.{h,cpp}` : école, niveau, portée, durée, composantes, effet.
- `SpellSlots` : emplacements par niveau, consommés à l'incantation, restaurés au repos.
- `Concentration` : un seul sort concentré à la fois ; il tombe si le lanceur subit des dégâts
  (jet de sauvegarde) ou en incante un autre.
- Catalogue de sorts en **JSON** (`EX-VIS-007`).
- Branchement sur les gabarits d'effet de zone du `LOT-22` — aucune géométrie nouvelle.
- Effets **hors combat** aussi : un sort d'utilité en exploration passe par le même catalogue.

#### Note de conception

C'est le lot où la tentation d'écrire des règles en dur est la plus forte, parce que chaque sort a
sa particularité. Y céder rend l'équilibrage impossible : un sort qui se règle en recompilant ne se
règle pas. Un sort est une **donnée** ; le C++ ne porte que les *mécanismes* qu'elle compose
(dégâts de zone, jet de sauvegarde, condition appliquée, durée).

#### Exigences couvertes

`EX-DND-*`.

#### Critères d'acceptation

- Un emplacement consommé est indisponible jusqu'au repos.
- La concentration tombe au bon moment (dégâts avec échec de sauvegarde, second sort concentré),
  et **pas** aux mauvais.
- Un sort de zone touche exactement les cases du gabarit du `LOT-22`.
- Aucune règle de sort codée en dur dans le C++.

### LOT-26 — Butin, marchands, économie {#lot-26}

> Statut : **à faire**.
> Prérequis : [LOT-14](@ref lot-14), [LOT-15](@ref lot-15).

#### Objectif

Boucler la boucle économique : gagner du butin, le vendre, acheter mieux.

#### Périmètre

- `Source/Core/Rpg/LootTable.{h,cpp}` : tables de butin en **JSON**, tirées avec le hasard
  déterministe du `LOT-12` — un coffre rouvert après rechargement de sauvegarde doit donner le
  **même** contenu, sinon le joueur peut relancer jusqu'au bon tirage.
- Or et valeur des objets.
- `Shop.{h,cpp}` : achat, vente, marge du marchand, stock.
- IHM marchand, sur le système de design existant.

#### Le piège du tirage

Un butin tiré au moment de l'ouverture, avec une graine liée à l'instant, se re-tire différemment à
chaque chargement de sauvegarde. La graine doit dériver de l'**identité du coffre** et de l'état de
la partie, pas de l'horloge — `deriveSeed(baseSeed, step, entityId)` existe pour cela.

#### Exigences couvertes

`EX-RPG-*`, `EX-INV-*`.

#### Critères d'acceptation

- Un coffre donne le **même** butin après sauvegarde et rechargement.
- Vendre puis racheter un objet ne crée ni ne détruit de valeur au-delà de la marge annoncée.
- Le stock d'un marchand se comporte de façon définie quand il est épuisé.

### LOT-27 — Contenu du *vertical slice* {#lot-27}

> Statut : **à faire**.
> Prérequis : phases B, C et D complètes.

#### Objectif

Produire le contenu jouable qui démontre la boucle entière : *« un personnage explore une carte
top-down, parle à un PNJ, déclenche une rencontre, gagne un combat tactique au d20 »*.

#### Périmètre

- Un **village** : quelques bâtiments, trois PNJ, un marchand, un point de départ.
- Un **donjon** : trois salles, deux rencontres, un coffre gardé.
- Une **quête principale** reliant les deux, avec dialogues à choix et au moins un jet de compétence.
- Deux types d'**ennemis** avec des profils d'IA distincts (mêlée agressive, distance prudente).
- Un **combat scénarisé** déclenché par un dialogue.

Tout se produit **dans l'éditeur** (`LOT-11`), pas en écrivant du JSON à la main : c'est le test
grandeur nature de l'outil, et la preuve qu'un non-développeur pourrait le faire (`EX-VIS-006`).

#### Outillage

`scripts/check_world_graph.py` remplace le `check_demo_sequence.py` hérité, retiré au `LOT-01`.
Il valide, en CI :

- le **graphe de cartes** : aucun portail orphelin, aucune carte inatteignable ;
- les **références de dialogue et de quête** : chaque nœud cible, chaque drapeau, chaque
  récompense existe ;
- que chaque zone de rencontre est un **terrain tactique valide** (contrainte du `LOT-11`, née de
  la décision « combat sur la carte »).

#### Exigences couvertes

`EX-RPG-*`, `EX-LVL-*`, et concrétisation de `EX-VIS-001` à `EX-VIS-005`.

#### Critères d'acceptation

- La boucle complète se joue de bout en bout.
- Un **test système** la rejoue en headless, du départ à la victoire du combat.
- `check_world_graph.py` vert.
- Le contenu se recharge après sauvegarde à n'importe quel point de la boucle.

### LOT-28 — Audio, effets et version `0.2.0` {#lot-28}

> Statut : **à faire**.
> Prérequis : [LOT-27](@ref lot-27).

#### Objectif

Donner au jeu son identité sonore et ses retours d'impact, puis clore le programme du *vertical
slice* par une version publiable.

#### Périmètre

##### Audio

`hmi::AudioEngine` et `SoundCatalog` sont hérités et fonctionnels — ce lot les **alimente**, il ne
les réécrit pas :

- musique de carte et thème de combat, avec bascule à l'entrée et à la sortie de rencontre ;
- bruitages : pas, interaction, ouverture de coffre, jet de dé, coup porté, coup critique, échec,
  fin de combat, navigation de menu.

La règle héritée tient : **le jeu reste pleinement jouable en silence**, sans périphérique audio,
et le volume est réglable et persisté.

##### Effets

`ParticleRenderer` et la secousse d'écran sont hérités : particules à l'impact, secousse **sur
critique uniquement** (une secousse à chaque coup rendrait un combat tour par tour épuisant).

##### Équilibrage

Passe de réglage sur les données du slice : PV, CA, dégâts, seuils d'expérience, prix. Tout est en
JSON depuis la phase C — aucune recompilation.

##### Clôture

- Régénération du **cahier de test** (`scripts/generate_cahier_test.py`).
- Mise en cohérence documentaire globale : specs, guides, manuel.
- Bump `project(VERSION 0.2.0)` dans le `CMakeLists.txt` racine — **seul endroit** où la version
  s'écrit — puis tag `v0.2.0`.

#### Note de méthode

La documentation de chaque domaine est mise à jour **par le lot qui le livre**, pas ici. Ce lot ne
fait que la cohérence d'ensemble et la régénération. Un lot fourre-tout de fin de programme n'est
jamais fini : c'est le risque à éviter.

#### Exigences couvertes

`EX-REN-*` audio et effets, `EX-NFR-*` (budget de rendu mesuré, patron hérité).

#### Critères d'acceptation

- Le jeu est pleinement jouable **sans périphérique audio**.
- Volume réglable et persisté ; bascule musicale exploration ↔ combat sans coupure brutale.
- Secousse d'écran réservée aux critiques.
- Cahier de test régénéré, tous les linters verts, version bumpée et taguée.

### LOT-29 — Groupe de quatre personnages {#lot-29}

> Statut : **à faire**.
> Prérequis : [LOT-27](@ref lot-27). Vient **après** le *vertical slice*, délibérément.

#### Objectif

Passer d'un héros seul à un groupe de quatre — recrutement, compagnons suiveurs en exploration,
combat tactique à quatre alliés.

#### Pourquoi ce lot est un lot d'**ajout**, pas une refonte

C'est la décision de cadrage n° 4, et tout le programme la prépare : **rien ne doit supposer
l'unicité du personnage**. Concrètement, au moment d'aborder ce lot, ces précautions doivent déjà
être en place :

- `CharacterSheet` est un objet **autonome** (`LOT-13`), jamais un singleton joueur ;
- `TurnOrder` est **multi-alliés** dès le `LOT-20`, avec un test qui monte quatre alliés ;
- `SaveGame` stocke une **liste** de personnages dès le `LOT-17`, pas un champ unique ;
- les écrans de fiche et d'inventaire sont conçus pour un **sélecteur de personnage**, même quand
  ils n'en affichent qu'un.

Si l'un de ces points a dérivé en chemin, ce lot redevient une refonte — c'est le signal
d'alarme à surveiller pendant les phases C et D.

#### Périmètre

- **Recrutement** de compagnons (dialogue, quête).
- **Personnages suiveurs** en exploration : ordre de marche, suivi du héros, pathing simple.
- **Sélecteur de personnage** activé dans les écrans de fiche, d'inventaire et d'équipement.
- Quatre alliés dans l'ordre d'initiative ; **ciblage allié** (soins, sorts de soutien).
- Répartition de l'expérience et du butin.

#### Le point délicat

Le combat à quatre alliés multiplie les tours et allonge la boucle : l'IHM du `LOT-24` doit rester
lisible avec huit combattants au bandeau d'initiative. C'est le seul endroit où ce lot peut exiger
un vrai travail d'interface plutôt qu'un simple ajout.

#### Exigences couvertes

`EX-RPG-*`, `EX-CBT-*`, `EX-IHM-*`.

#### Critères d'acceptation

- Un combat à **4 alliés contre 4 ennemis** se déroule en headless jusqu'à une condition de fin.
- **Aucune régression** du jeu à un personnage : le contenu du `LOT-27` reste jouable tel quel.
- Les suiveurs ne restent pas coincés dans le décor ni ne bloquent le héros dans un passage étroit.
- Le bandeau d'initiative reste lisible à huit combattants.
