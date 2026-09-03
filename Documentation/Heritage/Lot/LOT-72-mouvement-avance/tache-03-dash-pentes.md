# TACHE-03 — Dash et pentes : déjà couvert, validation seulement {#lot-72-tache-03-dash-pentes}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` (tests) · **Statut :**
fait

## Contexte
Le cadrage initial de cette tâche proposait deux changements : faire suivre les pentes au dash
(qui ne les suivait pas, croyait-on) et convertir sa vitesse résiduelle en « glissade » à la sortie
d'un dash contre une pente.

Relecture du code avant implémentation (`CharacterPhysicsSystem::update`) : `resolveCollisionAndState`
— qui contient le balayage classique **et** la passe de suivi de pente/plafond
(`core::resolveSlopeFollow`/`resolveCeilingSlopeFollow`) — est appelée **inconditionnellement**
chaque pas, que `resolveVelocity` ait pris la branche dash ou non. Le suivi de pente s'applique donc
**déjà** pendant un dash, sans code supplémentaire : `resolveSlopeFollow` scanne toute l'étendue
horizontale couverte par la boîte pendant le pas (avant **et** après), pas seulement sa position
finale, ce qui couvre nativement les grands déplacements d'un dash comme les petits pas d'une
marche.

Quant à la « glissade de sortie » : elle n'a pas de sens séparé du suivi normal. Une fois le dash
terminé, le contrôle horizontal normal reprend immédiatement (`velocity.x = moveX × moveSpeed`), et
le suivi de pente recalcule la position verticale à **chaque** pas suivant à partir de la position
courante — il n'y a donc jamais d'« arrêt net » à corriger : soit le joueur maintient une direction
et continue naturellement de suivre la pente, soit il relâche et s'arrête, comme n'importe quel
déplacement normal sur une pente.

**Décision** : aucun code de production n'est ajouté. Cette tâche ajoute uniquement les tests qui
manquaient pour **vérifier explicitement** ce comportement déjà correct (`EX-GP-060`), sur le modèle
de la TACHE-05 (wall slide).

## Travail réalisé
- Test d'intégration `DashSuitUnePenteAscendante`
  (`Source/Test/Integration/test_physique_personnage.cpp`) : dasher puis marcher (direction
  maintenue) le long d'une pente ascendante ne clippe jamais sous/au-dessus de sa surface.

## Fichiers impactés
- `Source/Test/Integration/test_physique_personnage.cpp`.

## Tests (obligatoires)
- **Dash sur pente ascendante** : le bord bas reste dans les bornes de la surface pendant toute la
  traversée, quelle que soit la vitesse du dash.

## Points d'attention
- Ne pas dupliquer `core::resolveSlopeFollow`/`resolveCeilingSlopeFollow` : toute retouche future du
  suivi de pente profite automatiquement au dash, sans code séparé à maintenir.
- Le test doit maintenir la direction (`moveX`) au-delà de la fin du dash pour que la marche normale
  prenne le relais sans à-coup après son expiration — sans quoi le personnage s'arrête simplement là
  où le dash s'est terminé (comportement normal, pas un bug).

## Définition de fait (DoD)
- Non-régression confirmée par test ; `ctest` vert.

## Exigences
`EX-GP-060`, `EX-GP-017`, `EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007`, `EX-NFR-002`.
