# TACHE-02 — Documentation des résultats observés {#lot-annexe-16-tache-02-documentation-resultats}

**Lot :** [LOT-ANNEXE-16](epic.md) · **Emplacement :** `Documentation/Lot-Annexe` · **Statut :** fait

## Contexte
TACHE-01 produit des chiffres (taux de réussite croisé par paire de niveaux) ; cette tâche les met
par écrit, dans le même esprit que la note d'usage de `LOT-ANNEXE-11` (TACHE-04) — un document, pas
du code — pour qu'ils restent consultables sans avoir à relancer la campagne d'exécution croisée.

## Travail à réaliser
- **`Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/resultats-transfert.md`** :
  document présentant, pour chaque paire (A, B) exécutée par TACHE-01 : les mécaniques partagées
  et différentes entre les deux niveaux (lecture humaine, pas générée automatiquement), le taux de
  réussite croisé mesuré, une mise en regard avec le taux de réussite sur niveau d'origine
  (`LOT-ANNEXE-15`) pour le même modèle.
- **Section d'attentes préalables**, rédigée **avant** toute mesure réelle (au moment de
  l'exécution effective de ce lot, pas de ce cadrage) : rappelle explicitement que l'entraînement
  reste niveau par niveau et qu'un transfert faible est attendu, pour qu'un résultat faible ne soit
  pas lu après coup comme un échec du programme.
- Lien depuis `Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/epic.md` (section
  `## Résultats`, ajoutée à l'implémentation, pointant vers ce document).

## Fichiers impactés
- `Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/resultats-transfert.md` — nouveau.
- `Documentation/Lot-Annexe/LOT-ANNEXE-16-evaluation-hors-niveau/epic.md` — modifié (section
  `## Résultats` ajoutée une fois la mesure réalisée).

## Tests (obligatoires)
- Sans objet au sens `ctest` (livrable documentaire) — vérification par relecture : chaque paire
  (A, B) exécutée par TACHE-01 a bien une entrée correspondante dans le document, aucun chiffre
  rapporté sans sa paire de niveaux associée.

## Points d'attention
- **Ce document est écrit après exécution réelle de la campagne** (TACHE-01), pas anticipé — au
  moment de ce cadrage, aucune mesure n'existe encore ; cette tâche ne fait que réserver
  l'emplacement et la structure attendue du document.
- **Aucune affirmation comparative sur la qualité d'un algorithme** (« l'algorithme X généralise
  mieux que Y ») ne doit être tirée d'un transfert croisé mesuré sur un régime d'entraînement qui
  n'a jamais visé la généralisation — la décision de cadrage de l'épic s'applique directement à la
  rédaction de ce document.

## Définition de fait (DoD)
- `resultats-transfert.md` rédigé et lié depuis l'`epic.md` du lot une fois la campagne de TACHE-01
  exécutée ; Doxygen à jour (nouveau fichier intégré à la navigation si nécessaire).

## Notions abordées
@ref guide-annexe-evaluation-rl — généralisation hors du niveau d'entraînement, sur-apprentissage à
un seul environnement.

## Exigences
`EX-IA-017` (nouvelle, du même lot).
