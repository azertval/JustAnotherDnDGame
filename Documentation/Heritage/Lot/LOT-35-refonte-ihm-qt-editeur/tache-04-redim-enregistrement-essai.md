# TACHE-04 — Redimensionnement, enregistrement, essai immédiat ; documentation & vérification {#lot-35-tache-04-redim-enregistrement-essai}

**Lot :** [LOT-35](epic.md) · **Emplacement :** `Source/Editor`, `Documentation` · **Statut :** non commencé

## Contexte
Complète l'éditeur Qt avec les actions « niveau » restantes (redimensionner, enregistrer, essayer),
puis **clôt le lot par la documentation et la vérification**. Ces actions réutilisent les chemins
`Core`/`HMI` existants ; la nouveauté est leur présentation en actions/dialogues Qt.

## Travail à réaliser
- **Redimensionnement** : dialogue Qt (`QDialog` avec deux `QSpinBox` largeur/hauteur, plafond
  `MAX_LEVEL_DIMENSION`) remplaçant la saisie texte `Ctrl+R` ; appelle `LevelDraft::resize` ; si
  destructeur (`wouldResizeDropContent`), **confirmation** (`QMessageBox`) avant application.
- **Enregistrement** : action `Ctrl+S` → `LevelDraft::toLevel()` (validation via `LevelLoader`) puis
  `core::LevelWriter::saveToFile` dans le dossier `Levels`. Message d'erreur lisible si brouillon
  invalide (**aucun fichier écrit**) ; confirmation avant écrasement d'un fichier différent. Indicateur
  « modifié » (`dirty`) dans le titre/barre d'état.
- **Essai immédiat (playtest)** : action → lance le niveau en cours dans le viewport (réutilise le
  chemin de rendu/jeu du [LOT-34](@ref lot-34) TACHE-04, alimenté par le `Level` validé **en
  mémoire**), `Échap` en sort et **restitue l'éditeur intact** (brouillon + historique préservés).
- **Barre d'état** : type de tuile actif, outil courant, position survolée, état `dirty`, dernier
  message.
- **Documentation** :
  - Mettre à jour `Documentation/Guide/guide-editeur.md` pour l'éditeur Qt (docks, palette arbre,
    outils, raccourcis, enregistrement, essai).
  - Déclarer/mettre à jour les exigences `EX-IHM-010`/`EX-IHM-011` dans
    `Documentation/Specification/editeur-niveaux.md`.
  - Regénérer le cahier de test si des cas sont ajoutés (`scripts/generate_cahier_test.py`).

## Fichiers impactés
- `Source/Editor/` (dialogue de redimensionnement, actions Enregistrer/Essai, barre d'état).
- `Documentation/Guide/guide-editeur.md`, `Documentation/Specification/editeur-niveaux.md`, cahier de
  test généré.

## Tests (obligatoires)
- **Non-régression** : `LevelDraft::resize`/`toLevel` et `LevelWriter` sont déjà testés (`Core`) —
  vérifier qu'ils restent verts et non dupliqués. Un même contenu enregistré produit un fichier
  **identique** à l'éditeur historique (sérialisation `LevelWriter` inchangée).
- **Logique de dialogue testable** : bornage de taille et détection « redimensionnement destructeur »
  (via `wouldResizeDropContent`) vérifiables sans Qt.
- **`--check` du cahier de test** vert en CI (`lint-exigences`).
- **Vérification manuelle** : redimensionner (avec/sans perte), enregistrer (valide/invalide/
  écrasement), essayer puis revenir.

## Points d'attention
- **Garde-fous de perte de données** : conserver les confirmations existantes (redimensionnement
  destructeur, écrasement, quitter avec `dirty`).
- **Essai n'altère jamais le brouillon** : parité stricte avec `EditorScreen` (l'essai lit un `Level`
  en mémoire, n'écrit rien).
- **Doxygen 1.9.8** : générer la doc localement avant push.

## Définition de fait (DoD)
- Redimensionner/enregistrer/essayer fonctionnent avec leurs garde-fous ; guide éditeur et exigences à
  jour ; tests verts (dont cahier `--check`) ; `/W4 /WX`, Doxygen, lint verts ; critères du
  [LOT-35](epic.md) satisfaits (vérification manuelle).

## Exigences
`EX-EDIT-005` (redimensionnement), `EX-EDIT-006`/`EX-EDIT-007` (enregistrement/validation),
`EX-EDIT-008` (essai immédiat), `EX-EDIT-017` (plafond de taille) — présentation Qt ; `EX-IHM-010`/
`EX-IHM-011`.
