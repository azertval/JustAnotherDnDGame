# LOT-ANNEXE-02 — Moteur d'autodiff maison {#lot-annexe-02}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor<float>`).
> Ajoute la différentiation automatique en mode *reverse* (rétropropagation) — condition **ferme**
> de l'apprentissage par gradient (génération 3, exigence non négociable de l'utilisateur, pas une
> option). Sans ce lot, seul l'algorithme évolutionniste (génération 2) resterait possible.

## Objectif
`Tensor<T>` (`LOT-ANNEXE-01`) sait stocker des valeurs et calculer, mais ne garde aucune trace de
**comment** un résultat a été obtenu — impossible d'en dériver un gradient. Or l'apprentissage par
gradient (génération 3, cf. [Lots annexes](@ref lots-annexe)) a besoin, pour chaque paramètre d'un
réseau, de savoir de combien la sortie varierait si ce paramètre variait d'une quantité
infinitésimale — c'est la définition même du gradient, et le calculer à la main pour chaque
opération composée serait aussi fastidieux qu'source d'erreurs. Ce lot construit ce mécanisme une
fois, générique, pour que les lots suivants (réseaux en `LOT-ANNEXE-03`, optimiseurs en
`LOT-ANNEXE-04`, apprentissage par gradient en génération 3) n'aient jamais à dériver une formule à
la main.

## Périmètre

### Inclus
- **Graphe de calcul dynamique** (`aisolver::autodiff::Node`) : construit **à l'exécution**, à
  chaque passe avant (style *define-by-run*, pas de graphe statique précompilé) — chaque nœud porte
  sa valeur (`Tensor<float>`), son gradient accumulé, et une règle de dérivation locale capturée en
  fermeture sur ses parents.
- **Opérations différentiables de base** : addition, multiplication (élément par élément), produit
  matriciel, et deux activations (`relu`, `tanh`) — chacune avec sa règle de dérivation associée.
- **Fabrique générique d'opérations** (`unaryOp`/`binaryOp`) exposée publiquement : les opérations
  ci-dessus sont construites **au-dessus** de cette fabrique plutôt que d'être des cas spéciaux
  câblés dans `Node` — nécessaire pour que `LOT-ANNEXE-03` ajoute deux nouvelles activations
  (`sigmoid`, `softmax`) sans modifier `Node` ni le moteur de rétropropagation.
- **`backward()`** : parcours topologique inverse du graphe depuis un nœud racine **scalaire**
  (tenseur à un seul élément — une perte, typiquement), accumulation des gradients dans chaque nœud
  parent.
- **Vérification de gradient par différences finies** (*gradient checking*), sous forme d'utilitaire
  réutilisable, condition **bloquante** avant qu'une opération différentiable soit consommée par un
  lot ultérieur.

### Exclus (hors périmètre de ce lot)
- **Dérivées d'ordre supérieur** (gradient du gradient) : l'entraînement par gradient visé
  (génération 3, SGD/Adam en `LOT-ANNEXE-04`) n'a besoin que du premier ordre — ajouter le second
  ordre maintenant serait un investissement sans consommateur identifié.
