// ============================================================================
// 工程标准 (Engineering Standards)
// - 坐标系: 左下角为原点
// - 宽度(Width): 上下方向 (Y轴)
// - 长度(Length): 左右方向 (X轴)
// - 约束: 长度 >= 宽度
// ============================================================================

// difficulty_mapper.cpp - Difficulty Mapping Implementation

#include "difficulty_mapper.h"
#include <cmath>

GeneratorConfig DifficultyMapper::GetPreset(DifficultyLevel diff, ScaleLevel scale) {
    GeneratorConfig config;

    // Base scale configuration
    switch (scale) {
        case ScaleLevel::kSmall:
            config.num_types = 15;
            config.stock_width = 150;
            config.stock_length = 300;
            break;
        case ScaleLevel::kMedium:
            config.num_types = 30;
            config.stock_width = 200;
            config.stock_length = 400;
            break;
        case ScaleLevel::kLarge:
            config.num_types = 60;
            config.stock_width = 300;
            config.stock_length = 600;
            break;
    }

    // Difficulty-specific configuration
    switch (diff) {
        case DifficultyLevel::kEasy:
            // Easy: large items, high demand, low variability
            config.min_size_ratio = 0.10;
            config.max_size_ratio = 0.40;
            config.size_cv = 0.15;
            config.min_demand = 3;
            config.max_demand = 20;
            config.demand_skew = 0.0;
            config.prime_offset = false;
            config.num_clusters = 0;
            config.peak_ratio = 0.0;
            config.strategy = 1;  // Random
            break;

        case DifficultyLevel::kMedium:
            // Medium: balanced configuration
            config.min_size_ratio = 0.08;
            config.max_size_ratio = 0.35;
            config.size_cv = 0.30;
            config.min_demand = 2;
            config.max_demand = 15;
            config.demand_skew = 0.2;
            config.prime_offset = false;
            config.num_clusters = 0;
            config.peak_ratio = 0.1;
            config.strategy = 1;  // Random
            break;

        case DifficultyLevel::kHard:
            // Hard: smaller items, lower demand, higher variability
            config.min_size_ratio = 0.05;
            config.max_size_ratio = 0.25;
            config.size_cv = 0.50;
            config.min_demand = 1;
            config.max_demand = 10;
            config.demand_skew = 0.4;
            config.prime_offset = false;
            config.num_clusters = 3;
            config.peak_ratio = 0.2;
            config.strategy = 2;  // Cluster
            break;

        case DifficultyLevel::kExpert:
            // Expert: extreme configuration with prime offsets
            config.min_size_ratio = 0.03;
            config.max_size_ratio = 0.20;
            config.size_cv = 0.70;
            config.min_demand = 1;
            config.max_demand = 8;
            config.demand_skew = 0.6;
            config.prime_offset = true;
            config.num_clusters = 4;
            config.peak_ratio = 0.25;
            config.strategy = 3;  // Residual
            break;
    }

    // Adjust num_types based on both scale and difficulty
    if (diff == DifficultyLevel::kEasy) {
        config.num_types = static_cast<int>(config.num_types * 0.7);
    } else if (diff == DifficultyLevel::kExpert) {
        config.num_types = static_cast<int>(config.num_types * 1.3);
    }

    // Default output path
    config.output_path = "D:/YM-Code/CS-2D-Data/data/";

    return config;
}

double DifficultyMapper::EstimateDifficultyScore(const GeneratorConfig& config) {
    // Weights for each factor (calibrated from solver experiments)
    const double W_SIZE_RATIO = 0.35;   // Size ratio impact
    const double W_TYPE_COUNT = 0.25;   // Number of types impact
    const double W_DEMAND = 0.15;       // Demand distribution impact
    const double W_SIZE_CV = 0.15;      // Size variability impact
    const double W_PRIME = 0.10;        // Prime offset impact

    double score = 0.0;

    // Size ratio factor: smaller items = harder
    double size_factor = CalcSizeRatioFactor(config);
    score += W_SIZE_RATIO * size_factor;

    // Type count factor: more types = harder (logarithmic)
    double type_factor = CalcTypeCountFactor(config);
    score += W_TYPE_COUNT * type_factor;

    // Demand factor: low demand with skew = harder
    double demand_factor = CalcDemandFactor(config);
    score += W_DEMAND * demand_factor;

    // Size variability: high CV = harder
    double cv_factor = std::min(1.0, config.size_cv / 0.8);
    score += W_SIZE_CV * cv_factor;

    // Prime offset: adds difficulty
    if (config.prime_offset) {
        score += W_PRIME * 1.0;
    }

    // Normalize to 0-100 scale
    return score * 100.0;
}

QString DifficultyMapper::EstimateGap(const GeneratorConfig& config) {
    double score = EstimateDifficultyScore(config);

    if (score < 20) {
        return "< 1%";
    } else if (score < 35) {
        return "1-3%";
    } else if (score < 50) {
        return "3-8%";
    } else if (score < 65) {
        return "5-15%";
    } else if (score < 80) {
        return "10-25%";
    } else {
        return "> 20%";
    }
}

double DifficultyMapper::EstimateUtilization(const GeneratorConfig& config) {
    // Lower size ratio = potentially lower utilization
    double avg_ratio = (config.min_size_ratio + config.max_size_ratio) / 2.0;

    // Base utilization estimate
    double util = 0.85 + avg_ratio * 0.3;

    // Adjust for prime offset (harder to pack)
    if (config.prime_offset) {
        util -= 0.05;
    }

    // Adjust for clustering
    if (config.num_clusters > 0) {
        util += 0.02;  // Clustering can improve packing slightly
    }

    return std::min(0.98, std::max(0.70, util));
}

double DifficultyMapper::CalcSizeRatioFactor(const GeneratorConfig& config) {
    // Average size ratio (smaller = harder)
    double avg_ratio = (config.min_size_ratio + config.max_size_ratio) / 2.0;

    // Invert: smaller ratio = higher difficulty
    // Map 0.05-0.40 to 1.0-0.0
    double factor = 1.0 - (avg_ratio - 0.05) / 0.35;
    return std::max(0.0, std::min(1.0, factor));
}

double DifficultyMapper::CalcDemandFactor(const GeneratorConfig& config) {
    // Average demand (lower = harder)
    double avg_demand = (config.min_demand + config.max_demand) / 2.0;

    // Map 1-20 to 1.0-0.0
    double demand_factor = 1.0 - (avg_demand - 1.0) / 19.0;

    // Skew increases difficulty
    double skew_factor = config.demand_skew;

    return std::max(0.0, std::min(1.0, (demand_factor + skew_factor) / 2.0));
}

double DifficultyMapper::CalcTypeCountFactor(const GeneratorConfig& config) {
    // Logarithmic scale: 10->0.3, 30->0.5, 60->0.7, 100->0.85
    double log_types = std::log(config.num_types) / std::log(100.0);
    return std::max(0.0, std::min(1.0, log_types));
}
