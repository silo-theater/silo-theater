/*
 *  Copyright (C) 2026 Silo-theater contributors
 *  SPDX-License-Identifier: AGPL-3.0-or-later
 *  See LICENSE for more information.
 */

import QtQuick

Window {
    // Full screen on the box, where EGLFS gives one window the size of the
    // HDMI mode; a normal window on a desktop.
    width: 1280
    height: 720
    visible: true
    title: qsTr("Silo-theater")
    color: "#101014"

    Text {
        anchors.centerIn: parent
        text: qsTr("Silo-theater")
        color: "#f0f0f0"
        font.pixelSize: parent.height / 12
    }
}
