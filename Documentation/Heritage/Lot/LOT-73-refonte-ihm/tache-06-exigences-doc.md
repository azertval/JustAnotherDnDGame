# TACHE-06 — Exigences, localisation, documentation {#lot-73-tache-06-exigences-doc}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Documentation`, `Source/Elements/Localization` ·
**Statut :** fait

## Contexte
Les trois défauts traités par ce lot avaient un point commun : **aucune exigence ne les
interdisait**. La spécification IHM disait comment l'interface devait être construite, habillée et
répartie, mais jamais **qui décide de la taille de la fenêtre**, ni **à quelle portée un changement
d'habillage doit coûter**, ni qu'**un réglage exposé doit agir**. Sans énoncé, un correctif reste
local et le défaut revient — il était revenu deux fois.

Second manque, du même ordre : rien ne vérifiait que les deux catalogues de traduction déclarent les
mêmes clés. Une clé ajoutée d'un seul côté ne casse rien de visible — `Localization::text` replie
sur la langue par défaut et l'interface s'affiche, en français au milieu d'un écran anglais. Le
défaut ne se voit qu'en changeant de langue, écran par écran, ce que personne ne fait après avoir
ajouté un libellé.

## Travail réalisé
- **Section 9 de la spécification IHM** (`Documentation/Specification/interface-ihm.md`), quatre
  exigences nouvelles : [`EX-IHM-080`](@ref EX-IHM-080) (aucun écran ne contraint la fenêtre),
  [`EX-IHM-081`](@ref EX-IHM-081) (facteur et géométrie bornés par l'écran),
  [`EX-IHM-082`](@ref EX-IHM-082) (une portée ne repolit pas plus large qu'elle),
  [`EX-IHM-083`](@ref EX-IHM-083) (tout réglage exposé atteint le moteur). Traçabilité mise à jour.
- **Clés de traduction** : `ai_mode.chart_moving_average` et `ai_mode.preview_tip_waiting`, dans les
  deux catalogues.
- **`LocalizationTest.LesDeuxCataloguesDeclarentLesMemesCles`** : garde-fou de parité, **dans les
  deux sens** — une clé anglaise oubliée en français compte autant. Porté sur les fichiers
  réellement livrés, jamais sur une copie de test qui pourrait en diverger.
- **Documentation de code** : `guide-ihm-qt.md` (invariant de taille, portées de thème),
  `guide-design-ihm.md` (deux feuilles, habillage hybride du Mode IA), `README.md` de
  `Elements/Themes` et `Elements/UI`, `CHANGELOG.md`.
- **Cahier de test** régénéré depuis les blocs `\castest{...}` par
  `scripts/generate_cahier_test.py` — jamais édité à la main.

## Constat consigné, hors périmètre : la latence du jeu en configuration Debug
L'exploration menée pour ce lot a relevé une **seconde** famille de latence, distincte du gel
d'interface traité en `TACHE-02` et non retenue dans le périmètre. Elle est consignée ici pour ne
pas être redécouverte une troisième fois :

- **Une trace émise à chaque frame dans le chemin de rendu.** `SpriteRenderer::render` appelle
  `logStatisticsIfChanged()` à chaque frame ; sa garde compare des compteurs dont l'un
  (`culled`) est recalculé par le culling à chaque mouvement de caméra — elle est donc inopérante
  dès que le joueur bouge. Le niveau minimal par défaut étant `Trace`, un enregistrement part par
  frame, vers trois puits **synchrones** dont `OutputDebugStringA` (notification inter-processus
  quand un débogueur est attaché) et un `std::endl` par ligne (vidage disque). Le guide de
  journalisation interdit explicitement toute trace dans un chemin par frame.
- **`MemoryLogSink` croît sans borne** : un `std::vector<std::string>` jamais purgé, une entrée par
  frame pour toute la session.
- **Aucun préréglage `RelWithDebInfo`** dans `CMakePresets.json` : déboguer impose `/Od /Ob0 /RTC1`
  et les itérateurs vérifiés, ce qui frappe le plus durement un ECS à gabarits et un chemin de
  composition par quad.

Vérification en une commande, sans rien modifier : lancer avec `--log-level=info`. Si la latence
disparaît, les deux premiers points sont confirmés.
