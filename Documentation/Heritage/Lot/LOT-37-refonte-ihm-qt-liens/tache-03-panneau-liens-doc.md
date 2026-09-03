# TACHE-03 — Panneau « Liens » (liste, surbrillance, suppression) ; documentation & vérification {#lot-37-tache-03-panneau-liens-doc}

**Lot :** [LOT-37](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Interface`,
`Documentation` · **Statut :** fait

> Réalisé dans `Source/HMI/Editor/LinkPanel.{h,cpp}` (dock, `QTableView`/`QStandardItemModel`,
> alimenté par `hmi::buildLinkRows`) et `Source/HMI/Interface/MainWindow.{h,cpp}` +
> `Source/Elements/UI/MainWindow.ui` (dock `LinksPanel`, `LAYOUT_VERSION` 2 → 3).

## Contexte
Complète la lisibilité des liaisons par une **vue tabulaire** dockée : lister toutes les liaisons du
niveau, mettre en surbrillance celle sélectionnée dans le viewport, et en supprimer. Clôt le lot par
la documentation et la vérification.

## Travail à réaliser
- **Dock « Liens »** (`QDockWidget`) : liste/table (`QTableView` ou `QListView`) des liaisons du
  brouillon — pour chaque entrée : type (mécanisme / danger commuté), coordonnées du déclencheur et de
  la cible, éventuel libellé lisible.
  - **Vue du modèle** : lit `LevelDraft` (`_mechanisms`, `_dangerLinks`) ; se rafraîchit quand le
    brouillon change (création/suppression via geste TACHE-02, chargement d'un niveau).
- **Sélection ↔ surbrillance** : sélectionner une ligne met en évidence la flèche correspondante dans
  le viewport (et réciproquement, survol viewport → ligne, si peu coûteux).
- **Suppression** : action/bouton « Supprimer le lien » → `LevelDraft::unlinkMechanism(cible)` ;
  rafraîchir liste + viewport.
- **Documentation** — **guides à mettre à jour** :
  - `Documentation/Guide/guide-editeur.md` : section « lier des mécanismes » réécrite (flèches,
    panneau Liens, création/suppression), en remplacement de la description par teinte.
  - `Documentation/Guide/guide-rendu.md` : documenter la **primitive de ligne/flèche** ajoutée au
    pipeline (TACHE-01).
  - Déclarer `EX-IHM-030`/`EX-IHM-031` dans `Documentation/Specification/editeur-niveaux.md` ; noter
    le remplacement de l'indication par teinte.
  - Regénérer le cahier de test si nécessaire (`generate_cahier_test.py --check` vert).

## Fichiers impactés
- `Source/Editor/LinkPanel.{h,cpp}` (dock, vue, modèle de liens).
- `Documentation/Guide/guide-editeur.md`, `Documentation/Specification/editeur-niveaux.md`, cahier de
  test généré.

## Tests (obligatoires)
- **Modèle de liens testable** : construire une liste d'affichage à partir d'un `LevelDraft` donné
  (nombre d'entrées, contenu, ordre déterministe) sans Qt ; la suppression retire la bonne entrée.
- **Non-régression** : `unlinkMechanism` testé côté `Core`.
- **`--check` du cahier de test** vert.
- **Vérification manuelle** : le panneau liste les liens ; sélectionner surligne la flèche ; supprimer
  met à jour liste et viewport ; cohérence après chargement d'un niveau existant.

## Points d'attention
- **Le panneau est une vue, pas un état** : il lit `LevelDraft` et déclenche `unlinkMechanism` ;
  aucune copie de l'état des liaisons.
- **Cohérence bidirectionnelle** viewport ↔ panneau à maintenir sur chaque mutation.
- **Doxygen 1.9.8** : générer la doc localement avant push.

## Définition de fait (DoD)
- Panneau « Liens » listant/surlignant/supprimant les liaisons, cohérent avec le viewport ; modèle
  **testé** ; guide et exigences à jour ; tests verts (dont cahier `--check`) ; `/W4 /WX`, Doxygen,
  lint verts ; critères du [LOT-37](epic.md) satisfaits (vérification manuelle).

## Exigences
`EX-IHM-031` (panneau listant/éditant les liaisons) ; `EX-IHM-030` ; reconduit `EX-EDIT-003` ;
`EX-NFR-010`.
