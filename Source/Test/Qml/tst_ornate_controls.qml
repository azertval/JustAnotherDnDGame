import QtQuick
import QtTest
import Jadg.Ui

/*!
    Comportement des briques de la charte v2 que les ecrans et la galerie supposent (LOT-87, T2.7).

    L'etat visuel d'un bouton ou d'une case se DEDUIT de ce que le controle sait deja (enabled,
    down, hovered, highlighted, checked), et `forcedState` l'impose. La piece du cahier se nomme a
    partir de cet etat : une deduction fausse montre la mauvaise image sans aucune erreur.
*/
TestCase {
    id: testCase

    name: "OrnateControls"
    width: 800
    height: 600
    visible: true
    when: windowShown

    Component {
        id: buttonComponent

        OrnateButton {
            text: "Nouvelle partie"
        }
    }

    Component {
        id: checkComponent

        OrnateCheck {
            text: "Synchronisation verticale"
        }
    }

    function cleanup() {
        Tokens.uiScale = 1
    }

    function test_bouton_etat_deduit_data() {
        return [
            { tag: "normal", kind: "primary", enabled: true, highlighted: false, state: "normal" },
            { tag: "desactive", kind: "primary", enabled: false, highlighted: true, state: "disabled" },
            { tag: "focus-hors-menu", kind: "primary", enabled: true, highlighted: true, state: "hover" },
            { tag: "entree-courante-menu", kind: "menu", enabled: true, highlighted: true, state: "active" },
            { tag: "menu-desactive", kind: "menu", enabled: false, highlighted: true, state: "disabled" },
        ]
    }

    function test_bouton_etat_deduit(data) {
        const button = createTemporaryObject(buttonComponent, testCase,
                                             { kind: data.kind, enabled: data.enabled,
                                               highlighted: data.highlighted })
        compare(button.visualState, data.state)
        compare(button.key, "ui/button/" + data.kind + "/" + data.state)
    }

    function test_bouton_etat_impose_l_emporte() {
        const button = createTemporaryObject(buttonComponent, testCase,
                                             { enabled: false, forcedState: "pressed" })
        compare(button.visualState, "pressed")
        verify(button.lit)
        verify(!button.disabledLook)
    }

    function test_bouton_enfonce() {
        const button = createTemporaryObject(buttonComponent, testCase, { kind: "apply" })
        const clicked = createTemporaryObject(signalSpyComponent, testCase,
                                              { target: button, signalName: "clicked" })
        mousePress(button)
        compare(button.visualState, "pressed")
        mouseRelease(button)
        compare(clicked.count, 1)
    }

    function test_bouton_taille_suit_l_echelle_data() {
        return [
            { tag: "menu", kind: "menu", width: 480, height: 72 },
            { tag: "retour", kind: "back", width: 320, height: 72 },
            { tag: "action", kind: "primary", width: 280, height: 56 },
        ]
    }

    function test_bouton_taille_suit_l_echelle(data) {
        const button = createTemporaryObject(buttonComponent, testCase, { kind: data.kind })
        compare(button.implicitWidth, data.width)
        compare(button.implicitHeight, data.height)
        Tokens.uiScale = 1.5
        compare(button.implicitWidth, data.width * 1.5)
        compare(button.implicitHeight, data.height * 1.5)
    }

    function test_case_se_coche_au_clic_et_au_clavier() {
        const check = createTemporaryObject(checkComponent, testCase)
        compare(check.visualState, "unchecked")
        mouseClick(check)
        verify(check.checked)
        compare(check.key, "ui/control/checkbox/checked")
        check.forceActiveFocus()
        keyClick(Qt.Key_Space)
        verify(!check.checked)
        compare(check.visualState, "unchecked")
    }

    function test_case_desactivee() {
        const check = createTemporaryObject(checkComponent, testCase, { checked: true, enabled: false })
        compare(check.visualState, "disabled")
        mouseClick(check)
        verify(check.checked, "une case désactivée ne doit pas changer au clic")
    }

    Component {
        id: signalSpyComponent

        SignalSpy {}
    }
}
