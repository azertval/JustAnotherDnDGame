# LOT-ANNEXE-07 — Espace d'action, décodage politique et format de rejeu v1 {#lot-annexe-07}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-05](@ref lot-annexe-05) (`HeadlessLevelEnvironment`,
> `core::PlayerInput`) et [LOT-ANNEXE-01](@ref lot-annexe-01) (`aisolver::Rng`). Troisième lot de la
> génération 1 : définit à la fois la **sortie** que produira toute politique entraînée et le
> **format de fichier** dans lequel exporter la séquence gagnante — volontairement tôt (génération
> 1, pas 5), pour que la génération 2 dispose déjà d'un format cible dès son premier export.

## Objectif
`HeadlessLevelEnvironment` (`LOT-ANNEXE-05`) attend un `core::PlayerInput` à chaque pas, mais rien ne
définit encore comment la sortie d'un réseau de neurones (`LOT-ANNEXE-03`) devient un tel `core::
PlayerInput`, ni comment une distribution de probabilité sur des actions (nécessaire à l'exploration
de la génération 3) se traduit en une décision concrète, déterministe ou non selon le contexte. Ce
lot répond à ces deux besoins et, parce que la génération 2 (`LOT-ANNEXE-10`/`11`) doit déjà pouvoir
**exporter** la séquence d'actions gagnante d'un entraînement réussi, définit dans la foulée le
format de fichier de rejeu — sans quoi la génération 2 devrait inventer un format transitoire que la
génération 5 réécrirait ensuite, au prix d'une migration évitable.

## Périmètre

### Inclus
- **Espace d'action discret** (`aisolver::ActionSpace`) : produit cartésien direction (gauche /
  neutre / droite) × saut-appuyé (oui/non) × saut-maintenu (oui/non) × dash (oui/non), dérivé des
  champs de `core::PlayerInput` (`moveX`, `jumpPressed`, `jumpHeld`, `dashPressed`) ; `moveY` fixé à
  `0` (aucun niveau démo n'exploite l'orientation du dash vers le haut/bas comme condition de
  réussite identifiée à ce stade — voir Décisions de cadrage).
- **Décodage déterministe** (`aisolver::decodeArgmax`) : sélectionne l'action de probabilité
  maximale dans la distribution produite par le réseau — utilisé pour le rejeu final exporté
  (`LOT-ANNEXE-11`) et pour tout modèle évolutionniste (`LOT-ANNEXE-10`, qui ne produit pas de
  distribution stochastique à proprement parler).
- **Décodage stochastique** (`aisolver::decodeStochastic`) : échantillonne une action selon la
  distribution (softmax déjà produite par le réseau, `LOT-ANNEXE-03`), pondérée par une température
  configurable, via une instance d'`aisolver::Rng` fournie par l'appelant — utilisé pour
  l'exploration pendant l'entraînement par gradient (génération 3).
- **Format de rejeu v1** (`Source/AiSolver/Replay`) : `aisolver::ReplayFile` — identifiant/empreinte
  du fichier de niveau d'origine, séquence ordonnée de `core::PlayerInput` (un par pas fixe),
  métadonnées (nom de l'algorithme, date, graine utilisée, statistiques finales de l'entraînement),
  numéro de version explicite. `aisolver::writeReplay`/`aisolver::readReplay` (lecture/écriture,
  aucune validation de cohérence à la lecture — ajoutée par `LOT-ANNEXE-17`, qui étend ce format).
- Tests : couverture de toutes les combinaisons d'action valides, absence de biais statistique dans
  le décodage stochastique, écriture/lecture round-trip du format de rejeu.

### Exclus (hors périmètre de ce lot)
- **Le réseau produisant la distribution d'action** : `LOT-ANNEXE-03` (structure), génération 2/3
  (entraînement) — ce lot consomme une distribution déjà produite, il ne la calcule pas.
- **Validation du fichier de rejeu à la lecture** (empreinte de niveau vérifiée, fichier corrompu
  détecté) : `LOT-ANNEXE-17`, qui stabilise et durcit ce format v1 à la lumière de l'usage réel des
  générations 2 et 3 — ce lot pose un format **lisible/inscriptible**, pas encore **validé**.
- **Lecture du fichier de rejeu par le jeu** (`HMI`) : `LOT-ANNEXE-18`, génération 5.
- **Orientation verticale du dash comme dimension de l'espace d'action** (`moveY ≠ 0`) : aucun
  niveau de la séquence `demo-*.json` ne conditionne sa résolution sur un dash orienté verticalement
  plutôt qu'horizontalement — l'ajouter maintenant grossirait l'espace d'action (×3 combinaisons
  supplémentaires) sans consommateur identifié ; réévaluable si un niveau d'entraînement futur
  l'exige.
- **Actions continues** (`moveX`/`moveY` à valeur flottante autre que `-1/0/1`) : `core::PlayerInput`
  les autorise, mais aucun script existant (`test_parcours_complet.cpp`) ni aucune mécanique du jeu
  n'exploite une gradation continue — un espace discret est suffisant et bien plus simple à faire
  apprendre à un réseau de petite taille (`LOT-ANNEXE-03`).

## Décisions de cadrage
- **Espace d'action discret et fini, jamais continu.** `core::PlayerInput::moveX`/`moveY` acceptent
  n'importe quelle valeur flottante dans `[-1, 1]`, mais tous les scripts existants
  (`test_parcours_complet.cpp`) et toute la logique du jeu ne distinguent en pratique que
  `{-1, 0, 1}` — un espace d'action discret réduit la sortie du réseau à un nombre fini de neurones
  de sortie (une distribution catégorielle, `softmax`), bien plus simple à faire converger qu'une
  sortie continue, pour un bénéfice de expressivité non demandé par le jeu lui-même.
- **`jumpPressed` et `dashPressed` restent des booléens de front, jamais maintenus au sens de
  l'espace d'action.** Une politique choisit une action **par pas** ; `core::PlayerInput` interprète
  déjà `jumpPressed`/`dashPressed` comme des fronts montants côté `Core` (`CharacterPhysicsSystem`,
  `LOT-ANNEXE-05` ne modifie pas ce contrat) — si la politique choisit « saut » deux pas de suite,
  `Core` ne déclenchera qu'un unique saut au premier front, exactement comme un joueur humain qui
  maintient la touche ; l'espace d'action n'a pas besoin de modéliser cette distinction, il se
  contente de fournir un `jumpPressed` cohérent avec l'intention du pas courant.
- **Format de rejeu défini dès la génération 1, délibérément avant tout usage réel** (voir Objectif) :
  compromis assumé — un format défini sans retour d'usage réel risque de manquer des champs, ce que
  `LOT-ANNEXE-17` corrige explicitement plus tard sans rupture de compatibilité (numéro de version
  déjà prévu ici).
- **Empreinte de niveau incluse dans le format dès cette version**, bien que sa **vérification** à
  la lecture n'arrive qu'en `LOT-ANNEXE-17` : le champ existe pour que les fichiers exportés par la
  génération 2 la portent déjà — l'ajouter rétroactivement à des fichiers déjà écrits serait une
  migration de format évitable en le prévoyant maintenant.
- **La température du décodage stochastique est un paramètre d'appel, jamais une constante figée
  dans `decodeStochastic`** : l'exploration en début d'entraînement (température haute, actions
  plus uniformément réparties) et en fin d'entraînement (température basse, proche du déterministe)
  sont des besoins différents que la génération 3 doit pouvoir régler sans modifier ce lot.
- **Toute source d'aléatoire du décodage stochastique passe par une instance d'`aisolver::Rng`
  fournie par l'appelant**, jamais une graine interne à `decodeStochastic` — cohérence avec la
  décision transverse de `LOT-ANNEXE-01` (aucune source d'aléatoire basée sur l'horloge, tout est
  reproductible à partir d'une graine explicite).

## Notions abordées
Voir @ref guide-annexe-apprentissage-renforcement, section « Action, politique » (politique
déterministe vs stochastique, pourquoi l'exploration nécessite un décodage stochastique). Le
format de rejeu lui-même (TACHE-03) est une décision d'ingénierie logicielle propre au projet, pas
une notion de la littérature — aucune source externe associée.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-007`](@ref EX-IA-007).
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-008`](@ref EX-IA-008).
- Réutilisées : `EX-IA-001` (`aisolver::Rng`), `EX-IA-005` (`HeadlessLevelEnvironment`,
  `core::PlayerInput`), `EX-LVL-005` (principe de numéro de version sans rupture rétroactive, repris
  par analogie pour le format de rejeu).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-espace-action-discret.md) | Espace d'action discret | `Source/AiSolver/Env` | ✅ |
| [TACHE-02](tache-02-decodage-argmax-stochastique.md) | Décodage déterministe et stochastique | `Source/AiSolver/Env` | ✅ |
| [TACHE-03](tache-03-format-rejeu-v1.md) | Format de rejeu v1 | `Source/AiSolver/Replay` | ✅ |
| [TACHE-04](tache-04-tests.md) | Tests : couverture, absence de biais, round-trip | `Source/Test/Unit/AiSolver` | ✅ |

## Critères d'acceptation du lot
1. Toutes les combinaisons de l'espace d'action discret produisent un `core::PlayerInput` valide et
   distinct, sans collision entre deux combinaisons différentes.
2. `decodeArgmax` sélectionne toujours l'action de probabilité maximale, y compris en cas d'égalité
   (règle de départage documentée et testée).
3. `decodeStochastic`, sur un grand nombre de tirages à graine variable, reproduit la distribution
   d'entrée à une tolérance statistique documentée (aucun biais systématique vers une action).
4. Un fichier de rejeu écrit puis relu produit exactement la même séquence d'actions et les mêmes
   métadonnées (round-trip sans perte).
5. Un fichier de rejeu sans numéro de version explicite est lu comme la version initiale, sans
   erreur (même principe que `EX-LVL-005`).
6. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-05](@ref lot-annexe-05) (`core::PlayerInput`, via `HeadlessLevelEnvironment`)
et [LOT-ANNEXE-01](@ref lot-annexe-01) (`aisolver::Rng`). [LOT-ANNEXE-10](@ref lot-annexe-10)/[LOT-ANNEXE-11](@ref
lot-annexe-11) (génération 2) et toute la génération 3 en dépendent directement pour convertir la
sortie d'un réseau en action jouable et pour exporter leurs résultats. [LOT-ANNEXE-17](@ref
lot-annexe-17) étend ce format sans le dupliquer.

## Navigation des tâches
- @subpage lot-annexe-07-tache-01-espace-action-discret
- @subpage lot-annexe-07-tache-02-decodage-argmax-stochastique
- @subpage lot-annexe-07-tache-03-format-rejeu-v1
- @subpage lot-annexe-07-tache-04-tests
