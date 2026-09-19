# Références communes et préparation régionale

Cette préparation croise le Sourcebook, la table régionale CC166, des fiches individuelles ciblées et les données du jeu. Elle ne certifie pas la lecture exhaustive des livres ni l’inspection de chaque image. Les dossiers régionaux séparent ce que dit le texte, ce qui a été observé et les lots proposés. [Index](epic.md).

## Corpus et localisateurs

| Source | Localisateur local de travail | Convention |
|---|---|---|
| SB | `Documentation/SourceBook/Tanares_Sourcebook.pdf` | Double page ; PDF physique 1-based ET page imprimée |
| CC | `Documentation/SourceBook/VTT/Character Compendium - High.pdf` | PDF166 table régionale ; PDF168 Elder Dragon ; PDF169 publicité |
| PG | `Documentation/SourceBook/Players_Guide_to_Tanares_Version_20231218.pdf` | Double page ; espèces et variantes à vérifier au besoin |
| Index PNJ historique | [160 fiches](../LOT-91-atelier-pnj/atelier/pnj/index.json) | Slugs existants ; pas exhaustif CC168 ; collectif/compagnon peut demander plusieurs unités |
| Données jeu | `Source/Elements/World/regions/`, `locations/` | 13 régions ; certaines locations sont objets/PNJ/règles extraits |
| Progression | [Roadmap existante](../roadmap.md) LOT16/27/96 | Lecture seulement ; ne pas la modifier dans ce chantier |

Les liens PDF locaux sont des localisateurs du corpus fourni, non des fichiers à ajouter à Git. Aucune copie de PDF nécessaire à la consultation de ces notes. SB PDF46–103 couvre les treize régions ; PDF104–106 les marges ; PDF26–34 la Pénombre ; PDF107–118 les organisations. Un bandeau paysager n’est jamais une carte.

