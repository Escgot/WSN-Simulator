#pragma once

namespace Config {
    // Energy costs (per action)
    constexpr float E_COLLECT    = 0.2f;   // Energy to sense/collect data
    constexpr float E_ELEC       = 0.05f;  // Base electronics cost per transmission
    constexpr float E_AMP        = 0.01f;  // Amplifier cost factor (multiplied by d^2)
    constexpr float E_RECEIVE    = 0.1f;   // Energy to receive a message
    constexpr float E_AGGREGATE  = 0.05f;  // Energy to aggregate data

    // Network defaults
    constexpr float SINK_ENERGY     = 9999.0f;
    constexpr float DEFAULT_RANGE   = 15.0f;
    constexpr int   DEFAULT_ROUNDS  = 10;
}
