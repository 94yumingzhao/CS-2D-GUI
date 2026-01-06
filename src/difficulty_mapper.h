// difficulty_mapper.h - Difficulty Level to Generator Parameters Mapping
//
// Provides preset configurations for different difficulty levels

#ifndef DIFFICULTY_MAPPER_H_
#define DIFFICULTY_MAPPER_H_

#include <QString>

// Difficulty levels
enum class DifficultyLevel {
    kEasy,      // Simple: fewer types, low size ratio, high demand
    kMedium,    // Balanced configuration
    kHard,      // Complex: more types, high size ratio, low demand
    kExpert     // Extreme: prime offsets, challenging configurations
};

// Scale levels
enum class ScaleLevel {
    kSmall,     // 10-20 item types
    kMedium,    // 20-40 item types
    kLarge      // 40-80 item types
};

// Generator configuration structure
struct GeneratorConfig {
    // Scale parameters
    int num_types = 20;         // Number of item types (5-100)
    int stock_width = 200;      // Stock plate width W
    int stock_length = 400;     // Stock plate length L

    // Size parameters (ratio relative to stock area)
    double min_size_ratio = 0.08;   // Min item area ratio (0.03-0.25)
    double max_size_ratio = 0.35;   // Max item area ratio (0.15-0.60)
    double size_cv = 0.30;          // Size coefficient of variation (0.0-0.8)

    // Demand parameters
    int min_demand = 1;         // Min demand quantity (1-10)
    int max_demand = 15;        // Max demand quantity (2-50)
    double demand_skew = 0.0;   // Demand skewness (0=uniform, 1=highly skewed)

    // Advanced options
    bool prime_offset = false;  // Prime offset (increases indivisibility)
    int num_clusters = 0;       // Size clustering (0=none, 2-5=clustered)
    double peak_ratio = 0.0;    // Hot item ratio (0=uniform, 0.1-0.3=some high demand)

    // Generation strategy
    int strategy = 1;           // 0=reverse, 1=random, 2=cluster, 3=residual

    // Common settings
    int seed = 0;               // Random seed (0=auto)
    int count = 1;              // Number of instances to generate
    QString output_path;        // Output directory
};

// Difficulty mapping utilities
class DifficultyMapper {
public:
    // Get preset configuration for difficulty and scale
    static GeneratorConfig GetPreset(DifficultyLevel diff, ScaleLevel scale);

    // Estimate difficulty score from configuration (0-100)
    static double EstimateDifficultyScore(const GeneratorConfig& config);

    // Estimate expected Gap range as string
    static QString EstimateGap(const GeneratorConfig& config);

    // Get utilization estimate (lower bound)
    static double EstimateUtilization(const GeneratorConfig& config);

private:
    // Calculate size ratio impact
    static double CalcSizeRatioFactor(const GeneratorConfig& config);

    // Calculate demand distribution impact
    static double CalcDemandFactor(const GeneratorConfig& config);

    // Calculate type count impact
    static double CalcTypeCountFactor(const GeneratorConfig& config);
};

#endif  // DIFFICULTY_MAPPER_H_
