#pragma once

enum class ShadowFilter {
    Hard,
    PCF3X3
};

struct ShadowSettings {
    bool enabled = true;

    float constant_bias = 0.0005f;
    float slope_bias = 0.005f;

    ShadowFilter filter = ShadowFilter::PCF3X3;
};
