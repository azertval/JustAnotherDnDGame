# Fabrique d’assets — pilote PNJ

Génération et relecture dans le chat local ; commandes Python pour mémoire, préparation,
réception, contrôle et rapport. Aucun SDK, clé API ou plafond en dollars. Ce dossier contient
les originaux générés du projet et leur historique. Les images de référence sont locales,
ignorées par Git, y compris celles issues du projet ; chemins et empreintes seuls sont versionnés.

## Exécution

Depuis la racine, avec Python 3.13 et Pillow/numpy ; PyMuPDF sert à la lecture du corpus :

```powershell
py -3.13 scripts/asset_factory.py init
py -3.13 scripts/asset_factory.py request anariel portrait
# Le chat transmet prompt.txt et les références figées à imagegen.
py -3.13 scripts/asset_factory.py receive anariel portrait-001 <sortie-imagegen.png>
py -3.13 scripts/asset_factory.py request anariel sheet
py -3.13 scripts/asset_factory.py receive anariel sheet-001 <sortie-imagegen.png>
py -3.13 scripts/asset_factory.py qc anariel
py -3.13 scripts/asset_factory.py report
py -3.13 scripts/asset_factory.py status
```

Une passe `request <slug> <idle|walk|hit|death|attack|cast> --correction "…"` refait
seulement l’animation fautive. Chaque tour reçoit un numéro neuf ; une réception existante
ne s’écrase pas. `reprocess <slug> <tour>` rejoue seulement l’extraction après correction de
l’outil, garde ses verdicts précédents dans `processing-*.json` et ne compte pas comme un appel
au générateur. Une requête sans réponse reste explicitement en attente : ne pas la facturer
ni la rejouer silencieusement. Le journal ne connaît pas les quotas consommés si le service
ne les expose pas.
Les réceptions sont séquentielles : ce prototype ne fournit pas encore de verrou entre
plusieurs processus. Les empreintes de texte normalisent CRLF en LF pour survivre au
checkout Git ; les empreintes binaires restent exactes. Les anciens hashes bruts des
premiers tours sont conservés dans `original_byte_hashes`.

## Mémoire et contrôles

- `assets/Npc/<slug>/` : fiche sourcée, bloc B, palette héritée, empreintes du résultat manuel,
  tours, candidats, provenance, mesures et preuves ancien/nouveau. Les copies figées
  `reference-*.png` et `pose-reference.png` restent locales et doivent être conservées pour
  retraiter leurs tours ; après un nouveau clone, les rétablir depuis leurs sources avec
  vérification des empreintes avant réception, sans les ajouter à Git.
- `index.json` : état global reconstruisible à partir des journaux ; aucun doublon régional.
- `.cache/asset-factory/baseline/` : copie locale des assets livrés au début du PoC, retrouvables
  au commit enregistré dans chaque `baseline.json`.
- `profile.json` : dimensions/seuils du pilote. La calibration provisoire **interdit la
  promotion** ; elle doit être éprouvée et revue, pas simplement renommée pour passer au vert.
- `rapport-poc.md` : raisons des rejets, tours, comparaisons. Les GIF sont des preuves de
  lecture ; ils ne sont pas consommés par le moteur.

Le découpage n’ajoute ni ne duplique de frame et ne rogne aucun dessin pour le faire tenir.
Les particules proches peuvent être regroupées ; toute ambiguïté de compte reste un rejet.
La hauteur de référence et les contacts au sol servent à une échelle commune par tour.
Cette estimation géométrique **ne sait pas isoler une tête d’une arme ou un pied d’un effet** :
les cas concernés exigent un meilleur ancrage corporel avant validation, sans réduire
artificiellement le personnage. La normalisation alpha/palette reste distincte de la revue
du fond, du halo, de la silhouette, de l’arme et de la locomotion.

Après ouverture réelle des portraits et preuves, `review <slug> rejected|review|validated
--notes "…"` enregistre le verdict avec empreintes. Un résultat numérique bloquant empêche
`validated`. Changer le brief, la palette, le profil ou le processeur invalide le verdict.
`integrate <slug>` exige les deux contrôles à jour et une branche distincte de main ; conserve
les anciens fichiers dans une transaction locale et la table `replaces`. Une installation
interrompue reste à examiner avant reprise. L’installation seule ne vaut pas validation en jeu.

## Acceptation finale du PoC

Les cinq PNJ doivent être conformes, leurs trente bandes et descripteurs chargés par le jeu,
visibles dans la galerie, comparés aux manuels, puis livrés avec crédits, rapport et CI verte.
Les tests d’intégration du jeu exécutés avant promotion ne prouvent que l’état déjà livré.
Ne pas marquer T00 terminé tant que ces preuves manquent.

Tests locaux du traitement : `py -3.13 -m pytest scripts/tests/test_asset_factory.py`.
Build et tests du jeu : `powershell -File scripts/build.ps1 -Label integration`.
