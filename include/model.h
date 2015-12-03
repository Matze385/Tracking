#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <vector>
#include <map>

#include "segmentationhypothesis.h"
#include "linkinghypothesis.h"
#include "exclusionconstraint.h"
#include "helpers.h"
#include "settings.h"

namespace mht
{

/**
 * @brief The model holds all detections and their links, as well as exclusion constraints between detections
 * @detail WARNING: at the moment you can only run either learn or infer once on the model. 
 * 		   Build a new one if you need it multiple times
 */
class Model
{
public:	
	/**
	 * @brief Read a model consisting of segmentation hypotheses and linking hypotheses from a json file
	 * @param filename
	 */
	void readFromJson(const std::string& filename);

	/**
	 * @return the number of weights which is estimated by checking how many features are given for detections, links and divisions
	 */
	size_t computeNumWeights();
	
	/**
	 * @brief Find the minimal-energy configuration using an ILP
	 * @param weights a vector of weights to use
	 * @return the vector of per-variable labels, can be used with the detection/linking hypotheses to query their state
	 */
	helpers::Solution infer(const std::vector<helpers::ValueType>& weights);

	/**
	 * @brief Run learning using a given ground truth file
	 * @details Loads the ground truth from another JSON file
	 * 
	 * @param gt_filename JSON file containing a mapping of "src"(int), "dest"(int) -> "value"(bool)
	 * @return the vector of learned weights
	 */
	std::vector<helpers::ValueType> learn(const std::string& gt_filename);

	/**
	 * @brief Export a found solution vector as a readable json file
	 * 
	 * @param filename where to save the result
	 * @param sol the labeling to save
	 */
	void saveResultToJson(const std::string& filename, const helpers::Solution& sol) const;

	/**
	 * @brief Read in a ground truth solution (a boolean value per link) from a json file
	 * 
	 * @param filename where to find the ground truth
	 * @return the solution as a vector of per-opengm-variable labelings
	 */
	helpers::Solution readGTfromJson(const std::string& filename);

	/**
	 * @brief check that the solution does not violate any constraints
	 * @detail WARNING: may only be used after calling initializeOpenGMModel(), learn() or infer() because it needs an initialized opengm model!
	 * 
	 * @param sol solution vector
	 * @return boolean value describing whether this solution is valid
	 */
	bool verifySolution(const helpers::Solution& sol) const;

	/**
	 * @brief Create a graphviz dot output of the full graph, showing used nodes/links in blue and exclusion constraints in red
	 * 
	 * @param filename output filename
	 * @param sol pointer to solution vector, if nullptr it will be ignored
	 */
	void toDot(const std::string& filename, const helpers::Solution* sol = nullptr) const;

	/**
	 * @brief Initialize the OpenGM model by adding variables, factors and constraints.
	 * @detail This is called by learn() or infer()
	 * 
	 * @param weights a reference to the weights object that will be used in all 
	 */
	void initializeOpenGMModel(helpers::WeightsType& weights);

	/**
	 * @return a vector of strings describing each entry in the weight vector
	 */
	std::vector<std::string> getWeightDescriptions();

private:
	// segmentation hypotheses
	std::map<int, SegmentationHypothesis> segmentationHypotheses_;
	// linking hypotheses are stored as shared pointer so it is easier to pass them around
	std::map<std::pair<int, int>, std::shared_ptr<LinkingHypothesis> > linkingHypotheses_;
	// exclusion constraints
	std::vector<ExclusionConstraint> exclusionConstraints_;

	// OpenGM stuff
	helpers::GraphicalModelType model_;

	// model settings
	std::shared_ptr<helpers::Settings> settings_;

	// numbers of weights
	size_t numDetWeights_;
	size_t numDivWeights_;
	size_t numAppWeights_;
	size_t numDisWeights_;
	size_t numLinkWeights_;
};

} // end namespace mht

#endif // MODEL_H
