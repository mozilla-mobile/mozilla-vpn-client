#version 310 es // qsb throws an error if the version is not
                // defined in the first line of the file
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifdef GL_ES
precision lowp int;
precision lowp float;
#endif

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObject {
    float animationProgress;
} ubo;

float drawCircle(float distance, float radius) {
    float antialias = 0.005;

    return smoothstep(radius, radius - antialias, distance);
}

float composeRing(float distance, float strokeWidth, float radius) {
    float circle1 = drawCircle(distance, radius);
    float circle2 = drawCircle(distance, radius - strokeWidth);

    return circle1 - circle2;
}

float calcRingRadius(float minRadius, float maxRadius, float currentRadius, float offset) {
    return mod(maxRadius * offset + currentRadius, maxRadius) + minRadius;
}

void main() {
    // Center coordinate
    vec2 center = vec2(0.5, 0.275);
    float centerDistance = length(qt_TexCoord0 - center);

    // Ring properties
    float strokeWidth = 0.015;
    float minRadius = 0.0;
    float maxRadius = 0.5;
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * 1.0;

    // Rings
    float ringRadius1 = calcRingRadius(minRadius, maxRadius, ubo.animationProgress, 0.0);
    float ring1 = composeRing(centerDistance, strokeWidth, ringRadius1);

    float ringRadius2 = calcRingRadius(minRadius, maxRadius, ubo.animationProgress, 0.33 * 1.0);
    float ring2 = composeRing(centerDistance, strokeWidth, ringRadius2);

    float ringRadius3 = calcRingRadius(minRadius, maxRadius, ubo.animationProgress, 0.66 * 1.0);
    float ring3 = composeRing(centerDistance, strokeWidth, ringRadius3);

    // Radial gradient
    float gradientWidth = 5.0;
    float gradientBlur = 0.8;
    float gradientOffset = 0.28;
    float gradientMask = 1.0 - max(0.0, 0.25 - pow(max(0.0, abs(centerDistance - gradientOffset) * gradientWidth), gradientWidth * gradientBlur));

    // Output rings
    float rings = ring1 + ring2 + ring3;
    outColor = color * rings - gradientMask;
}
