# TACHE-04 — Documentation et vérification {#lot-64-tache-04-documentation-verification}

**Lot :** [LOT-64](epic.md) · **Emplacement :** `Source/Test`, `Documentation` ·
**Statut :** fait (vérification automatisée complète ; essai manuel restant à faire par le level
designer avant fusion, voir DoD)

## Contexte
Ce lot modifie une exigence **livrée**, ce qui est rare et demande de la précision. `EX-REN-015`
affirme aujourd'hui que la caméra ne suit « **jamais** le personnage en continu ». C'était vrai, et
c'était une décision de conception assumée du `LOT-32`. Ce lot ne l'annule pas : il en fait une
propriété du **mode par salle**, à côté de deux autres modes. La formulation doit refléter cette
nuance sans laisser croire que la décision du `LOT-32` était mauvaise.

## Travail à réaliser
- **Spécifications** :
  - `rendu-technique.md` — déclarer `EX-REN-016` ; **reformuler** `EX-REN-015` pour qu'elle décrive
    le mode *par salle* plutôt que le comportement unique du moteur, en conservant sa justification
    d'origine ;
  - `niveaux.md` — déclarer `EX-LVL-006` (le niveau porte son cadrage) ;
  - `editeur-niveaux.md` — déclarer `EX-EDIT-028`.
- **Guide du développeur** (`Documentation/Guide/guide-rendu.md`) : les trois modes, la règle de
  repli et pourquoi elle existe, la caméra de suivi (zone morte, anticipation, bornage) et ses deux
  pièges — cadence sur le pas fixe, alignement pixel. C'est l'information que la prochaine personne
  cherchera.
- **Guide des niveaux** (`guide-niveaux.md`) : le champ de cadrage dans le format, la version
  incrémentée.
- **Manuel** (`Documentation/Manuel/partager-un-niveau.md`) : le choix de cadrage dans l'éditeur,
  écrit pour un non-codeur — quel mode pour quel genre de tableau.
- **Cahier de test** régénéré ; chaque nouveau `TEST()` porte son bloc `\castest{}` écrit en même
  temps que lui.
- **Vérification manuelle**, au moment prévu : un tableau dans chacun des trois modes, en observant
  particulièrement la netteté du pixel art en mode suivi et le comportement aux bords du niveau.
  Étendue par [TACHE-05](tache-05-zones-camera-taille-suivi.md) : dessiner des zones de tailles
  différentes dans un même niveau et vérifier la bascule entre elles au canevas comme en jeu.

## Fichiers impactés
- `Documentation/Specification/{rendu-technique,niveaux,editeur-niveaux}.md`.
- `Documentation/Guide/guide-rendu.md`, `guide-niveaux.md`.
- `Documentation/Manuel/partager-un-niveau.md`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — `EX-LVL-006`, `EX-REN-016` et `EX-EDIT-028` déclarées une
  fois et référencées.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %, **et** les jobs Release et ASan de [LOT-58](@ref lot-58).
- **Non-régression du contenu livré** : les quinze tableaux existants se jouent exactement comme
  avant — c'est le critère d'acceptation numéro un du lot, et il se vérifie ici une dernière fois.

## Points d'attention
- **Reformuler `EX-REN-015` sans la renier.** Sa justification — ne pas rapetisser le rendu, ne pas
  suivre en continu sur un niveau à salles — reste valable pour son mode. Une exigence livrée qu'on
  réécrit doit garder trace de son intention d'origine.
- Ne documenter que le livré : si un mode a été rogné, il ne figure ni dans le guide ni dans le
  manuel.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Le manuel s'adresse à un non-codeur : « ce tableau se voit d'un coup d'œil » plutôt que « mode de
  cadrage statique ».
- **Trouvé pendant l'essai manuel** : la parallaxe des décors (`EX-DEC-006`) n'avait jamais eu
  d'effet visible avant ce lot (aucune caméra ne défilait en continu) ; le mode *suivi* la rend
  visible pour la première fois, et un décor Fond/Premier plan semble alors « suivre » la caméra.
  Neutralisée spécifiquement pour ce mode (`hmi::composeWorldSprites`, `applyDecorParallax`) —
  signe qu'une caractéristique dormante d'un lot antérieur peut redevenir active à l'improviste
  quand une nouvelle caméra change une hypothèse implicite (« la caméra ne défile jamais »).

## Définition de fait (DoD)
- Les trois nouvelles exigences sont déclarées, `EX-REN-015` est reformulée en conservant sa
  justification, guides et manuel décrivent les trois modes et leurs pièges, le cahier est
  régénéré, la CI complète est verte, la non-régression des quinze tableaux est vérifiée et l'essai
  manuel est fait.

## Exigences
`EX-LVL-006`, `EX-REN-016`, `EX-EDIT-028` (déclarées ici) ; réutilise `EX-REN-015` (reformulée),
`EX-NFR-012` (conventions), `EX-NFR-020` (tests), `EX-NFR-021` (test système), `EX-NFR-022`
(CI verte).
