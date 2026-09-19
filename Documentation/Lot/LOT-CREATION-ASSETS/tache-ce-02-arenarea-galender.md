# CE-02 — Arenarea, Galender et l’Arène du Destin

> À faire après pilote, reprises CE-R01/CE-R02 et CE-01, selon consommateurs LOT27/96.
> Six unités métier proposées ; branche `lot/LOT-CREATION-ASSETS-central-empire-02`, une PR.
> [Contrat](epic.md) · [Région](regions/central-empire/preparation.md). Aucune génération dans cette préparation.

Servir le trajet du parvis à l’entraînement et au jugement d’arène. Galender est le mentor/collectionneur télékinétique (CC48 inspecté, SB PDF52/imprimée102) ; marbre, jardins et fontaines sont attestés à Arenarea (SB PDF50/imprimée98). Ne pas remplacer les champions prévus par une collection nouvelle.

| Unité | Famille | Source | Besoin et opération proposée |
|---|---|---|---|
| `galender` complet | PNJ nommé | CC48 ; SB52 | Dialogue et démonstration/combat ; créer après registre, portrait + six animations détaillés |
| Sol/terrain de combat | Tuile/décor | SB50 ; LOT27, atelier textures | UNE pièce manquante de l’Arena of Fate ; réutiliser/adapter textures Colisée validées |
| Accès/hall d’entraînement | Décor | SB50 | UNE pièce du passage parvis/entraînement ; adapter marbre/colonnes existants |
| Lame télékinétique | Effet/objet | CC48 et capacité effectivement montrée | Créer seulement si rendu séparé de l’animation ; sinon retirer cette unité |
| Détail rituel Tamera/Bauron | Symbole/objet | SB37/43, lune/soleil observés | UNE unité choisie si rituel/UI d’arène la montre ; réutiliser symbole global, ne pas prétendre présence imposée par canon |
| Héraut de l’Arène | PNJ commun | LOT16/27 ; SB50 contexte | Dialogue de jugement ; réutiliser rôle existant ou créer modèle explicite, tenue projet à documenter |

Réemployer armes communes et architecture CE-01 validées ; Ironhand uniquement après [refonte CE-R02](tache-ce-r02-refonte-ironhand.md), textures Arenarea/Colisée uniquement après [reprise CE-R01](tache-ce-r01-reprise-trois-sites.md). Lion et loup restent à auditer si leurs rencontres sont retenues. Vérifier silhouette/gabarit et qualité au lieu de présumer conformité. Galender et le héraut intégrés à la famille Npc respectent ses six animations, même si cette scène n’en joue qu’une partie ; une animation fausse est refaite. Le cast optionnel concerne seulement un autre profil qui le prévoit, par exemple un monstre sans sort. Autres champions, noble accusé et propriétés du casino attendent les consommateurs précis.

## Préparation, production et acceptation

- [ ] Chaque texture/pièce modifiée réussit la [carte de test T01](tache-01-carte-test-textures.md) : témoin figé, échelle, jonctions, parcours, captures réelles et revue visuelle, puis vérification dans sa carte consommatrice.

- [ ] Relier les lignes aux vrais IDs scène/quête/UI et au registre global ; comparer fichiers existants, figer créer/réutiliser/adapter. Retirer toute unité sans usage. La proposition ne déclare pas le consommateur déjà implémenté.
- [ ] Préparer fiche sourcée, observations et choix, A+B+C, palette et ancre ; références visuelles corpus permises au générateur mais **PDF/rendus/crops hors Git**. Noter page PDF/imprimée, hash et coordonnées ; aucune copie/calque.
- [ ] Détailler fichiers par unité : personnage animé = portrait et bandes nécessaires + `.anim.json`, sans fractionner sa livraison ; élément de décor = pièce et emprise précises, jamais pack non borné. Maximum dix unités, ici six.
- [ ] Pour toute famille nouvelle, ajouter son profil et QC au moment du besoin (code futur hors dossier documentaire). Générer dans ce chat ; outils `py -3.13` pour préparation/QC/intégration. Limite service = sauvegarde/reprise.
- [ ] Refaire uniquement animations fautives par passe X ; appliquer mesures étalonnées pilote, alpha, palette, silhouette, taille, mouvement, raccord, arme et ancrage. Ouvrir planche-contact et GIF ; consigner verdict et revue humaine en cas de doute. Incertain bloque.
- [ ] Intégrer seuls originaux validés, sans remplacer silencieusement les anciens ; garder `replaces`. Vérifier galerie EX-CNT-042, chargement réel `.anim.json`, captures après textures et parcours de la scène.
- [ ] Rapport de série, crédits, changelog, tests requis, `scripts/build.ps1`, CI verte et PR. Pas de génération en CI. Une série est finie quand ses unités retenues sont validées et intégrées, pas quand dix cases sont remplies.
