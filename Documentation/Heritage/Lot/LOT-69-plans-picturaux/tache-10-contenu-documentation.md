# TACHE-10 — Contenu, documentation et référentiel {#lot-69-tache-10-contenu-documentation}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Documentation` ·
**Statut :** livré

## Contexte
> **Correction en cours de tâche.** Le cadrage annonçait `demo-final.json` comme **seul** niveau
> porteur de décors. C'est faux : **les vingt-deux** niveaux livrés en portent (soixante-deux
> entrées au total, refondues au `LOT-65`), `demo-final` étant simplement le plus fourni avec sept.
> Le report vaut donc pour tous, et la migration a été étendue en conséquence — laisser vingt et un
> tableaux se dépouiller silencieusement de leur habillage aurait été la régression que ce lot
> existe pour éviter.

Il faut préserver leur intention visuelle sans prétendre à une conversion mécanique : un assemblage
de sprites ne devient pas une fresque peinte par transformation automatique.

Le volet référentiel est contraint par `scripts/lint_exigences.py` : chaque exigence déclarée
**exactement une fois**, aucune référence orpheline, et toute exigence déclarée référencée quelque
part.

## Travail à réaliser
- **Migrer les vingt-deux niveaux** : retirer `decors`, déclarer un **fond** à densité 8 et — pour
  les seuls niveaux qui portaient des décors de premier plan — un **plan de devant** à densité
  native. Livrer partout un plan de devant presque vide coûterait une texture pleine taille et une
  passe de rendu par image (`TACHE-09`) pour ne rien montrer.
- **Facteurs de parallaxe sur `demo-final` et `demo-mouvement` seulement** — les deux seuls niveaux
  dont la caméra **défile** (*par salle* et *suivi*). Ailleurs, le cadrage *niveau entier* neutralise
  la parallaxe (`hmi::planeParallaxActive`) : déclarer un facteur y serait une promesse que rien ne
  tient.
- **Générer les PNG** par un script (`scripts/generate_demo_plans.py`) qui peint un fond dérivé du
  **thème** de chaque niveau, puis y **reporte ses décors à leurs anciennes positions** — même
  géométrie que l'ancien rendu (coin haut-gauche sur la position, taille `pixels / 16` unités). Le
  résultat est **reproductible** (`LOT-66`), ce qu'une image dessinée à la main et commitée ne serait
  pas. Puis supprimer `Source/Elements/Assets/Decors/` et le générateur d'assets de décors.
- **Conserver les motifs Kenney comme entrées du générateur** (`scripts/motifs/`) : quatre des
  motifs reportés sont des images, pas du code. Les supprimer avec le reste du dossier rendrait le
  script injouable — un générateur privé de ses sources n'est plus reproductible, ce qui lui ôte sa
  seule raison d'exister. Les motifs procéduraux, eux, vivent désormais dans le script.
- Vérifier `check_demo_sequence.py` et la séquence, et contrôler la **présence et les dimensions**
  des PNG référencés.
- **Référentiel** — la stratégie est déjà en place depuis le cadrage et doit être **maintenue** :
  aucune ancre supprimée, aucune renumérotation ; les exigences retirées vivent dans la section
  dédiée de [`decors.md`](@ref spec-decors), leur texte d'origine intact, chacune précédée du motif
  du retrait. Les dossiers `LOT-49`/`LOT-50`/`LOT-51` et le `CHANGELOG` continuent de s'y référer
  sans être réécrits — réécrire un lot livré falsifierait son histoire.
- Mettre à jour les **guides** utilisateur (niveaux, éditeur, rendu, atelier pixel art) : la notion
  de décor y disparaît au profit des plans, et le mode création y est décrit — y compris ce qu'il
  **n'est pas** (la référence est un repère géométrique, pas un aperçu).
- Figer les **statuts** : chapeau de `decors.md`, tableau de découpage de l'epic, `lots.md`.
- `CHANGELOG.md` sous `[Non publié]` : compléter la section du lot, en mentionnant explicitement le
  **retrait** du système de décors et l'**inversion** de la décision du `LOT-64` sur la parallaxe en
  mode suivi — deux changements visibles qui ne doivent pas se découvrir à l'usage.
- Régénérer le cahier de test et relancer les quatre linters.

## Fichiers impactés
`Source/Elements/Levels/demo-*.json`, `Source/Elements/Levels/Plans/*` (nouveaux),
`scripts/generate_demo_plans.py` et `scripts/motifs/*` (nouveaux),
`Source/Elements/Assets/{Decors/*, CREDITS.md, README.md}`, `Documentation/Specification/*`,
`Documentation/Guide/*`, `Documentation/Lot/lots.md`,
`Documentation/Lot/LOT-69-plans-picturaux/*`, `CHANGELOG.md`.

## Tests (obligatoires)
- `test_parcours_complet.cpp` / `test_couverture_mecaniques.cpp` : les niveaux livrés chargent et se
  jouent inchangés — la migration ne touche que l'habillage.
- `test_plans_livres.cpp` (nouveau) : **aucun niveau du dépôt ne contient `decors`**, et **tout plan
  référencé existe aux dimensions attendues** pour sa densité. Le contrôle porte sur les fichiers du
  dépôt : ni `Core` (qui ne vérifie pas l'existence, `EX-NFR-011`) ni `HMI` (qui replie sur un
  damier, `EX-NFR-040`) ne signaleraient un plan manquant.

## Points d'attention
La stratégie « exigences retirées » doit être **validée en exécutant le lint**, pas supposée : c'est
le point où le volet documentaire peut casser silencieusement. Elle est préférable à un ajout dans la
liste d'exceptions du linter, dont le contrat documenté ne couvre que les invariants transverses et
le post-MVP — l'élargir à « retirée » modifierait ce contrat pour rien.

Générer les plans par script plutôt que commiter des images dessinées est un choix de
**reproductibilité**, dans la ligne du `LOT-66`. Il a une limite à assumer : le résultat est un
report fidèle de l'ancien habillage, pas un décor peint qui exploiterait vraiment sept plans. Ce
dernier est un acte de *level design*, hors de ce lot d'outillage.

## Definition de fait (DoD)
Aucun niveau ne porte plus de décors, tous les plans référencés existent, les quatre linters passent,
le cahier de test est régénéré, les statuts sont figés. `ctest` à 100 %.

## Exigences
`EX-LVL-009`, `EX-LVL-012`, `EX-LVL-013`, `EX-DEC-040`, `EX-DEC-032`, `EX-NFR-031`.
