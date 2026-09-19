# CE-R02 — Refaire le soldat Ironhand

> Une unité métier complète, après calibration [T00](tache-00-socle-pilote-pnj.md), avant réemploi régional. Refonte obligatoire demandée par l’utilisateur.
> Branche prévue `lot/LOT-CREATION-ASSETS-ce-ironhand`, une PR avec rapport QC.

L’ancien modèle a contourné les critères : [LOT-93](../LOT-93-atelier-monstres/epic.md) documente notamment trois figures de hit complétées avec la première image d’idle via `--garde-finale hit`. Cette dérogation ne vaut plus validation. Refaire le portrait et toutes les animations à partir de l’identité Ironhand et des ancres calibrées, sans reconduire les défauts de l’ancien modèle.

- [ ] Préparer fiche, uniforme, équipement, silhouette, palette et références sourcées ; différencier emblème militaire et blason régional. Conserver le modèle ancien et ses empreintes comme comparaison, pas comme étalon QC. Références du corpus autorisées au générateur, hors Git.
- [ ] Conserver l’identité canonique `Monsters/ironhand-soldier`. Son profil actuel `medium` comporte idle 6, walk 8, hit 4, death 6 et attack 8 : **32 frames réellement produites**, portrait, cinq bandes et leurs `.anim.json`. Le profil monstre sans sort n’exige pas cast ; son absence n’est pas la dérogation à corriger.
- [ ] Régénérer portrait puis cinq animations avec mémoire et prompts A+B+C. Toute animation rejetée repart seule en passe X. Interdire frame empruntée à idle, duplication de remplissage ou détachement du bouclier compté comme personnage.
- [ ] Contrôler dimensions du profil, alpha, palette, hauteur, pied au sol, corps stable, continuité idle/walk, vraie locomotion, arme et bouclier uniques, effets autorisés par la fiche. Appliquer les seuils figés avant génération ; aucun passe-droit lié à l’ancien LOT-93.
- [ ] Relire portrait, planches-contact et GIF ; consigner comparaison ancien/nouveau et verdict de chaque animation. Charger réellement les cinq descripteurs et montrer le modèle dans la galerie EX-CNT-042.
- [ ] Vérifier les consommateurs actuels : trois sentinelles de Martpart et sept d’Arenarea référencent ce slug. Après reprise des textures [CE-R01](tache-ce-r01-reprise-trois-sites.md), contrôler sa taille, son ancrage et ses occultations en jeu, auprès du témoin validé de [T01](tache-01-carte-test-textures.md), sans changer cet étalon.
- [ ] Intégrer seulement après QC complet ; mettre à jour manifeste, crédits et changelog, conserver les liens de remplacement, exécuter les tests pertinents via `scripts/build.ps1`, obtenir CI verte et PR avec rapport.

Toute réutilisation à Ben’net ou ailleurs dépend de cette version refaite et validée. Un verdict indécis reste à revoir ; aucune acceptation automatique de la variante précédente.