- **Opérations non encore consommées** (soustraction, division, exponentielle, logarithme,
  sélection d'un indice, `minimum`/`clamp`, `sigmoid`/`softmax`) : reportées à `LOT-ANNEXE-03`, qui
  les ajoute via la fabrique générique posée ici — `sigmoid`/`softmax` en TACHE-02, les autres en
  TACHE-05 (@ref lot-annexe-03-tache-05-operations-differentiables-complementaires) —, au moment où
  un réseau et une perte réels les consomment ; évite du code différentiable non exercé par un
  usage réel. **Attention** : ces opérations ne sont pas facultatives pour autant, la génération 3
  en dépend entièrement (`-log π(a|s) × G` n'est pas exprimable sans `logOp` ni `selectIndex`) —
  leur report est une question de calendrier, pas de périmètre du programme.
- **Parallélisation du graphe** (construction ou parcours multi-thread) : cohérent avec le RNG
  mono-thread de `LOT-ANNEXE-01` et la boucle mono-thread déjà actée pour le jeu (`EX-ARCH-060`).
- **Réutilisation/mise en cache d'un graphe entre deux passes avant** : chaque passe avant reconstruit
  son propre graphe (jeté après `backward()`, libéré par comptage de références) — plus simple et
  suffisant tant qu'aucun besoin de graphe statique (compilation, optimisation de graphe) n'apparaît.

## Décisions de cadrage
- **Graphe dynamique (*define-by-run*), pas de graphe statique compilé à l'avance.** Cohérent avec
  la contrainte « from scratch » (pas de compilateur de graphe à écrire) et avec l'usage réel :
  chaque exemple d'entraînement (génération 3) traverse le réseau une fois, produit un graphe, le
  différentie, le jette. Un graphe statique n'apporterait un bénéfice que pour une optimisation de
  performance non demandée à ce stade.
- **La rétropropagation n'accepte qu'une racine scalaire** (`Tensor` à un seul élément). Une perte
  d'entraînement est par construction un scalaire ; différentier une sortie non scalaire
  supposerait une matrice jacobienne complète, non nécessaire ici. Un appelant qui a besoin de
  différentier un vecteur doit d'abord le réduire (`aisolver::sum`/`mean`, `LOT-ANNEXE-01`) —
  cohérent avec ce que fait toute fonction de perte de toute façon.
- **Fabrique générique (`unaryOp`/`binaryOp`) plutôt qu'une liste fermée d'opérations câblées dans
  `Node`.** Décision structurante : sans elle, `LOT-ANNEXE-03` devrait rouvrir et modifier `Node.h`/
  `backward()` pour ajouter `sigmoid`/`softmax`, un lot censé n'être qu'un consommateur de
  l'autodiff. Avec elle, ajouter une opération est un ajout **local** (nouvelle fonction utilisant la
  fabrique), jamais une modification du moteur.
- **Le gradient checking est un utilitaire de production** (`Source/AiSolver/Math/Autodiff/
  GradientCheck.h/.cpp`), pas un fichier de test isolé : il est réutilisé tel quel par les tests de
  `LOT-ANNEXE-03` pour valider `sigmoid`/`softmax` avant qu'un réseau ne les consomme — le même
  garde-fou doit s'appliquer à toute opération différentiable ajoutée après ce lot, sans dupliquer
  la logique de différences finies dans chaque suite de tests.
- **Les gradients s'accumulent (`+=`), jamais n'écrasent (`=`).** Un nœud utilisé plusieurs fois
  dans un même graphe (ex. un biais partagé, ou plus tard un poids réutilisé sur plusieurs
  observations d'un lot) doit recevoir la **somme** des contributions de chaque chemin qui le
  traverse — omettre l'accumulation romprait silencieusement tout graphe non strictement arborescent.

## Notions abordées
Voir @ref guide-annexe-autodiff (dérivée, règle de la chaîne, graphe de calcul, mode direct vs
inverse, rétropropagation, différences finies). Sources directes : Rumelhart, Hinton, Williams
(1986, popularisation de la rétropropagation) ; Linnainmaa (1970, mode inverse de différentiation
automatique) ; Baydin, Pearlmutter, Radul, Siskind (2018, synthèse moderne direct/inverse) —
bibliographie complète dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-002`](@ref EX-IA-002).
- Réutilisées : `EX-IA-001` (`Tensor<float>`, socle de valeur de chaque nœud), `EX-NFR-010`/
  `EX-NFR-012`/`EX-NFR-013`/`EX-NFR-020` (testabilité headless, conventions, `/W4 /WX`, couverture de
  tests), `EX-ARCH-001` (sens de dépendance, `AiSolver → Core`, jamais l'inverse).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-graphe-calcul.md) | Graphe de calcul dynamique (`Node`) | `Source/AiSolver/Math/Autodiff` | ✅ |
| [TACHE-02](tache-02-operations-differentiables.md) | Opérations différentiables de base | `Source/AiSolver/Math/Autodiff` | ✅ |
| [TACHE-03](tache-03-retropropagation.md) | `backward()` : parcours topologique inverse | `Source/AiSolver/Math/Autodiff` | ✅ |
| [TACHE-04](tache-04-verification-gradient.md) | Vérification de gradient par différences finies | `Source/Test/Unit/AiSolver/Math` | ✅ |

## Critères d'acceptation du lot
1. Un graphe construit à partir d'opérations composées (ex. `matmul` suivi de `relu` puis d'une
   somme) calcule, via `backward()`, un gradient identique (tolérance `1e-2` relative) au gradient
   numérique obtenu par différences finies, pour **chacune** des opérations livrées (`add`, `mul`,
   `matmul`, `relu`, `tanh`).
2. Un nœud réutilisé plusieurs fois dans un même graphe reçoit la **somme** des gradients de chacun
   de ses usages (vérifié par test explicite, pas seulement déduit du gradient checking global).
3. `backward()` refuse (assertion) d'être appelé sur un nœud racine non scalaire.
4. Ajouter une nouvelle opération différentiable (démontré par un cas de test dédié utilisant
   directement `unaryOp`/`binaryOp`, sans modifier `Node.h`) ne nécessite **aucune** modification du
   moteur (`Node`, `backward()`) — vérifie concrètement la décision de cadrage sur la fabrique
   générique.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `scripts/lint_exigences.py` verts.

## Dépendances
Bâtit sur [LOT-ANNEXE-01](@ref lot-annexe-01) (`Tensor<float>`, seul type de valeur porté par un
`Node`). Aucun autre lot annexe amont. `LOT-ANNEXE-03` (réseaux) et `LOT-ANNEXE-04` (optimiseurs,
via les gradients accumulés dans chaque `Node`) en dépendent directement.

## Navigation des tâches
- @subpage lot-annexe-02-tache-01-graphe-calcul
- @subpage lot-annexe-02-tache-02-operations-differentiables
- @subpage lot-annexe-02-tache-03-retropropagation
- @subpage lot-annexe-02-tache-04-verification-gradient
