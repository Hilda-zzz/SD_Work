#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <map>

// Part types
enum class PartTier { Light, Medium, Heavy, Extreme };

// Car stats
struct CarStats {
	double weight;
	double acceleration;
	double handling;
	double total;

	// Pre-body stats
	double preBodyWeight;
	double preBodyAcceleration;
	double preBodyHandling;

	// Body contributions
	double bodyWeight;
	double bodyAcceleration;
	double bodyHandling;
};

// Car configuration
struct CarConfiguration {
	PartTier spoiler;
	PartTier engine;
	PartTier wheels;
	PartTier body;
	CarStats stats;
};

// Convert PartTier to string for output
std::string tierToString(PartTier tier) {
	switch (tier) {
	case PartTier::Light: return "Light";
	case PartTier::Medium: return "Medium";
	case PartTier::Heavy: return "Heavy";
	case PartTier::Extreme: return "Extreme";
	default: return "Unknown";
	}
}

// Calculate car stats based on parts
CarStats calculateCarStats(PartTier spoiler, PartTier engine, PartTier wheels, PartTier body) {
	// Base stats
	const double baseWeight = 100.0;
	const double baseAcceleration = 100.0;
	const double baseHandling = 100.0;

	// Start with base stats
	double finalWeight = baseWeight;
	double finalAcceleration = baseAcceleration;
	double finalHandling = baseHandling;

	// Stats after applying first 3 parts (before body)
	double preBodyWeight = 0.0;
	double preBodyAcceleration = 0.0;
	double preBodyHandling = 0.0;

	// Body's contribution
	double bodyWeight = 0.0;
	double bodyAcceleration = 0.0;
	double bodyHandling = 0.0;

	// Apply Spoiler effects - focus on Weight
	switch (spoiler) {
	case PartTier::Light:
		finalWeight += baseWeight * 0.25;
		// No negative at Light tier
		break;
	case PartTier::Medium:
		finalWeight += baseWeight * 0.40;
		finalHandling -= baseHandling * 0.15;
		break;
	case PartTier::Heavy:
		finalWeight += baseWeight * 0.55;
		finalHandling -= baseHandling * 0.30;
		break;
	case PartTier::Extreme:
		finalWeight += baseWeight * 0.70;
		finalHandling -= baseHandling * 0.45;
		break;
	}

	// Apply Engine effects - focus on Acceleration
	switch (engine) {
	case PartTier::Light:
		finalAcceleration += baseAcceleration * 0.25;
		// No negative at Light tier
		break;
	case PartTier::Medium:
		finalAcceleration += baseAcceleration * 0.40;
		finalWeight -= baseWeight * 0.15;
		break;
	case PartTier::Heavy:
		finalAcceleration += baseAcceleration * 0.55;
		finalWeight -= baseWeight * 0.30;
		break;
	case PartTier::Extreme:
		finalAcceleration += baseAcceleration * 0.70;
		finalWeight -= baseWeight * 0.45;
		break;
	}

	// Apply Wheels effects - focus on Handling
	switch (wheels) {
	case PartTier::Light:
		finalHandling += baseHandling * 0.25;
		// No negative at Light tier
		break;
	case PartTier::Medium:
		finalHandling += baseHandling * 0.40;
		finalAcceleration -= baseAcceleration * 0.15;
		break;
	case PartTier::Heavy:
		finalHandling += baseHandling * 0.55;
		finalAcceleration -= baseAcceleration * 0.30;
		break;
	case PartTier::Extreme:
		finalHandling += baseHandling * 0.70;
		finalAcceleration -= baseAcceleration * 0.45;
		break;
	}

	// Store pre-body stats
	preBodyWeight = finalWeight;
	preBodyAcceleration = finalAcceleration;
	preBodyHandling = finalHandling;

	// Apply Body effects - balanced across attributes
	switch (body) {
	case PartTier::Light:
		// Light body gives small boost to all stats
		bodyWeight = baseWeight * 0.08;
		bodyAcceleration = baseAcceleration * 0.08;
		bodyHandling = baseHandling * 0.08;
		// No negative effect for Light tier
		break;
	case PartTier::Medium:
		// Medium body with more focus on Handling
		bodyHandling = baseHandling * 0.25;
		bodyWeight = -baseWeight * 0.10;
		bodyAcceleration = -baseAcceleration * 0.05;
		break;
	case PartTier::Heavy:
		// Heavy body with more focus on Acceleration
		bodyAcceleration = baseAcceleration * 0.25;
		bodyWeight = -baseWeight * 0.05;
		bodyHandling = -baseHandling * 0.10;
		break;
	case PartTier::Extreme:
		// Extreme body with more focus on Weight
		bodyWeight = baseWeight * 0.25;
		bodyAcceleration = -baseAcceleration * 0.10;
		bodyHandling = -baseHandling * 0.05;
		break;
	}

	// Apply body effects to final stats
	finalWeight += bodyWeight;
	finalAcceleration += bodyAcceleration;
	finalHandling += bodyHandling;

	// Calculate total
	double total = finalWeight + finalAcceleration + finalHandling;

	// Return struct now includes both final values and the breakdown
	return { finalWeight, finalAcceleration, finalHandling, total,
			preBodyWeight, preBodyAcceleration, preBodyHandling,
			bodyWeight, bodyAcceleration, bodyHandling };
}

