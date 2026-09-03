# TACHE-06 — Thème clair et sombre de l'éditeur {#lot-56-tache-06-theme-clair-sombre}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/Elements/Themes` · **Statut :** fait

## Contexte
L'habillage du projet est sombre, sans alternative ni réglage, et rien dans `Source/HMI` n'interroge le
réglage clair/sombre du système. Pour le **jeu**, c'est normal : le fond sombre et l'accent ambre font
partie de son apparence, au même titre que ses tuiles. Pour l'**éditeur**, c'est un défaut : c'est un
outil de production, utilisé de jour, sur de longues sessions, souvent à côté d'autres applications.

Cette tâche traite donc **le seul châssis d'édition**. Le menu principal, l'écran Options et le jeu
conservent l'identité sombre en toute circonstance — cette frontière n'est pas introduite ici : elle
est portée par les jetons depuis la TACHE-01 et par la feuille de style depuis la TACHE-02. Il ne reste
qu'à fournir un second jeu de valeurs pour la portée variable, et à le choisir.

C'est ce qui fait de cette tâche la **première à retirer** si le lot doit être resserré : elle emporte
alors `EX-IHM-054` avec elle, sans rien invalider — la séparation des portées reste acquise, et le
thème clair pourra être ajouté plus tard sans rien reprendre.

## Travail à réaliser
- **Second jeu de valeurs claires** pour la portée **variable** uniquement, couvrant exactement les
  mêmes rôles que le jeu sombre (la symétrie est déjà vérifiée par un test de la TACHE-01). Les jetons
  **invariants** ne sont pas touchés.
- **Détection du réglage système** au démarrage et **réaction à son changement** en cours d'exécution.
- **Application à chaud** : changer de thème régénère la feuille de style et la palette de la portée
  variable, invalide les icônes dessinées (TACHE-04) et les vignettes (TACHE-05), sans redémarrage et
  sans reconstruire les widgets.
- **Réglage dans le menu Affichage de l'éditeur** — Système, Clair, Sombre — persisté entre deux
  sessions. Il vit auprès des bascules de panneaux et de vue, et **non** dans l'écran Options : c'est
  un réglage de l'outil, pas du jeu, et l'écran Options est justement l'un des écrans qu'il n'affecte
  pas.
- **Couleur d'effacement du viewport** suivant le thème actif **en mode édition**, et restant sur le
  jeton invariant en mode jeu et en essai (règle posée en TACHE-01).
- **Traduction** des libellés du réglage dans les deux catalogues.

## Fichiers impactés
- `Source/HMI/Interface/DesignTokens.h` — jeu de valeurs claires de la portée variable.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` — détection, résolution et application à chaud.
- `Source/HMI/Interface/MainWindow.{h,cpp}`, `Source/Elements/UI/MainWindow.ui` — entrée du menu
  Affichage.
- `Source/HMI/Game/GameViewport.cpp` — couleur d'effacement suivant le thème actif en édition.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Interface/test_application_theme.cpp` — complété.

## Tests (obligatoires)
- **Résolution du thème effectif** : fonction **pure** prenant le réglage utilisateur et le réglage
  système, produisant le thème à appliquer — Système + système clair → clair ; Système + système
  sombre → sombre ; Clair ou Sombre forcé → ignore le système. Testée sans instance d'application.
- **Étanchéité de l'identité** : quel que soit le thème d'éditeur actif, les jetons invariants et les
  règles de style du menu principal et de la page Options sont **inchangés au caractère près**. C'est
  le test qui exprime l'exigence de cette tâche, et il complète celui de la TACHE-02 en couvrant cette
  fois la bascule réelle.
- **Couleur d'effacement selon le mode** : en édition elle suit le thème actif, en jeu et en essai elle
  reste sur le jeton invariant — y compris après une bascule de thème effectuée avant de lancer
  l'essai.
- **Contraste** : pour chaque thème d'éditeur, le rapport de contraste entre le texte et son fond, et
  entre le texte atténué et son fond, dépasse un seuil de lisibilité fixé — garde-fou contre un thème
  clair livré illisible.
- **Persistance** : le réglage est relu au démarrage suivant.

## Points d'attention
- **Un thème clair n'est pas le thème sombre aux couleurs inversées.** Les bordures, les ombres et les
  lignes alternées demandent des écarts de luminosité différents en clair et en sombre ; inverser
  mécaniquement produit un thème plat. Le test de contraste borne le défaut, il ne le remplace pas.
- **Le passage éditeur → jeu doit être franc.** L'utilisateur travaillant en thème clair verra le jeu
  rester sombre : c'est voulu, mais la transition doit être nette et non un dégradé d'états
  intermédiaires. Vérifier l'aller-retour éditeur → essai → éditeur, et le retour au menu principal.
- **Ne pas oublier les icônes et les vignettes** lors du changement à chaud : dessinées depuis les
  jetons, elles restent aux anciennes couleurs si elles ne sont pas régénérées.
- Les boîtes de dialogue ouvertes **depuis l'éditeur** appartiennent au châssis d'édition et suivent son
  thème ; celles ouvertes depuis le menu ou les Options suivent l'identité. Trancher explicitement les
  cas partagés plutôt que de les laisser dépendre du parent de la fenêtre.
- Le contenu du jeu — tuiles, décors, fonds, personnage — ne change **jamais** avec le thème, y compris
  dans le canevas d'édition : seul l'habillage est concerné. Une tuile ne doit pas changer d'apparence
  parce que l'éditeur est passé en clair.
- Vérifier le rendu du viewport pendant la bascule : la couleur d'effacement change à la frame
  suivante, ce qui ne doit produire aucun clignotement visible.

## Définition de fait (DoD)
- L'éditeur suit le réglage clair/sombre du système par défaut, accepte un forçage depuis son menu
  Affichage, persiste ce choix et l'applique sans redémarrage ; le menu principal, l'écran Options et
  le jeu sont inchangés quel que soit le thème actif, ce qu'un test garantit ; la couleur d'effacement
  du viewport suit le thème en édition et l'identité en jeu ; les deux jeux de valeurs satisfont le
  seuil de contraste ; icônes et vignettes suivent le thème actif ; `/W4 /WX` propre.

## Exigences
`EX-IHM-054` (thème clair/sombre de l'éditeur, réglable et persisté, sans effet sur l'identité du jeu) ;
réutilise `EX-IHM-050` (deux portées d'habillage), `EX-IHM-051` (source unique des grandeurs),
`EX-REN-033` (traduction).
