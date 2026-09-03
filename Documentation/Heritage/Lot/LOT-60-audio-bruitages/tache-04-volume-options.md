# TACHE-04 — Réglage de volume persisté {#lot-60-tache-04-volume-options}

**Lot :** [LOT-60](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
Un jeu qui se met à faire du bruit sans offrir de le baisser est plus pénible que muet.
`hmi::OptionsPage` gère déjà deux réglages persistés — V-Sync et langue — plus l'état de la manette
et trois écrans de remappage : le réglage de volume s'y ajoute selon un patron établi, sans rien
inventer.

## Travail à réaliser
- **Curseur de volume global** dans `hmi::OptionsPage`, avec sa valeur lisible et un libellé
  traduit (`EX-REN-033`).
- **Effet immédiat** : déplacer le curseur change le volume sans redémarrage ni validation.
- **Retour sonore au réglage** : jouer un échantillon court au relâchement du curseur — sans quoi
  on règle un volume à l'aveugle. Au relâchement, pas à chaque pas du curseur.
- **Persistance** au même endroit et selon le même mécanisme que les réglages existants de la page ;
  ne pas ouvrir un troisième emplacement de configuration.
- **Muet** atteignable proprement (valeur zéro), et retrouvé au lancement suivant.
- **Navigation clavier / souris / manette**, comme tout le reste de la page.

## Fichiers impactés
- `Source/HMI/Interface/OptionsPage.{h,cpp}`, `Source/Elements/UI/OptionsPage.ui`.
- `Source/HMI/Audio/AudioEngine.{h,cpp}` — application du volume.
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/Test/Unit/HMI/Audio/test_audio_engine.cpp` (étendu).

## Tests (obligatoires)
- Le volume est **borné** : les valeurs hors plage sont ramenées aux extrêmes, jamais propagées.
- Aller-retour de persistance : réglé, écrit, relu à l'identique.
- Réglage **absent** du fichier de configuration → valeur par défaut, sans erreur.
- Volume à zéro : aucune lecture n'est tentée, et rien ne plante.
- Libellés présents dans les **deux** catalogues de traduction.

## Points d'attention
- **Ne pas jouer l'échantillon de test à chaque pas du curseur** : au relâchement uniquement, sinon
  le réglage devient une mitraillette.
- Le volume doit s'appliquer à l'échantillon de test lui-même, faute de quoi il ne renseigne sur
  rien.
- La page d'options est atteignable **depuis le menu principal et depuis la pause**
  (`LOT-59`) : vérifier que le retour revient au bon écran dans les deux cas — c'est la table de
  transitions de `LOT-59` TACHE-01 qui le porte, pas une variable posée ici.
- Vérifier le rendu du curseur dans les thèmes **clair et sombre** (`LOT-56`) : la feuille de style
  du projet couvre l'IHM, un nouveau type de widget peut y être mal servi.

## État
Un `QSlider` (`volumeSlider`, onglet Audio, remplace le placeholder « à venir ») règle le volume
global : effet immédiat (`valueChanged`), persistance dans la **même portée `QSettings`** que la
langue et le mode de rendu (clé `volume`, relevée au lancement puisque `OptionsPage` est construite
au démarrage — le réglage s'applique même si le joueur n'ouvre jamais les options). Retour sonore
au **relâchement** du curseur (`sliderReleased`), jamais à chaque pas.

**Écart avec le texte original de cette tâche** : celui-ci indiquait persister « au même endroit et
selon le même mécanisme que les réglages existants de la page », en citant V-Sync comme référence.
Vérification faite, **V-Sync ne persiste pas réellement** aujourd'hui (`GameViewport::_vsync`
redémarre toujours à `true`) — seuls la langue et le mode de rendu utilisent `QSettings`. Le volume
suit donc ce second mécanisme, réellement éprouvé, plutôt que V-Sync.

**Non couvert par un test automatisé** : l'aller-retour de persistance `QSettings` et le repli sur
la valeur par défaut si absente. Aucun réglage `QSettings` existant du projet (langue, mode de
rendu) n'est testé de cette façon — c'est la convention établie, vérifiée manuellement (`TACHE-05`).
Le bornage du volume reste couvert par les tests unitaires de `AudioEngine` (`TACHE-01`).

## Définition de fait (DoD)
- Le volume se règle dans les options avec effet immédiat et retour sonore au relâchement, est borné,
  persiste entre deux lancements, se comporte correctement à zéro et depuis les deux chemins d'accès
  à la page, et est traduit ; `/W4 /WX` propre.

## Exigences
`EX-REN-048` (volume réglable et persisté) ; réutilise `EX-IHM-001` (interface hors-jeu en Qt),
`EX-REN-033` (traduction), `EX-CTRL-012` (clavier et manette), `EX-NFR-040` (erreur récupérable),
`EX-IHM-050` à `EX-IHM-055` (système de design, thèmes).
