#pragma once

class DCBlock {
public:
    // https://ccrma.stanford.edu/~jos/fp/DC_Blocker_Software_Implementations.html
    // https://ccrma.stanford.edu/~jos/filters/DC_Blocker.html
    float filter(float x) {
        const float r = 0.995f;
        float       y = x - xm1 + r * ym1;
        xm1           = x;
        ym1           = y;
        return y;
    }
    float xm1 = 0;
    float ym1 = 0;
};
