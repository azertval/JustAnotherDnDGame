# TACHE-01 — Émetteur de particules déterministe {#lot-53-tache-01-emetteur-deterministe}

**Lot :** [LOT-53](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** ✅ fait

## Contexte
Un système de particules est le premier endroit d'un moteur où l'on est tenté d'utiliser l'horloge
et un générateur aléatoire non maîtrisé — c'est plus simple à écrire, et « ce n'est que du visuel ».

Ce serait ici une erreur structurelle : tout le projet tient le **déterminisme** depuis `LOT-01`
(`EX-NFR-002`), la simulation avance à pas fixe (`EX-REN-021`), et les tests système reposent sur la
reproductibilité. Une source d'aléa non maîtrisée dans `Core` rendrait deux exécutions identiques
visuellement différentes, et ouvrirait la porte à des divergences plus graves.

## Travail à réaliser
- **Composant et système de particules** dans `Core`, simulés au **pas fixe** : position, vitesse,
  durée de vie, teinte, avec intégration simple et disparition en fin de vie.
- **Aléa maîtrisé** : générateur pseudo-aléatoire à **graine explicite**, jamais l'horloge. La graine
  dérive d'une valeur reproductible (numéro de pas, identifiant d'entité) de sorte que deux
  exécutions de la même séquence d'entrées produisent exactement les mêmes particules.
- **Budget borné** : nombre maximal de particules simultanées, **constante nommée**. Au-delà, les
  plus anciennes sont recyclées plutôt que d'allouer — pas de croissance non bornée.
- **Aucune collision, aucun effet de gameplay** : les particules ne sont pas testées contre la grille,
  ne déclenchent rien, et ne sont lues par aucun système de jeu (`EX-ARCH-012`).
- **Description d'effet** : forme des données décrivant un effet (nombre de particules, dispersion,
  durée de vie, vitesse initiale), en constantes nommées — pas de nombres magiques dispersés.

## Fichiers impactés
- `Source/Core/Ecs/Components/Particle.h` (nouveau).
- `Source/Core/Ecs/Systems/ParticleSystem.{h,cpp}` (nouveau).
- `Source/Core/Math/` (générateur pseudo-aléatoire déterministe, si absent).
- `Source/Core/CMakeLists.txt`.
- `Source/Test/Unit/Core/Ecs/test_particle_system.cpp` (nouveau).

## Tests (obligatoires)
- **Déterminisme** : deux exécutions de la même séquence produisent exactement les mêmes positions
  de particules à chaque pas. C'est le test central de la tâche.
- **Budget** : émettre bien au-delà du maximum ne dépasse jamais le plafond, et recycle les plus
  anciennes.
- Durée de vie : une particule disparaît au pas attendu, ni avant ni après.
- Aucun effet sur les entités de gameplay présentes dans le même monde.
- Tests `Core` purs, sans GPU.

## Points d'attention
- **Ne jamais utiliser l'horloge système**, même indirectement (initialisation de graine par défaut
  de la bibliothèque standard, par exemple). C'est le piège exact que ce lot doit éviter.
- Le recyclage doit être **déterministe** lui aussi : quelle particule est recyclée ne peut pas
  dépendre d'un ordre d'itération instable (le sparse set de l'ECS ne garantit pas d'ordre).
- Le coût par pas doit rester borné : quelques centaines de particules, intégration simple, pas de
  tri.

## Définition de fait (DoD)
- Les particules sont simulées au pas fixe, strictement déterministes, bornées par un budget nommé,
  sans aucun effet sur le gameplay ; déterminisme et budget assertés par test ; `/W4 /WX` propre.

## Exigences
`EX-REN-008` (effets visuels déterministes et bornés) ; réutilise `EX-NFR-002` (déterminisme),
`EX-REN-021` (pas fixe), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-005` (budget),
`EX-NFR-020` (tests unitaires `Core`).
