"""Assemble la page de maquettes depuis les fragments de planche.

Chaque planche est dessinee a 1280 px de large (la largeur de la fenetre de jeu) ; la page
la remet a l'echelle du conteneur, de sorte que les proportions restent exactes.
"""

from pathlib import Path

HERE = Path(__file__).parent
CARET = (HERE / "_caret.frag").read_text(encoding="utf-8").strip()

TITLES = {
    "_body_main.frag": "Menu principal",
    "_body_variantes.frag": "Trois directions",
    "_body_pause.frag": "Pause",
    "_body_options.frag": "Options",
    "_body_select.frag": "Sélection de niveau",
    "_body_fin.frag": "Fin de niveau",
    "_body_credits.frag": "Crédits",
    "_body_ia.frag": "Mode IA",
    "_body_systeme.frag": "Système de design",
}

NOTES = {
    "_body_main.frag": (
        "Le cadre autour des entrées a sauté, remplacé par une vraie scène de fond : ciel en bandes, "
        "lune tracée en pixels entiers, trois plans de silhouettes et le sol au pas de 32 px du jeu. "
        "Un dégradé sombre côté gauche garde le texte lisible sans masquer le décor."
    ),
    "_body_variantes.frag": (
        "Trois directions ont été mises côte à côte, et c'est la A — Ambre nuit — qui est retenue : "
        "le bleu-nuit et l'ambre existants, rendus en pixel art. Les deux écartées restent affichées "
        "en retrait, pour garder trace de ce qui a été envisagé et de ce que ça coûtait."
    ),
    "_body_pause.frag": (
        "Le recouvrement garde la scène visible derrière, assombrie mais jamais floutée : le flou "
        "n'existe pas en pixel art. Le cadre passe en bordure accent pour signaler qu'on est dans "
        "une fenêtre modale, pas dans un écran à part entière. En haut à droite : le compteur "
        "d'images/s que vous avez demandé, dessiné par-dessus le voile pour rester lisible en pause."
    ),
    "_body_options.frag": (
        "L'écran le plus dense, et le vrai test de la direction : case à cocher, glissière, liste "
        "déroulante, bouton et onglets doivent tous exister en pixel art sans devenir illisibles. "
        "Les six onglets incluent les trois pages de remappage ajoutées en code. La ligne "
        "« Compteur d'images/s » remplace l'ancien sélecteur de limite : ce que le joueur veut voir, "
        "c'est le chiffre, pas un plafond."
    ),
    "_body_select.frag": (
        "L'état d'un niveau est porté par une pastille de couleur autant que par le suffixe texte : "
        "plein pour terminé, accent pour en cours, creux pour verrouillé. Un joueur daltonien garde "
        "le suffixe, un joueur pressé lit la pastille."
    ),
    "_body_fin.frag": (
        "Même patron de recouvrement que la Pause. Le titre porte le nom du tableau terminé, et le "
        "bilan tient en trois chiffres — vous les avez validés : au-delà, l'écran devient un tableau "
        "de bord alors qu'il doit se lire en une seconde avant d'enchaîner."
    ),
    "_body_credits.frag": (
        "Trois sections séparées par des filets, en-têtes en accent et petites capitales espacées. "
        "Les noms propres et les licences ne sont jamais traduits : le texte est celui de "
        "CreditsScreen.ui, mot pour mot."
    ),
    "_body_ia.frag": (
        "L'écran arrivé après la refonte d'identité, et le seul qui n'avait jamais eu de planche : "
        "livré avec la bonne palette et la bonne police, mais sans cadre et avec un focus signalé "
        "par la seule teinte. La planche pose les deux manques — cadre à bordure franche autour du "
        "contenu, curseur explicite à gauche du contrôle focalisé. C'est aussi l'écran le plus dense "
        "en contrôles numériques : il fallait vérifier qu'un tableau de chiffres tient en pixel art."
    ),
    "_body_systeme.frag": (
        "La planche de référence : c'est elle qui devient la nouvelle portée identité des jetons et "
        "la section identité de theme.qss. Les trois lignes de l'encart Échelle montrent le problème "
        "de départ et sa correction."
    ),
}

