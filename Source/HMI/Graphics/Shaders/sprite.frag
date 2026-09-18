// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

// Fragment shader du pipeline 2D : échantillonne la texture liée et la multiplie
// par la teinte du sommet. L'échantillonnage est *nearest* -- fixé par le `QRhiSampler` côté C++,
// pas ici : c'est l'invariant de netteté du pixel art (`EX-ARCH-022`).
//
// L'alpha n'est **pas** prémultiplié (`hmi::DecodedImage`), en accord avec le mélange
// SrcAlpha / OneMinusSrcAlpha configuré sur le pipeline.
#version 440

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D uTexture;

void main() {
    fragColor = texture(uTexture, vUv) * vColor;
}
