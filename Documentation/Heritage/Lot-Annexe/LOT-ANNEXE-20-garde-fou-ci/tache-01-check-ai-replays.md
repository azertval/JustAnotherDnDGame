# TACHE-01 — scripts/check_ai_replays.py {#lot-annexe-20-tache-01-check-ai-replays}

**Lot :** [LOT-ANNEXE-20](epic.md) · **Emplacement :** `scripts` · **Statut :** fait

## Contexte
`LOT-ANNEXE-17` valide un rejeu à la lecture, en C++, au moment où le jeu (ou la CLI) le charge —
mais rien ne le vérifie systématiquement en amont, avant qu'un commit modifiant un niveau
n'atteigne `main`. Cette tâche porte la même vérification (empreinte de niveau) dans un script
Python autonome, sur le modèle exact de `scripts/check_demo_sequence.py` déjà présent dans le dépôt.

## Travail à réaliser
- **`scripts/check_ai_replays.py`** : structure calquée sur `check_demo_sequence.py` — imports
  standard uniquement (`os`, `json`, `sys`), fonction `fnv1a_64(data: bytes) -> int` (réimplémentation
  manuelle de l'algorithme, **même convention d'octets** que `aisolver::computeLevelFingerprint` :
  lecture du fichier de niveau en mode binaire, aucune normalisation de fin de ligne ni
  d'encodage au-delà d'UTF-8 brut), fonction `check(root)` qui :
  1. énumère les fichiers de rejeu sous le dossier de rejeux publiés (`Source/Elements/Replays/`,
     ou le chemin retenu par `LOT-ANNEXE-18` TACHE-02) ;
  2. pour chacun, lit son JSON, en extrait `levelPath` et `levelFingerprint` ;
  3. résout `levelPath` sous `Source/Elements/Levels/`, vérifie son existence ;
  4. si absent, ajoute une erreur « niveau introuvable » ; sinon, calcule `fnv1a_64` sur son contenu
     brut et la compare à `levelFingerprint` — divergence ajoutée comme erreur « empreinte
     périmée », avec le nom du rejeu et du niveau concernés ;
  5. retourne un code de sortie `0` si aucune erreur, `1` sinon, avec un résumé imprimé (nombre de
     rejeux vérifiés, liste des problèmes) — même style de sortie que `check_demo_sequence.py`.
- `main()` minimal, `if __name__ == '__main__': sys.exit(main())`, résolution de `root` par rapport
  à l'emplacement du script (`os.path.dirname(os.path.dirname(os.path.abspath(__file__)))`), comme
  `check_demo_sequence.py`.

## Fichiers impactés
- `scripts/check_ai_replays.py` — nouveau.

## Tests (obligatoires)
- **Empreinte identique à l'implémentation C++** : sur un fichier de niveau fixe committé (ex. un
  niveau `demo-*.json` existant), `fnv1a_64` appliqué par ce script produit la **même** valeur
  numérique que `aisolver::computeLevelFingerprint` (`LOT-ANNEXE-17`) appliqué au même fichier —
  vérifié par une valeur de référence documentée dans les deux implémentations (constante partagée
  en commentaire, ou fixture de test commune).
- **Rejeu valide** : `check(root)` retourne `0` sur un jeu de rejeux dont les empreintes
  correspondent à leurs niveaux actuels.
- **Rejeu périmé (empreinte divergente)** : `check(root)` retourne `1`, message identifiant le
  rejeu et le niveau concernés.
- **Rejeu orphelin (niveau introuvable)** : `check(root)` retourne `1`, message distinct du cas
  précédent.
- **Absence de tout rejeu publié** : `check(root)` retourne `0` avec un résumé indiquant zéro rejeu
  vérifié (cas non bloquant, pas une erreur — le dossier peut légitimement être vide avant que
  `LOT-ANNEXE-18` ne publie son premier rejeu de démonstration).

## Points d'attention
- **La convention d'octets doit rester identique entre les deux implémentations** (C++ et
  Python) : lecture en mode binaire strict, aucune conversion de fin de ligne (`\r\n` vs `\n`)
  implicite lors de la lecture du fichier — un piège classique de portabilité Windows/Unix,
  documenté explicitement ici parce que `LOT-ANNEXE-17` l'a identifié comme risque de divergence.
- **Ce script ne construit ni n'exécute aucun binaire C++** : il lit uniquement des fichiers texte
  (JSON de rejeu, JSON de niveau) — condition pour qu'il reste exécutable dans un job CI léger, sans
  étape de build préalable.

## Définition de fait (DoD)
- `check_ai_replays.py` fonctionnel, testé manuellement sur les cas listés ; valeur d'empreinte
  vérifiée identique à l'implémentation C++ sur au moins un fichier de référence commun.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-021` (nouvelle, partagée avec TACHE-02 du même lot).
