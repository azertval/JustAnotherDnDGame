# CE-01 — Martpart, Myr et la piste du repaire

> À faire après [T00](tache-00-socle-pilote-pnj.md) et les reprises CE-R01/CE-R02, selon consommateurs LOT16/27/96.
> Six unités métier proposées ; branche `lot/LOT-CREATION-ASSETS-central-empire-01`, une PR.
> [Contrat](epic.md) · [Région](regions/central-empire/preparation.md). Aucune génération dans cette préparation.

La quête « Les enfants de Martpart » donne la destination : dialogue Myr au marché, témoins, ruelles, accès au repaire. Myr dirige les Death Merchants (SB PDF51/imprimée101, CC91). Le Third Eye est lié à Uptown dans SB PDF50 ; son repaire sous Martpart est une **adaptation projet explicite LOT27**, pas un déplacement de canon à masquer.

| Unité | Famille | Source | Besoin et opération proposée |
|---|---|---|---|
| `myr` complet | PNJ nommé | CC91 observé ; SB51 | Dialogue et figurine d’exploration/combat de quête ; créer après contrôle registre |
| Pièce de ruelle du marché | Décor/tuile | SB50 imprimées98–99 ; LOT96 | Passage vers indices ; adapter UNE pièce manquante après audit `Assets/Scene/martpart` |
| Étal interactif | Décor/objet | SB50 ; LOT16 | Commerce/interaction témoin si utilisé ; réutiliser ou adapter UNE pièce existante |
| Accès intérieur du repaire | Décor | LOT27 ; SB50 contexte seulement | Entrée de l’exploration ; créer UNE pièce précisément manquante, autres pièces en série ultérieure si nécessaires |
| Coffre du registre | Objet interactif | LOT16/27 adaptation scénario | Interaction donnant preuve/flag ; réutiliser ou adapter, forme non canonique à définir |
| Signalétique impériale | Symbole/objet | SB25 imprimée48, écu dragon observé | Contrôle urbain uniquement si affiché ; réutiliser ou créer symbole canonique global |

Réemplois conditionnés aux reprises obligatoires : soldat Ironhand refait dans [CE-R02](tache-ce-r02-refonte-ironhand.md), textures Martpart intégralement revues dans [CE-R01](tache-ce-r01-reprise-trois-sites.md). Aucune ancienne acceptation ne dispense de ces tâches. Les parents, receleur et bandits manquants constituent une prochaine série mixte de cette même boucle, après inventaire des rôles réellement chargés. L’identité Myr ne sert pas d’archétype de tous les assassins. Unité décor supprimée si déjà satisfaite, sans remplacement de remplissage.

## Préparation, production et acceptation

- [ ] Chaque texture/pièce modifiée réussit la [carte de test T01](tache-01-carte-test-textures.md) : témoin figé, échelle, jonctions, parcours, captures réelles et revue visuelle, puis vérification dans sa carte consommatrice.

- [ ] Relier les lignes aux vrais IDs scène/quête/UI et au registre global ; comparer fichiers existants, figer créer/réutiliser/adapter. Retirer toute unité sans usage. La proposition ne déclare pas le consommateur déjà implémenté.
- [ ] Préparer fiche sourcée, observations et choix, A+B+C, palette et ancre ; références visuelles corpus permises au générateur mais **PDF/rendus/crops hors Git**. Noter page PDF/imprimée, hash et coordonnées ; aucune copie/calque.
- [ ] Détailler fichiers par unité : personnage animé = portrait et bandes nécessaires + `.anim.json`, sans fractionner sa livraison ; élément de décor = pièce et emprise précises, jamais pack non borné. Maximum dix unités, ici six.
- [ ] Pour toute famille nouvelle, ajouter son profil et QC au moment du besoin (code futur hors dossier documentaire). Générer dans ce chat ; outils `py -3.13` pour préparation/QC/intégration. Limite service = sauvegarde/reprise.
- [ ] Refaire uniquement animations fautives par passe X ; appliquer mesures étalonnées pilote, alpha, palette, silhouette, taille, mouvement, raccord, arme et ancrage. Ouvrir planche-contact et GIF ; consigner verdict et revue humaine en cas de doute. Incertain bloque.
- [ ] Intégrer seuls originaux validés, sans remplacer silencieusement les anciens ; garder `replaces`. Vérifier galerie EX-CNT-042, chargement réel `.anim.json`, captures après textures et parcours de la scène.
- [ ] Rapport de série, crédits, changelog, tests requis, `scripts/build.ps1`, CI verte et PR. Pas de génération en CI. Une série est finie quand ses unités retenues sont validées et intégrées, pas quand dix cases sont remplies.
