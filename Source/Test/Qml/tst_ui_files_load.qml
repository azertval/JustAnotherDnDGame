import QtQuick
import QtTest
import Jadg.Ui

/*!
    Chaque fichier `.ui.qml` du module Jadg.Ui se construit, sans un seul avertissement.

    qmllint lit un fichier ; il ne l'instancie pas. Une liaison qui lit la propriete d'un objet nul,
    un id mal orthographie dans une expression, une piece du cahier designee par une cle inconnue ne
    se voient qu'a l'execution -- dans le journal du joueur. La liste vient de la ressource du module
    (QmlTestSetup.cpp) : un ecran ajoute est teste sans etre declare ici.
*/
TestCase {
    id: testCase

    name: "UiFilesLoad"
    width: 1920
    height: 1080
    visible: true
    when: windowShown

    function test_la_liste_couvre_le_module() {
        verify(jadgUiFiles.length >= 40, "seulement " + jadgUiFiles.length + " fichier(s) lu(s)")
        verify(jadgUiFiles.indexOf("Controls/OrnateButton") >= 0, "Controls/OrnateButton absent")
        verify(jadgUiFiles.indexOf("Screens/MainMenuForm") >= 0, "Screens/MainMenuForm absent")
    }

    function test_construction_data() {
        return jadgUiFiles.map(function (file) {
            return { tag: file, type: file.substring(file.lastIndexOf("/") + 1) }
        })
    }

    function test_construction(data) {
        // Tout avertissement QML pendant la construction fait echouer le test, et le cite.
        failOnWarning(/.*/)
        const item = createTemporaryQmlObject("import QtQuick\nimport Jadg.Ui\n" + data.type
                                              + " { anchors.fill: parent }", testCase, data.tag)
        verify(item !== null, data.tag + " ne se construit pas")
        // Deux images rendues : les liaisons evaluees au premier affichage le sont aussi.
        waitForRendering(item)
        wait(50)
    }
}