CHIPS = {
    "_body_main.frag": [
        ("Fond", "scène à trois plans, ajoutée"),
        ("Cadre", "retiré — les entrées flottent"),
        ("Lisibilité", "dégradé latéral, pas un voile plein"),
        ("Lune", "cercle en pixels, pas un border-radius"),
    ],
    "_body_variantes.frag": [
        ("Retenue", "A — ambre nuit"),
        ("Accent", "#ffd133, inchangé depuis le LOT-56"),
        ("Écartées", "B cyan cathodique, C néon arcade"),
    ],
    "_body_pause.frag": [
        ("Voile", "noir 72 %, sans flou"),
        ("Cadre", "variante accent = modale"),
        ("Compteur", "images/s en haut à droite"),
        ("Scène", "figée, lisible derrière"),
    ],
    "_body_options.frag": [
        ("Contrôles", "case, glissière, liste, bouton"),
        ("Onglets", "actif en aplat accent"),
        ("Ajouté", "compteur d'images/s, en haut à droite"),
        ("Retiré", "résolution et limite d'images/s"),
        ("Focus", "la ligne entière, pas le seul contrôle"),
    ],
    "_body_select.frag": [
        ("État", "pastille + suffixe, jamais l'un seul"),
        ("Verrouillé", "gris indisponible, pastille creuse"),
        ("Onglets", "Séquence / Niveaux personnels"),
    ],
    "_body_fin.frag": [
        ("Bilan", "trois chiffres, validé"),
        ("Impact", "temps et morts restent à compter"),
        ("Drapeau", "vert réussite, hors palette d'accent"),
    ],
    "_body_credits.frag": [
        ("Hiérarchie", "en-tête accent / nom / licence"),
        ("Texte", "repris tel quel, rien d'inventé"),
    ],
    "_body_ia.frag": [
        ("Cadre", "ajouté — l'écran livré n'en portait aucun"),
        ("Focus", "curseur explicite, pas la seule teinte"),
        ("Tableau", "générations, lignes alternées"),
        ("Libellés", "repris de fr.lang, mot pour mot"),
    ],
    "_body_systeme.frag": [
        ("Palette", "12 rôles, aucun dégradé"),
        ("Échelle", "comparaison ×1 / ×2 / ×3"),
        ("Cadre", "anatomie des 9 tranches"),
        ("États", "repos / focus / indisponible"),
    ],
}

ORDER = [
    ("_body_main.frag", "1280 / 720"),
    ("_body_variantes.frag", "1280 / 840"),
    ("_body_pause.frag", "1280 / 720"),
    ("_body_options.frag", "1280 / 720"),
    ("_body_select.frag", "1280 / 720"),
    ("_body_fin.frag", "1280 / 720"),
    ("_body_credits.frag", "1280 / 720"),
    ("_body_ia.frag", "1280 / 720"),
    ("_body_systeme.frag", "1280 / 1180"),
]


def board_markup(fragment: str) -> str:
    """Rend un fragment de planche prêt pour la page : curseur résolu, accent figé."""
    source = (HERE / fragment).read_text(encoding="utf-8")
    return source.replace("@CARET@", CARET).replace("{{accent}}", "#ffd133")


def main() -> None:
    parts = [(HERE / "_page_head.html").read_text(encoding="utf-8")]

    for index, (fragment, ratio) in enumerate(ORDER, start=1):
        chips = "".join(
            f'<span class="chip"><b>{label}</b> {text}</span>'
            for label, text in CHIPS[fragment]
        )
        parts.append(
            f'\n<section>\n'
            f'  <div class="board-head">'
            f'<span class="board-num">{index:02d}</span>'
            f'<h2>{TITLES[fragment]}</h2></div>\n'
            f'  <p class="board-note">{NOTES[fragment]}</p>\n'
            f'  <div class="stage" style="--ar: {ratio};">\n'
            f'{board_markup(fragment)}\n'
            f'  </div>\n'
            f'  <div class="look-for">{chips}</div>\n'
            f'</section>\n'
        )

    parts.append((HERE / "_page_tail.html").read_text(encoding="utf-8"))

    out = HERE / "ecrans-jeu-parchemin.html"
    out.write_text("".join(parts), encoding="utf-8")
    print(f"{out.name} : {out.stat().st_size} octets")


if __name__ == "__main__":
    main()
