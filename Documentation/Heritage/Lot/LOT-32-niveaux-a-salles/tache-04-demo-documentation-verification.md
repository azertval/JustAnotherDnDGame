# TACHE-04 — Niveau de démonstration, documentation et vérification {#lot-32-tache-04-demo-documentation-verification}

**Lot :** [LOT-32](epic.md) · **Emplacement :** `Source/Elements`, `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : livrer le contenu qui **prouve** le découpage en salles (`RoomGrid`,
TACHE-01/02/03) en conditions réelles, et aligner la documentation sur ce qui a été livré, comme
pour chaque lot précédent.

## Travail à réaliser
- **Niveau de démonstration `demo-salles.json`** (`Source/Elements/Levels`) :
  - Au moins `2×2` salles (donc strictement plus grand que la taille de salle sur les deux axes).
  - **Entrée** dans une salle de coin, **sortie** dans une autre — un seul chemin entrée→sortie au
    global (`EX-LVL-004`, inchangé).
  - Au moins **une salle intermédiaire ouverte sur plusieurs voisines** (au moins deux bords
    franchissables) — illustre concrètement « une salle avec plusieurs entrées/sorties » sans
    aucune tuile ni configuration dédiée (couloirs peints en conséquence).
  - Franchissable sans blocage (aucune situation sans issue, cohérent avec `niveaux.md` section 3).
  - Intégré à la **séquence** de niveaux jouée (même mécanisme que les autres `demo-*.json`, cf.
    `GameScreen`/fabrique d'écrans, `LOT-09` TACHE-05).
- **`Documentation/Guide/guide-rendu.md`** : nouvelle section décrivant la partition en salles
  (`RoomGrid`), la caméra à coupure nette en jeu, et pourquoi c'est une **seconde** stratégie de
  cadrage (à côté du zoom « niveau entier » de `LOT-16`, toujours valide pour un niveau tenant dans
  une seule salle) plutôt qu'un remplacement.
- **`Documentation/Guide/guide-editeur.md`** : nouvelle section décrivant le quadrillage de salles
  (repère visuel, TACHE-03) et le conseil de level design qui en découle (aligner les couloirs sur
  les bords de salles pour des transitions lisibles).
- **`Documentation/Specification/rendu-technique.md`** : nouvelle exigence `EX-REN-015` (caméra par
  salle), correction de portée d'`EX-REN-013` (scopée aux niveaux tenant dans une seule salle).
- **`Documentation/Specification/editeur-niveaux.md`** : nouvelle section (à la suite de la section
  9, `LOT-31`) et nouvelle exigence `EX-EDIT-023` (quadrillage de salles).
- **`Documentation/Specification/niveaux.md`** : si pertinent, une ligne de traçabilité mentionnant
  que le découpage en salles est un **comportement de cadrage `HMI`**, sans impact sur le format de
  fichier (`EX-LVL-003`) ni la validation (`EX-LVL-004`) — pour éviter qu'un futur lecteur ne
  cherche une notion de salle côté `Core`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-32 (`EX-REN-015`, `EX-EDIT-023`,
  `EX-REN-013` corrigée), à l'image des entrées LOT-16/31 déjà présentes.
- **Doxygen** : génération locale complète (`doxygen Doxyfile` depuis `Documentation/`) avant de
  pousser, avec le **binaire de la CI (1.9.8)**, pas la version locale — piège déjà rencontré deux
  fois sur ce dépôt (LOT-15, LOT-31) : un span `` `Fichier.cpp::Nom` `` ou un caractère `<`/`>`
  littéral dans un span en accent grave passe le Doxygen local mais casse la CI.
- **`scripts/lint_exigences.py`** : doit rester vert (nouvelles exigences déclarées une fois,
  `EX-REN-013` réutilisée sans nouvelle déclaration, aucune référence orpheline).

## Fichiers impactés
- `Source/Elements/Levels/demo-salles.json` (nouveau).
- La séquence de niveaux jouée (fabrique d'écrans, cf. `LOT-09` TACHE-05).
- `Documentation/Guide/guide-rendu.md`, `Documentation/Guide/guide-editeur.md`.
- `Documentation/Specification/rendu-technique.md`, `Documentation/Specification/editeur-niveaux.md`,
  `Documentation/Specification/niveaux.md`.
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `doxygen Doxyfile` (depuis `Documentation/`, binaire CI 1.9.8) se termine avec un code de sortie
  `0` et **aucune** ligne de sortie (`QUIET=YES`, `WARN_AS_ERROR=FAIL_ON_WARNINGS`).
- Un test **système** (`Source/Test/Systeme`, même patron que les niveaux de démonstration
  existants) rejoue `demo-salles.json` bout en bout (entrée → sortie, en franchissant au moins deux
  frontières de salles) — preuve que le niveau est franchissable et que le passage de salle en jeu
  n'introduit aucun blocage physique (la caméra elle-même n'est pas observable par ce test, c'est
  la simulation `Core` sous-jacente qui l'est).
- Essai manuel en jeu : les quatre franchissements de frontière (haut/bas/gauche/droite) du niveau
  de démonstration produisent une coupure nette de caméra, sans zone hors champ ni tremblement.

## Points d'attention
- Cette tâche clôt le lot : vérifier que les critères d'acceptation d'`epic.md` sont tous
  effectivement remplis avant de le marquer terminé (statut + cases ✅ du tableau des tâches).
- Ne pas décrire l'historique de conception (« on a hésité entre X et Y ») dans les commentaires de
  code — cette nuance reste dans `epic.md` (décisions de cadrage), pas dans le code livré.

## Définition de fait (DoD)
- Niveau de démonstration livré et intégré à la séquence ; documentation cohérente avec le code
  livré ; lint des exigences et génération Doxygen locale (binaire CI) verts ; `CHANGELOG.md` à
  jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
`EX-LVL-004` (invariant entrée/sortie, vérifié par le niveau de démonstration), `EX-REN-015`,
`EX-EDIT-023` — aucune exigence propre nouvelle au-delà de celles déjà couvertes par TACHE-01/02/03.