**Cartes effectivement inspectées en complément** : [CC PDF167](<../../SourceBook/VTT/Character Compendium - High.pdf#page=167>) montre régions dont Mystical, côtes, rivières, montagnes, routes impériales/secondaires, frontières, marqueurs Village/Town/City/Capital/Castle-Fort/Site, rose des vents et échelle. Elle confirme notamment Kepesh au sud de Republic et à l’ouest/sud-ouest de Taii, Yama à l’est/nord-est de Mystical. Aucun tracé/distances au pixel ne doit être recopié ; décider l’échelle du jeu et documenter les écarts.

[SB PDF49/imprimées96–97](../../SourceBook/Tanares_Sourcebook.pdf#page=49) : carte Capital avec Martpart, Arenarea/Arena of Fate, Uptown et autres quartiers, Tourmaline Bay à l’ouest/nord-ouest, cours d’eau/ponts, docks, enceintes et rues. Référence pour transitions et hiérarchie des monuments, pas modèle de chaque maison. [SB PDF57/imprimées112–113](../../SourceBook/Tanares_Sourcebook.pdf#page=57) : Fisherman’s Wharf, légende12 sites ; port sud-ouest6, forteresse nord-ouest3, Parlement central1/Six Quarters8, arène est2, palais sud-est4, cathédrale sud5, Seabreeze7, Wallside9, Uptown nord-est10, Hope Square11, Cerulean Plaza nord12, bras d’eau/ponts au nord. Un parcours port→marché/guilde→Parlement est une proposition jouable, non un ordre canonique. Aucune autre carte régionale détaillée inspectée ; JPG de cartes séparés non consultés. Rendus de ces trois pages conservés hors dépôt.

## Références visuelles hors Git et mémoire versionnée

L’utilisateur autorise **l’envoi d’images du corpus au générateur intégré dans ce chat**. Conserver PDF, renders, crops et autres images d’entrée issues du corpus dans le répertoire de travail extérieur au dépôt ou dans un `.cache` dont l’exclusion est vérifiée. Ne jamais les copier dans Assets, Documentation, commit ou PR ; ne pas produire de calque. Seuls nouveaux originaux acceptés et métadonnées de provenance entrent dans Git. Le portrait final généré est une création conservée ; une illustration découpée du livre ne l’est pas.

Pour chaque référence, enregistrer `source_id`, titre/édition, chemin local ou identifiant stable, SHA-256 du document, pages PDF/imprimées, côté gauche/droit, coordonnées `(x,y,w,h)` du crop et dimensions/résolution du rendu, SHA-256 du rendu/crop, observation et date, personne/outil ayant relu. Ces empreintes ne sont pas encore remplies par cette préparation : les relever sur le fichier effectivement envoyé, sans valeur inventée.

Pour chaque unité : clé canonique `<famille>/<slug>`, usages région/scène/quête/UI (plusieurs possibles), type de donnée, source/page, fait textuel, observation visuelle, proposition, inconnue/conflit, dimensions/profil, fichiers attendus, réemploi, fiche/promptB/palette/portrait, ancres A+B+C, versions et historique prompt/réponse/verdict. Déclarer statut `à faire`, `généré`, `rejeté`, `validé`, `intégré` et motif d’attente ; lier verdicts aux hashes. Un symbole commun ne reçoit qu’une clé canonique. Les usages régionaux référencent cette clé, sans duplication de fichier.

## Pouvoirs, factions et organisations

Territoire, alliance politique, organisation, ordre et religion sont cinq catégories distinctes. Les descriptions ci-dessous sont des **observations visuelles**, pas une invitation à extraire les emblèmes du livre. Les cinq grands écus ont été inspectés ; les médaillons d’organisations ont seulement été observés sur planche-contact réduite, donc recadrage/relecture requis avant brief fin.

- Allied Forces : PDF20/imprimée39, écu bleu nuit en pointe, bord métal sombre argenté et petites pointes, balance dorée centrale avec axe vertical et plateaux triangulaires. [SB20](../../SourceBook/Tanares_Sourcebook.pdf#page=20)
- Arcanum : PDF22/imprimée42, écu jaune-or à bord bordeaux/prune, gemme rouge sommitale et gemme rouge centrale sertie, motif de support doré sous la gemme. Ne pas nommer la gemme «œil» sans crop plus précis. [SB22](../../SourceBook/Tanares_Sourcebook.pdf#page=22)
- Cult of Shadow Wing : PDF23/imprimée44, écu rond noir bord métal ; forme ailée violette à bord doré au centre, oiseau noir ailes déployées perché au sommet. [SB23](../../SourceBook/Tanares_Sourcebook.pdf#page=23)
- Forces of Darkall : PDF24/imprimée46, écu rond/ovale sombre cerclé rouge à pointes métalliques, crâne pâle avec détail circulaire au front, cadre polygonal sombre. [SB24](../../SourceBook/Tanares_Sourcebook.pdf#page=24)
- Tanarean Empire : PDF25/imprimée48, écu rouge bord acier riveté, tête de dragon de profil dorée, petites marques dorées disposées en arc au-dessus. [SB25](../../SourceBook/Tanares_Sourcebook.pdf#page=25)
- Organisations, relevé visuel de contact-sheet (échelle réduite ; fournir un crop original avant reproduction fine) : PDF107 Bank of Hajal médaillon clair et ligne végétale/serpentine verte autour d'un axe doré ; PDF108 Bloodbound médaillon rouge/or sombre ; PDF109 Celestianist disque doré et motif violet ; PDF110 Collectors motif polygonal gris dans anneau ; PDF111 D.E.A.T.H. créature draconique bleue/verte sur disque clair ; PDF112 Draconic Council motif draconique multicolore ; PDF113 Eternal Circle arbre vert dans anneau coloré ; PDF114 Parliament ancre/trident stylisé bleu-violet ; PDF115 Relic Hunters entrelacs circulaires orange sur disque clair ; PDF116 Third Eye étoile géométrique violette ; PDF117 Watchers motif tentaculaire violet/orange ; PDF118 autres organisations sans symbole isolé identifié. Ces descriptions sont observations limitées, pas spécifications héraldiques définitives. [SB107–118](../../SourceBook/Tanares_Sourcebook.pdf#page=107)
- Distribution textuellement vérifiée : Bloodbound siège Bauron's Paradise Casino Goldraft, présence maritime SB74 ; Celestianist Cathedral Skybell, robes Tamera blanc/or soleil et Bauron symbole lune SB109 ; D.E.A.T.H. siège Dragonends Fortress Umay et leader Renkyr SB111 ; Eternal Circle sans siège unique, bases naturelles dispersées et Ardnold Bak chef SB113 ; Parliament Skull Island proche Sandport, cinq capitaines Bretrar/Dorro/Liana/Black Soul/Maryne SB114 ; Third Eye Hidden Eye Ahimoth et dirigeants Tuani/Aldo/Oraelus SB116 ; Watchers Penumbral Mountain Stronghold ouest de la surface penumbrale et Golgöggoth SB117. [SB107–118](../../SourceBook/Tanares_Sourcebook.pdf#page=107)
- Tatouage Third Eye explicitement en forme d'œil, peut changer de forme chez porteur vivant, revient œil à la mort. Ne pas confondre ce tatouage narratif avec médaillon étoilé illustré. [SB116](../../SourceBook/Tanares_Sourcebook.pdf#page=116)

Myr dirige les Death Merchants (SB PDF51/CC91), pas Third Eye. Le Third Eye du chapitre capitale est lié à Uptown ; le repaire sous Martpart est une **adaptation explicite du projet LOT27**, conservée pour la progression. Bloodbound sous Arenarea et siège Goldraft sont des implantations distinctes. Ironhand = armée impériale, Imperial Wizards = administration arcanique ; ni l’un ni l’autre ne reçoit arbitrairement le blason régional du lion. ASG, Eldritch Hand, Wood Hunters, The Last Fire et clans locaux restent identifiés dans leurs régions.

## Panthéon et expressions matérielles

Notices SB imprimées27–37/PDF14–19 ; tenues, temples et symboles imprimées68–86/PDF35–44. Une religion ne prescrit pas l’habit de toute une population. Les motifs observés orientent une interprétation originale adaptée au jeu, pas un bouton découpé du livre.

| Divinité / religion | Ancrage, tenue/architecture textuelles | Médaillon vu / localisation |
|---|---|---|
| Tamera / Celestianism | vie/soleil, robes blanc/or, cathédrales blanches, or/argent/marbre, vitraux, bougies-autels. Taram + Jocasta ; Sons of Tamera, Seeds of Life, Scribes, Enlightened. | Soleil doré rayonnant à centre lumineux ; SB43 gauche /84. |
| Bauron / Celestianism | lune/loi/chasse, robes pourpres visage souvent caché, cathédrales fortifiées pierre sombre. Taram + Ellen ; Awakened, Heralds, Inquisition, Spiritual Guides. | Disque violet avec grand croissant et petites phases lunaires, monture argent ; SB37 gauche /72. |
| Tamera-Bauron / Windspeaker | une entité duale, distincte du dogme des deux jumeaux Celestianist ; forteresse-monastère de sommet, cours ouvertes pour griffons ; couleurs d'ordres PG36–38. | Figure centrale ailée, une aile blanche plumeuse et l'autre sombre violette ; SB42 droite /83. |
| Aibidh / Cuibhle | automne/air/magie ; soie bleu éthéré/blanc, pierre lumineuse/verre cristallin, carillons ; Aurindunnum et Mystical Knights sur stormbirds. | Quatre volutes claires autour d'un centre sombre ; SB35 droite /69. |
| Breith / Cuibhle | printemps/terre/forêt ; vert à capuche, corde, symboles bois, clairières/cavernes sacrées. | Cristal vert au centre de feuilles/terre brunes ; SB37 droite /73. |
| Òigridh / Cuibhle | été/feu/voyage ; soie émeraude/or, bracelets/pièces ; temples ouverts à feu éternel. | Motif igné rouge/or à quatre lobes ; SB42 gauche /82. |
| Bás / Cuibhle | hiver/eau/mort ; capes noires, crânes, armes givrées ; ziggourats d'obsidienne, trophées osseux. Temples propagent hiver ; alliance secrète Golgöggoth ne rend pas toute la foi identique au Cult. | Motif bleu aqueux/glacé à goutte centrale, encadrement noir/violet ; SB36 droite /71. |
| Glorious One / Path of Glory | discipline/excellence/voyage ; armure sous robe de voyage, bâton/sac/arme entretenue ; pierre/marbre blanc en hauteur, scènes héroïques. | Casque rouge/or ; SB40 droite /79. |
| Lumina / Luminism | connaissance/art ; habits élégants colorés, cravates/écharpes, symbole discret bague/collier ; bibliothèques/écoles/académies. | Rosace composée d'éléments semblables à livres/pages et plumes ; SB41 droite /81. |
| Nature Spirits / Spiritualism | pas de divinité ni représentation unique ; traditions locales, nature, runes/peaux animales Kolbjörn, sites naturels ; pas de hiérarchie mondiale. | Visage-arbre brun à feuillage orange dans médaillon argent ; illustration SB43 droite /85, à ne pas imposer comme blason universel. |
| Ba-Ka / foi Ba-Ka | âme/magie, Kepesh ; satin or/bleu/noir, pyramides grès à sommets dorés, Akhu ; connexion Arcanum avec Cirrus. | Oiseau à tête crânienne et disque solaire or ; SB36 gauche /70. |
| Fumetsu Tenshikin / Fumetsu Eido | dragon tameranium souverain de Yama sous guise divine ; temples bois, cordes/sigils papier, kimono blanc/jaune/rouge. | Signe noir sur disque rouge dans cadre or ; SB40 gauche /78. |
| Krynnethoth / Cult Shadow Wing | maladie/malédictions/failles, sanctuaires cachés, violet sombre ; infiltration de l'ancienne Église par Gideoni. | Livre ouvert surmonté de tentacules violets ; SB41 gauche /80. |
| Z’ulvath / Cult Shadow Wing | père des autres Ungods, corruption/dragons penumbraux ; pierre sombre, flammes violettes ; pas simple dragon national. | Entité tentaculaire violette ailée ; SB44 gauche /86. |
| C’thraxis et Droggath / Cult Shadow Wing | séduction/manipulation pour première, destruction/Spawners/Abhorroth pour second ; distinguer leurs sectes. | Sources symboles SB38 gauche /74 et SB39 gauche /76 ; pas inspectés visuellement dans cette passe. |
[Religion et ordres, SB35–44](../../SourceBook/Tanares_Sourcebook.pdf), [couleurs Cirrus, PG18–20](../../SourceBook/Players_Guide_to_Tanares_Version_20231218.pdf).

Dorsi / Dorsism : SB imprimée75/PDF38, vertus naines ; robes épaisses gris-brun, emblèmes de clan/Dorsi, amulette de pierre, sanctuaires de pierre gravés de vertus/batailles ; chimère attestée au texte, image non inspectée. Fruitful : imprimée77/PDF39, abondance/hospitalité ; blanc-vert, fleurs/fruits, symboles personnels bois, vergers sacrés et statues bois/pierre ; image non inspectée. Droggath PDF39 : les fidèles ne fabriquent pas d’image de leur dieu, le sang sert de symbole ; le médaillon éditorial ne doit pas devenir un accessoire rituel universel.

## Réemploi, histoire et contrôle de couverture

Le registre commence par la reprise des cinq PNJ pilotes, la [reprise exhaustive Martpart/Arenarea/Colisée](tache-ce-r01-reprise-trois-sites.md), la [refonte obligatoire Ironhand](tache-ce-r02-refonte-ironhand.md), puis l’audit lion/loup. Toute texture doit réussir la [carte de test en jeu avec témoin de taille](tache-01-carte-test-textures.md) et ses raccords. La présence au dépôt n’équivaut pas à validation visuelle. Objets récurrents SB imprimées252–263/PDF127–132 : monnaies, lettres Bank of Hajal, contrats vivants, portails, Stone Disks, horloges, sabliers et chandelles ; chacun entre dans un lot quand commerce/quête/transport le consomme. Yama utilise notamment Electron : ne pas imposer l’icône or à toutes les monnaies.

| Couverture | Acquis | À poursuivre au besoin |
|---|---|---|
| 13 régions | Lecture ciblée paysages, lieux, cultures, menaces ; 13 bandeaux et bannières observés | Cartes détaillées, tous costumes et chaque fiche individuelle |
| Centre/nord | SB46–69, Martpart/Arenarea relus, CC48/91 illustrés | Aeyefall ; histoire des ruines et cartes |
| Côtes/forêts/nains | SB70–87 ciblé ; Grace/Liana/Niary/Rurik fiches lues | Passages tronqués SB73/76/84/85, Zhaeral CC159 à relire, portraits à inspecter |
| Sud | SB88–103, PG26 Taii observé ; fiches ciblées | Morphologies et formes spécifiques des PNJ, cartes |
| Marges | SB104–106 textuel | Visuels Darkall/Undertanares/Wasteland |
| Pénombre | Textes couches1–6 ; Surface/Endless City/Clockwork Nexus observés | Visuels couches1–3/6, carte et fiches par rencontre |
| Histoire | Repérage SB236–251 imprimé/PDF119–126 | Lecture approfondie avant ruines Kölm/Zepardit/Orsal/Arcana |
| Bestiaire | Menaces régionales et quelques fiches | SB chapitre9 PDF133–175 à lire pour chaque créature retenue |

Journal des divergences à préserver : six/sept ordres Cirrus (SB61 vs SB42/63/PG18–20) ; Trygve bénédiction vs lycanthropie CC132 ; Avelum SB58 LG vs CC166 LN ; Ellen/Ella ; U’Tibam SB CG vs CC137 LN ; Ahr’Arthra SB CE vs CC7 LE ; Ohris SB205 NG vs CC166 LN ; Hoccugius conjuration SB139 vs Transmuter CC62/166 ; Ukhumlin/Ukhumlim ; Arandaros Vaeraunt/Vaeraunt ; Tyreen rumeurs vs humaine CC136 ; Thyra « dead? » incertain. Ne pas corriger gameplay depuis une interprétation graphique.

CC166 fournit une association régionale, **ni une affiliation politique automatique, ni une présence en scène, ni une liste de portraits à produire**. Des pages de lieux mélangent personnages, objets, règles et artefacts d’extraction. Les voisins JSON ne suffisent pas à déduire une route de progression, notamment Kepesh/Yama. Le plan ne revendique donc ni couverture exhaustive des livres ni toutes références visuelles vérifiées.
