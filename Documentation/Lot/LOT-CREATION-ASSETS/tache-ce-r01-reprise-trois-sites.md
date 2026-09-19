# CE-R01 — Reprise intégrale de Martpart, Arenarea et du Colisée

> Priorité de remise en conformité avant [CE-01](tache-ce-01-martpart-myr.md) et [CE-02](tache-ce-02-arenarea-galender.md). Dépend de [T01](tache-01-carte-test-textures.md).
> L’utilisateur signale que les trois sites échouent au test de cohérence et d’imbrication. Cette préparation ne prétend pas avoir reproduit cet échec.

« Market » correspond ici à Martpart. Ces trois sites du Central Empire sont à reprendre intégralement : textures, pièces, variantes, échelle, ancres, emprises, raccords et affectations dans les cartes. Aucun ensemble existant n’est réputé validé. Le travail comprend les nouvelles générations nécessaires et la requalification de chaque entrée ; aucune exemption par simple présence dans Git ou ancienne acceptation.

## Inventaire et séries de dix maximum

| Site | État constaté dans les fichiers | Découpage de reprise |
|---|---|---|
| Martpart | `Assets/Scene/martpart/manifest.json` : 22 textures | M01–M03 au minimum, au plus 10 unités par série ; couverture exhaustive des 22 entrées |
| Arenarea | `Levels/capital/arenarea.json` utilise actuellement `scene: martpart`, sans jeu `Scene/arenarea` distinct | A01 puis A02… selon inventaire des pièces à créer/adapter et affectations à corriger ; chaque série ≤ 10 |
| Colisée | `Assets/Scene/coliseum/manifest.json` : 36 textures | C01–C04 au minimum, au plus 10 unités par série ; couverture exhaustive des 36 entrées |

Les chemins ci-dessus sont relatifs à `Source/Elements/`. Les nombres sont ceux du dépôt lu, à figer au démarrage. Toute pièce ou variante nouvelle compte comme une unité supplémentaire. Un ensemble partagé corrigé n’est pas régénéré pour chaque carte, mais est testé dans **chaque** carte consommatrice. Pour Arenarea, définir son identité marbre/jardins/fontaines à partir de la [préparation régionale](regions/central-empire/preparation.md), puis décider quelles pièces restent communes et si un profil `Scene/arenarea` distinct est nécessaire.

- [ ] Établir la liste exhaustive clés → fichiers → placements/consommateurs, y compris pièces non affectées ou manquantes ; chaque entrée reçoit une décision motivée refaire/corriger/requalifier, aucune entrée oubliée. Une réutilisation exige tous les nouveaux contrôles.
- [ ] Capturer l’état initial dans T01 et dans `capital/martpart`, `capital/arenarea`, `coliseum`, avec le même témoin. Documenter les défauts signalés et ceux reproduits sans inventer de mesures.
- [ ] Répartir les clés nommément entre sous-séries cohérentes de dix maximum avant génération : sols et transitions, murs et ouvertures, volumes et mobilier. Inscrire les dépendances entre séries ; un raccord non encore disponible reste bloquant pour l’ensemble concerné.
- [ ] Revoir toutes les fiches, références régionales, palettes, proportions et prompts. Régénérer les éléments non conformes ; réextraire ou corriger l’ancrage seulement lorsque le dessin satisfait déjà le contrat. Ne pas masquer une erreur d’échelle par modification du personnage témoin.
- [ ] Pour chaque sous-série, appliquer T01, publier un rapport par clé et par raccord, puis vérifier les cartes livrées et la galerie. Modifier les matériaux/placements lorsque nécessaire à la cohérence, en conservant navigation et interactions.
- [ ] Une branche et une PR par sous-série (`lot/LOT-CREATION-ASSETS-ce-reprise-<serie>`), crédits, changelog, rapport QC et CI verte. Tout asset partagé impose une vérification de ses autres consommateurs.
- [ ] Clôturer seulement après couverture des trois sites, parcours complets, aucune texture sans verdict et aucune jonction requise en attente. Les ajouts CE-01/CE-02 utilisent alors ces bases revalidées.

Les sous-séries sont des unités de livraison, pas une autorisation de valider une carte partiellement corrigée. Les anciens résultats et captures servent à comparer, jamais d’exception aux nouveaux critères.
