# TACHE-01 — Mutateurs de décors sur `LevelDraft` {#lot-50-tache-01-mutateurs-decor}

**Lot :** [LOT-50](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
LOT-49 a posé le modèle et deux opérations : ajouter, supprimer. Manipuler un décor en demande
davantage — déplacer, redimensionner, pivoter, changer de couche, réordonner — et toutes doivent
passer par `core::LevelDraft` pour bénéficier de l'historique par instantanés complets déjà en place
(`LOT-14`).

Le point délicat n'est pas le déplacement, c'est le **réordonnancement** : trois couches ne
suffisent pas à ordonner dix décors qui se recouvrent, et l'ordre dans le vecteur annexe fait foi.
L'exposer proprement est moins coûteux que d'introduire une profondeur continue.

## Travail à réaliser
- **Mutateurs** sur `LevelDraft`, chacun désignant un décor par son index :
  - déplacer (nouvelle position en unités monde) ;
  - redimensionner (nouvelle échelle) ;
  - pivoter (nouvelle rotation) ;
  - changer de couche ;
  - réordonner à l'intérieur d'une couche (avancer, reculer, mettre au premier ou au dernier rang) ;
  - supprimer.
- **Stabilité des index** : chaque mutation qui réordonne ou supprime invalide les index. Définir
  explicitement le contrat (renvoi du nouvel index, ou identifiant stable) — c'est ce qui évitera
  que la sélection de l'éditeur pointe le mauvais décor après une opération.
- **Historique** : gratuit via `LevelDraft::State`, à condition que chaque mutateur passe par le
  mécanisme d'instantané existant.
- **Bornes** : un index hors bornes ne doit ni planter ni lever — retour d'échec exploitable
  (`EX-NFR-040`).

## Fichiers impactés
- `Source/Core/Levels/LevelDraft.{h,cpp}`.
- `Source/Test/Unit/Core/Levels/test_decor_mutations.cpp` (nouveau).

## Tests (obligatoires)
- Chaque mutateur : cas nominal, index hors bornes, valeurs limites (échelle nulle ou négative,
  rotation hors de `[0, 2π[`).
- **Réordonnancement** : avancer le dernier, reculer le premier, ordre préservé pour les autres.
- Changement de couche : le décor quitte l'ordre de son ancienne couche et rejoint celui de la
  nouvelle, à un rang défini.
- Annulation et rétablissement de chaque mutation.
- Contrat de stabilité des index respecté après réordonnancement et suppression.
- Tout dans `Core`, sans GPU.

## Points d'attention
- **Ne pas introduire de nouveau mécanisme d'historique.** Les instantanés complets restent adaptés à
  la taille des niveaux du projet (décision de `LOT-14`, rappelée dans les non-objectifs de
  l'éditeur).
- Une échelle nulle ou négative doit être rejetée ou normalisée : un décor invisible ou retourné par
  accident est un piège d'usage.
- Le réordonnancement ne doit **pas** changer la couche, et le changement de couche ne doit pas
  perdre le rang de façon arbitraire — définir les deux comportements, ne pas les laisser émerger.

## Définition de fait (DoD)
- Les six mutateurs existent, sont sûrs hors bornes, couverts par l'annulation, et le contrat de
  stabilité des index est défini et testé ; tests `Core` verts ; `/W4 /WX` propre.

## Exigences
`EX-DEC-010` (déplacer, redimensionner, superposer, supprimer) ; réutilise `EX-EDIT-005`
(annuler/refaire), `EX-DEC-001` (transform libre), `EX-DEC-002` (couches), `EX-NFR-040` (erreur
récupérable), `EX-NFR-011` (frontière `Core`/`HMI`).
