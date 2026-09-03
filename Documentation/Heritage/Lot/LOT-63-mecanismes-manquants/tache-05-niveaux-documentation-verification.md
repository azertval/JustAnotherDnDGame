# TACHE-05 — Niveaux de démonstration, documentation et vérification {#lot-63-tache-05-niveaux-documentation-verification}

**Lot :** [LOT-63](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Documentation` ·
**Statut :** fait (CI complète vérifiée localement ; essai manuel réel réservé à l'utilisateur,
cf. `feedback_no_live_gui_automation.md`)

## Contexte
Le `LOT-25` a posé une règle que le projet tient depuis : chaque mécanisme livré a **son** tableau
de démonstration, la séquence les enchaîne dans un ordre pédagogique, et un garde-fou
(`scripts/check_demo_sequence.py`) empêche qu'un niveau jouable échappe au test système.

Trois mécanismes arrivent : trois tableaux, et la mise à jour des documents que ce lot rend faux —
dont le tableau des contrôles, qui annonce « Interagir » comme non implémentée depuis le début.

## Travail à réaliser
- **Trois niveaux de démonstration**, un par mécanisme livré : `demo-cle.json`,
  `demo-plateforme.json`, et l'intégration de l'action « Interagir » dans l'un d'eux. Chacun isole
  son mécanisme, comme le veut la conception du `LOT-25`.
- **Insertion dans la séquence** au bon endroit pédagogique — après les mécanismes dont ils
  dépendent, avant `demo-final`. Depuis [LOT-59](@ref lot-59), la séquence est un **fichier de
  contenu** : la modifier ne demande plus de recompiler.
- **Test système étendu** : les nouveaux tableaux sont franchis de bout en bout (`EX-NFR-021`), et
  `scripts/check_demo_sequence.py` reste vert.
- **Spécifications** :
  - `gameplay.md` — lever le ⚠️ de `EX-GP-023`, déclarer `EX-GP-026` en section 3 ;
  - `controles.md` — la ligne « Interagir » du tableau perd sa mention ⚠️, et `EX-CTRL-022` est
    déclarée ;
  - `vision.md` — les plateformes mobiles ne sont plus « (plus tard) ».
- **Manuel utilisateur** (`Documentation/Manuel/jouer.md`) : la touche d'interaction dans le tableau
  des contrôles, et les trois mécanismes dans la section « Objectif d'un niveau », qui décrit déjà
  interrupteur, plaque et bloc.
- **Guide du développeur** : les trois mécanismes dans `guide-niveaux.md` et, pour la plateforme,
  l'**ordre de résolution** dans le pas — c'est l'information que la prochaine personne cherchera.
- **Cahier de test** régénéré ; chaque nouveau `TEST()` porte son bloc `\castest{}` écrit en même
  temps que lui.

## Fichiers impactés
- `Source/Elements/Levels/demo-cle.json`, `demo-plateforme.json` (nouveaux), fichier de séquence,
  `README.md`.
- `Source/Test/Systeme/test_parcours_complet.cpp`.
- `Documentation/Specification/{gameplay,controles,vision}.md`.
- `Documentation/Manuel/jouer.md`, `Documentation/Guide/guide-niveaux.md`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- Chaque nouveau tableau est **franchissable** de bout en bout par le test système.
- Chaque nouveau tableau **charge et valide** sans erreur.
- `python scripts/check_demo_sequence.py` — séquence jouée et séquence testée identiques.
- `python scripts/lint_exigences.py` — `EX-CTRL-022` et `EX-GP-026` déclarées une fois et
  référencées.
- `python scripts/generate_cahier_test.py --check`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %, **et** le job Release de [LOT-58](@ref lot-58).

## Points d'attention
- **Si une tâche a été rognée, ne pas documenter ce qui n'existe pas.** La plateforme mobile est la
  candidate au retrait ; dans ce cas `EX-GP-026` n'est pas déclarée, le niveau n'est pas ajouté, et
  `vision.md` reste inchangée. Un référentiel qui promet plus que le binaire est pire qu'un
  référentiel en retard.
- Un niveau de démonstration doit **isoler** son mécanisme : mélanger clé et plateforme dans le même
  tableau rendrait un échec de test système inexploitable.
- La séquence s'allonge : vérifier que le test système reste dans une durée raisonnable.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- **Essai manuel** au moment prévu par le lot : jouer les trois tableaux, y compris les cas
  désagréables (mourir la clé en main, sauter d'une plateforme montante, rester sous une plateforme
  qui monte).

## Définition de fait (DoD)
- Trois tableaux de démonstration sont livrés, insérés dans la séquence et franchis par le test
  système ; les mentions ⚠️ levées le sont réellement ; les deux nouvelles exigences sont déclarées ;
  manuel et guide décrivent le livré ; le cahier est régénéré ; la CI complète est verte ; l'essai
  manuel est fait.

## Exigences
`EX-CTRL-022`, `EX-GP-026` (déclarées ici) ; lève `EX-GP-023` ; réutilise `EX-NFR-021` (test
système de franchissabilité), `EX-LVL-012` (niveaux de démonstration), `EX-LVL-013` (séquence en
donnée), `EX-NFR-012` (conventions), `EX-NFR-022` (CI verte).