int main() {
	std::vector<CarConfiguration> allConfigurations;
	std::vector<PartTier> tiers = { PartTier::Light, PartTier::Medium, PartTier::Heavy, PartTier::Extreme };

	// Generate all possible configurations
	for (const auto& spoiler : tiers) {
		for (const auto& engine : tiers) {
			for (const auto& wheels : tiers) {
				for (const auto& body : tiers) {
					CarStats stats = calculateCarStats(spoiler, engine, wheels, body);
					allConfigurations.push_back({ spoiler, engine, wheels, body, stats });
				}
			}
		}
	}

	// Sort configurations by total stats (highest first)
	std::sort(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.total > b.stats.total;
		});

	// Print table header
	std::cout << std::left << std::setw(10) << "Spoiler"
		<< std::setw(10) << "Engine"
		<< std::setw(10) << "Wheels"
		<< std::setw(10) << "Body"
		<< std::right
		<< std::setw(28) << "Weight (Base + Body = Total)"
		<< std::setw(28) << "Accel (Base + Body = Total)"
		<< std::setw(28) << "Handling (Base + Body = Total)"
		<< std::setw(10) << "Total" << std::endl;

	std::cout << std::string(132, '-') << std::endl;

	// Print all configurations
	for (const auto& config : allConfigurations) {
		std::cout << std::left << std::setw(10) << tierToString(config.spoiler)
			<< std::setw(10) << tierToString(config.engine)
			<< std::setw(10) << tierToString(config.wheels)
			<< std::setw(10) << tierToString(config.body)
			<< std::right << std::fixed << std::setprecision(1)
			<< std::setw(15) << config.stats.preBodyWeight << " + " << std::setw(1) << config.stats.bodyWeight
			<< " = " << std::setw(6) << config.stats.weight
			<< std::setw(15) << config.stats.preBodyAcceleration << " + " << std::setw(1) << config.stats.bodyAcceleration
			<< " = " << std::setw(6) << config.stats.acceleration
			<< std::setw(15) << config.stats.preBodyHandling << " + " << std::setw(1) << config.stats.bodyHandling
			<< " = " << std::setw(6) << config.stats.handling
			<< std::setw(10) << config.stats.total << std::endl;
	}

	// Print summary information
	std::cout << "\nTotal number of configurations: " << allConfigurations.size() << std::endl;

	// Find best configuration for each attribute
	auto bestWeight = std::max_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.weight < b.stats.weight;
		});

	auto bestAcceleration = std::max_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.acceleration < b.stats.acceleration;
		});

	auto bestHandling = std::max_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.handling < b.stats.handling;
		});

	// Find worst configuration for each attribute to get the range
	auto worstWeight = std::min_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.weight < b.stats.weight;
		});

	auto worstAcceleration = std::min_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.acceleration < b.stats.acceleration;
		});

	auto worstHandling = std::min_element(allConfigurations.begin(), allConfigurations.end(),
		[](const CarConfiguration& a, const CarConfiguration& b) {
			return a.stats.handling < b.stats.handling;
		});

	// Print attribute ranges
	std::cout << "\n----- Attribute Ranges -----" << std::endl;
	std::cout << "Weight Range: " << worstWeight->stats.weight << " to " << bestWeight->stats.weight << std::endl;
	std::cout << "Acceleration Range: " << worstAcceleration->stats.acceleration << " to " << bestAcceleration->stats.acceleration << std::endl;
	std::cout << "Handling Range: " << worstHandling->stats.handling << " to " << bestHandling->stats.handling << std::endl;

	// Calculate average and median values for each attribute
	double totalWeight = 0, totalAcceleration = 0, totalHandling = 0;
	std::vector<double> weights, accelerations, handlings;

	// Maps to count occurrences of each unique value
	std::map<double, int> weightCounts;
	std::map<double, int> accelerationCounts;
	std::map<double, int> handlingCounts;

	for (const auto& config : allConfigurations) {
		totalWeight += config.stats.weight;
		totalAcceleration += config.stats.acceleration;
		totalHandling += config.stats.handling;

		weights.push_back(config.stats.weight);
		accelerations.push_back(config.stats.acceleration);
		handlings.push_back(config.stats.handling);

		// Count occurrences of each value
		weightCounts[config.stats.weight]++;
		accelerationCounts[config.stats.acceleration]++;
		handlingCounts[config.stats.handling]++;
	}

	// Calculate averages
	double avgWeight = totalWeight / allConfigurations.size();
	double avgAcceleration = totalAcceleration / allConfigurations.size();
	double avgHandling = totalHandling / allConfigurations.size();

	// Sort for median calculation
	std::sort(weights.begin(), weights.end());
	std::sort(accelerations.begin(), accelerations.end());
	std::sort(handlings.begin(), handlings.end());

	// Calculate medians
	double medianWeight = weights[weights.size() / 2];
	double medianAcceleration = accelerations[accelerations.size() / 2];
	double medianHandling = handlings[handlings.size() / 2];

	// Print statistical information
	std::cout << "\n----- Attribute Statistics -----" << std::endl;
	std::cout << "Weight: Avg = " << std::fixed << std::setprecision(1) << avgWeight
		<< ", Median = " << medianWeight << std::endl;
	std::cout << "Acceleration: Avg = " << avgAcceleration
		<< ", Median = " << medianAcceleration << std::endl;
	std::cout << "Handling: Avg = " << avgHandling
		<< ", Median = " << medianHandling << std::endl;

	// Print frequency distribution for each attribute
	std::cout << "\n----- Attribute Value Frequencies -----" << std::endl;

	std::cout << "Weight Values:" << std::endl;
	std::cout << std::left << std::setw(10) << "Value" << std::setw(15) << "Occurrences" << std::setw(10) << "Percentage" << std::endl;
	std::cout << std::string(35, '-') << std::endl;
	for (const auto& pair : weightCounts) {
		double percentage = (pair.second * 100.0) / allConfigurations.size();
		std::cout << std::left << std::setw(10) << pair.first
			<< std::setw(15) << pair.second
			<< std::setw(10) << std::fixed << std::setprecision(2) << percentage << "%" << std::endl;
	}

	std::cout << "\nAcceleration Values:" << std::endl;
	std::cout << std::left << std::setw(10) << "Value" << std::setw(15) << "Occurrences" << std::setw(10) << "Percentage" << std::endl;
	std::cout << std::string(35, '-') << std::endl;
	for (const auto& pair : accelerationCounts) {
		double percentage = (pair.second * 100.0) / allConfigurations.size();
		std::cout << std::left << std::setw(10) << pair.first
			<< std::setw(15) << pair.second
			<< std::setw(10) << std::fixed << std::setprecision(2) << percentage << "%" << std::endl;
	}

	std::cout << "\nHandling Values:" << std::endl;
	std::cout << std::left << std::setw(10) << "Value" << std::setw(15) << "Occurrences" << std::setw(10) << "Percentage" << std::endl;
	std::cout << std::string(35, '-') << std::endl;
	for (const auto& pair : handlingCounts) {
		double percentage = (pair.second * 100.0) / allConfigurations.size();
		std::cout << std::left << std::setw(10) << pair.first
			<< std::setw(15) << pair.second
			<< std::setw(10) << std::fixed << std::setprecision(2) << percentage << "%" << std::endl;
	}

	//std::cout << "\n----- Best Configurations -----" << std::endl;
	//std::cout << "Best configuration for Weight: "
	//	<< tierToString(bestWeight->spoiler) << " Spoiler, "
	//	<< tierToString(bestWeight->engine) << " Engine, "
	//	<< tierToString(bestWeight->wheels) << " Wheels, "
	//	<< tierToString(bestWeight->body) << " Body" << std::endl;

	//std::cout << "Best configuration for Acceleration: "
	//	<< tierToString(bestAcceleration->spoiler) << " Spoiler, "
	//	<< tierToString(bestAcceleration->engine) << " Engine, "
	//	<< tierToString(bestAcceleration->wheels) << " Wheels, "
	//	<< tierToString(bestAcceleration->body) << " Body" << std::endl;

	//std::cout << "Best configuration for Handling: "
	//	<< tierToString(bestHandling->spoiler) << " Spoiler, "
	//	<< tierToString(bestHandling->engine) << " Engine, "
	//	<< tierToString(bestHandling->wheels) << " Wheels, "
	//	<< tierToString(bestHandling->body) << " Body" << std::endl;

	return 0;
}