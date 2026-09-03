# TACHE-03 — Ouverture avec garde-fou `dirty`, indicateurs d'état/erreurs ; documentation & vérification {#lot-36-tache-03-ouverture-garde-fou-doc}

**Lot :** [LOT-36](epic.md) · **Emplacement :** `Source/Editor`, `Documentation` · **Statut :** non commencé

## Contexte
Relie le panneau « Niveaux » à l'éditeur (LOT-35) : **ouvrir** un niveau le charge dans le brouillon,
en protégeant les **modifications non enregistrées**, et affiche lisiblement les **erreurs de
chargement**. Clôt le lot par la documentation et la vérification.

## Travail à réaliser
- **Ouverture** : double-clic / Entrée sur un niveau → `core::LevelLoader::loadFromFile` →
  `core::LevelDraft::fromLevel`. En cas d'échec, afficher le message issu de `LevelValidationError`
  (parse, hors bornes, id manquant, entrée/sortie non unique, mécanisme non résolu…) sans crash.
- **Garde-fou `dirty`** : si le brouillon courant a des modifications non enregistrées, proposer
  **Enregistrer / Abandonner / Annuler** (`QMessageBox`) avant de changer de niveau — aucune perte
  silencieuse (réutilise la sémantique de confirmation de l'éditeur historique).
- **Indicateurs d'état** : niveau courant (nom + chemin), marque « modifié », dernier message d'erreur
  de chargement, dans la barre d'état / le titre.
- **Documentation** :
  - `Documentation/Guide/guide-editeur.md` (ou `guide-niveaux.md`) : section « gérer ses niveaux »
    (parcourir, rechercher, créer/renommer/dupliquer/supprimer, ouvrir avec garde-fou).
  - Déclarer les exigences `EX-IHM-020`/`EX-IHM-021` dans
    `Documentation/Specification/editeur-niveaux.md`.
  - Regénérer le cahier de test si des cas sont ajoutés (`generate_cahier_test.py --check` vert en CI).

## Fichiers impactés
- `Source/Editor/` (ouverture, garde-fou, indicateurs).
- `Documentation/Guide/guide-editeur.md` (ou `guide-niveaux.md`),
  `Documentation/Specification/editeur-niveaux.md`, cahier de test généré.

## Tests (obligatoires)
- **Logique de garde-fou testable** : la décision « faut-il demander confirmation avant d'ouvrir ? »
  (fonction de l'état `dirty`) et le mapping `LevelValidationError` → message sont vérifiables sans Qt.
- **Non-régression** : `LevelLoader`/`LevelDraft::fromLevel` restent testés côté `Core`.
- **`--check` du cahier de test** vert.
- **Vérification manuelle** : ouvrir un niveau valide ; ouvrir avec un brouillon modifié (les trois
  branches Enregistrer/Abandonner/Annuler) ; ouvrir un fichier corrompu (message lisible, pas de
  crash).

## Points d'attention
- **Ne jamais perdre un brouillon** : le garde-fou couvre l'ouverture d'un autre niveau, la création,
  et la fermeture de l'application.
- **Messages d'erreur compréhensibles** : réutiliser la catégorisation `LevelValidationError`
  existante plutôt qu'un message générique.
- **Doxygen 1.9.8** : générer la doc localement avant push.

## Définition de fait (DoD)
- Ouverture fiable avec garde-fou `dirty` et erreurs lisibles ; guide et exigences à jour ; tests
  verts (dont cahier `--check`) ; `/W4 /WX`, Doxygen, lint verts ; critères du [LOT-36](epic.md)
  satisfaits (vérification manuelle).

## Exigences
`EX-IHM-020`/`EX-IHM-021` ; reconduit `EX-EDIT-001` ; réutilise `EX-EDIT-006`/`EX-EDIT-007`,
`EX-NFR-010`, `EX-NFR-040`.
