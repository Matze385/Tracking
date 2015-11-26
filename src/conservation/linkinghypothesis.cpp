#include "conservation/linkinghypothesis.h"
#include <stdexcept>

using namespace helpers;

namespace constracking
{

LinkingHypothesis::LinkingHypothesis():
	srcId_(-1),
	destId_(-1),
	opengmVariableId_(-1)
{}

LinkingHypothesis::LinkingHypothesis(int srcId, int destId, const helpers::FeatureVector& features):
	srcId_(srcId),
	destId_(destId),
	features_(features),
	opengmVariableId_(-1)
{}

const std::pair<int, int> LinkingHypothesis::readFromJson(const Json::Value& entry)
{
	if(!entry.isObject())
		throw std::runtime_error("Cannot extract LinkingHypothesis from non-object JSON entry");
	if(!entry.isMember(JsonTypeNames[JsonTypes::SrcId]) || !entry[JsonTypeNames[JsonTypes::SrcId]].isInt() 
		|| !entry.isMember(JsonTypeNames[JsonTypes::DestId]) || !entry[JsonTypeNames[JsonTypes::DestId]].isInt() 
		|| !entry.isMember(JsonTypeNames[JsonTypes::Features]) || !entry[JsonTypeNames[JsonTypes::Features]].isArray())
		throw std::runtime_error("JSON entry for SegmentationHytpohesis is invalid");

	srcId_ = entry[JsonTypeNames[JsonTypes::SrcId]].asInt();
	destId_ = entry[JsonTypeNames[JsonTypes::DestId]].asInt();

	features_.clear();
	const Json::Value features = entry[JsonTypeNames[JsonTypes::Features]];
	for(int i = 0; i < (int)features.size(); i++)
	{
		features_.push_back(features[i].asDouble());
	}

	// std::cout << "Found linking hypothesis between " << srcId_ << " and " << destId_ << std::endl;
	return std::make_pair(srcId_, destId_);
}

void LinkingHypothesis::toDot(std::ostream& stream, const Solution* sol) const
{
	stream << "\t" << srcId_ << " -> " << destId_;

	if(sol != nullptr && opengmVariableId_ > 0 && sol->at(opengmVariableId_) > 0)
		stream << "[ color=\"blue\" fontcolor=\"blue\" ]";

	stream << "; \n" << std::flush;
}

void LinkingHypothesis::registerWithSegmentations(std::map<int, SegmentationHypothesis>& segmentationHypotheses)
{
	assert(segmentationHypotheses.find(srcId_) != segmentationHypotheses.end());
	assert(segmentationHypotheses.find(destId_) != segmentationHypotheses.end());

	// std::cout << "Registering outgoing link for " << srcId_ << std::endl;
	segmentationHypotheses[srcId_].addOutgoingLink(shared_from_this());
	// std::cout << "Registering incoming link for " << destId_ << std::endl;
	segmentationHypotheses[destId_].addIncomingLink(shared_from_this());
}

void LinkingHypothesis::addToOpenGMModel(
	GraphicalModelType& model, 
	WeightsType& weights, 
	const std::vector<size_t>& weightIds)
{
	// std::cout << "Adding linking hypothesis between " << srcId_ << " and " << destId_ << " to opengm" << std::endl;

	// Add variable to model. All Variables are binary!
	size_t numLabels = 2;
	model.addVariable(numLabels);
	opengmVariableId_ = model.numberOfVariables() - 1;

	// add unary factor to model
	std::vector<FeaturesAndIndicesType> featuresAndWeightsPerLabel;
	size_t numFeatures = features_.size();
	assert(weightIds.size() == numFeatures * 2);

	for(size_t l = 0; l < numLabels; l++)
	{
		FeaturesAndIndicesType featureAndIndex;

		featureAndIndex.features = features_;
		for(size_t i = 0; i < numFeatures; ++i)
			featureAndIndex.weightIds.push_back(weightIds[l * numFeatures + i]);

		featuresAndWeightsPerLabel.push_back(featureAndIndex);
	}

	LearnableUnaryFuncType unary(weights, featuresAndWeightsPerLabel);
	GraphicalModelType::FunctionIdentifier fid = model.addFunction(unary);
	model.addFactor(fid, &opengmVariableId_, &opengmVariableId_+1);
}

const Json::Value LinkingHypothesis::toJson(bool state) const
{
	Json::Value val;
	val[JsonTypeNames[JsonTypes::SrcId]] = Json::Value(srcId_);
	val[JsonTypeNames[JsonTypes::DestId]] = Json::Value(destId_);
	val[JsonTypeNames[JsonTypes::Value]] = Json::Value(state);
	return val;
}

} // end namespace constracking
