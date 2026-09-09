pragma Singleton
import QtQuick

/*!
    Jetons de design des ecrans du jeu -- la SOURCE UNIQUE de l'apparence (LOT-86).

    Ce fichier est ecrit a la main et appartient a la conception. Il n'est engendre par rien, et
    rien ne le recopie : c'est ce qui permet d'en changer une valeur sans compiler une ligne de
    C++. Le chassis d'edition a son propre jeu, en C++ chez l'editeur -- deux langages, deux
    binaires, aucun chemin entre eux : l'etancheite des deux portees est structurelle.

    Les couleurs sont nommees par ROLE, jamais par teinte. Un jeton `accent` survit a un changement
    de couleur ; un jeton qui s'appellerait `or` deviendrait un mensonge le jour ou l'accent passe
    au bleu.
*/
QtObject {
    id: tokens

    // --- Facteur d'agrandissement -------------------------------------------------------------
    //
    // ENTIER, borne a [1, 3]. Fractionnaire, il corromprait silencieusement les filets d'un pixel
    // du cadre de parchemin -- c'est la raison pour laquelle il ne sera jamais un reel.
    //
    // L'application ecrit cette valeur au demarrage, calculee depuis la hauteur de la fenetre. La
    // valeur par defaut 2 est celle de la CONCEPTION : c'est ce que Qt Design Studio affiche, et
    // c'est le facteur d'une fenetre 720p.
    property int scale: 2

    // --- Couleurs : parchemin de Tanares (LOT-66, LOT-76, EX-IHM-070) --------------------------
    //
    // Chaque teinte est RELEVEE sur les feuilles de personnage de la source -- histogramme
    // quantifie des pages rendues -- et non choisie a vue. Une couleur inventee ressemble a la
    // source sans en venir, et rien ne le dit jamais.
    readonly property color background: "#d0c0a0"   // parchemin vieilli
    readonly property color surface: "#e0d0b0"      // champ de la feuille
    readonly property color surfaceAlt: "#f0e0d0"   // encadre clair
    readonly property color border: "#907030"       // brun dore
    readonly property color text: "#302000"         // encre sepia
    readonly property color textMuted: "#705020"    // encre delavee
    readonly property color accent: "#c0a060"       // or des filets
    readonly property color accentHover: "#c0b080"

    // Seul role qui ne vienne pas des feuilles : une feuille de personnage n'a pas d'etat
    // d'erreur a montrer. Rouge de garance assombri, tenant le contraste sur le parchemin --
    // signale ici comme non atteste plutot que passe sous silence.
    readonly property color error: "#8a2f20"

    // Cadre : un trait exterieur d'encre, un filet ornemental dore en retrait, et l'ombre portee.
    // Ce ne sont pas des biseaux -- la lumiere ne vient pas d'en haut a gauche, il n'y a pas de
    // relief a simuler. Ce qui doit rester lisible, c'est l'ECART entre le trait et le filet :
    // deux traits de meme valeur ne composent pas un encadrement, mais une bordure epaisse.
    readonly property color frameEdge: "#302000"
    readonly property color frameOrnament: "#907030"
    readonly property color frameShadow: "#705020"

    // Grenat des cabochons et de la plaque du bandeau de titre (LOT-76), releve sur deux angles
    // opposes du meme cabochon -- qui donnent la meme valeur.
    readonly property color gem: "#701010"
    readonly property color gemShadow: "#400000"

    // --- Typographie ---------------------------------------------------------------------------
    //
    // Les familles sont nommees, pas chargees ici : l'application enregistre les TTF au demarrage
    // et Qt Design Studio les prend dans `FontFiles` du .qmlproject. Les deux voient donc les
    // memes noms. Changer de police se fait ICI, sans toucher au C++.
    readonly property string bodyFamily: "Pixelify Sans"

    // Titres d'ecran UNIQUEMENT : trop typee pour du corps de texte.
    readonly property string titleFamily: "Press Start 2P"

    // --- Grandeurs de l'identite, en PIXELS a l'echelle courante --------------------------------
    //
    // En pixels et non en points : le pixel art se dimensionne en pixels, et un point vaut une
    // fraction variable de pixel selon la definition de l'ecran -- le facteur entier n'aurait
    // alors plus rien d'entier.
    //
    // Deja multipliees par `scale`, pour que la conception n'ait jamais a le faire : ecrire
    // `font.pixelSize: Tokens.screenTitle` suffit. C'est aussi ce qui empeche d'oublier le
    // facteur sur un ecran et de le voir diverger des autres.
    readonly property int screenTitle: 23 * scale
    readonly property int sectionTitle: 14 * scale
    readonly property int body: 10 * scale
    readonly property int caption: 8 * scale

    readonly property int spaceSmall: 4 * scale
    readonly property int spaceMedium: 6 * scale
    readonly property int spaceLarge: 8 * scale
    readonly property int spaceExtraLarge: 12 * scale

    // Trait et filet d'un encadrement : une unite de maquette chacun.
    readonly property int frameThickness: 1 * scale
}
