#include <MovingBoundarySetup.h>
#include <boundaryProviders.h>
#include <persistcontainer.h>
#include <vcellxml.h>
#include <Logger.h>

using namespace moving_boundary;
//SpeciesSpecification
void SpeciesSpecification::registerType( ) {
	vcell_persist::Registrar::reg<SpeciesSpecification>("SpeciesSpecification");
}

void SpeciesSpecification::persist(std::ostream &os) const {
	vcell_persist::Token::insert<SpeciesSpecification>(os);
	vcell_persist::StdString<>::save(os,name);
	vcell_persist::StdString<>::save(os,initialConcentrationStr);
	vcell_persist::StdString<>::save(os,sourceExpressionStr);
}

SpeciesSpecification::SpeciesSpecification(std::istream &is) {
	vcell_persist::Token::check<SpeciesSpecification>(is);
	vcell_persist::StdString<>::restore(is,name);
	vcell_persist::StdString<>::restore(is,initialConcentrationStr);
	vcell_persist::StdString<>::restore(is,sourceExpressionStr);
}

//MovingBoundarySetup
void MovingBoundarySetup::registerType( ) {
	SpeciesSpecification::registerType( );
	vcell_persist::Registrar::reg<MovingBoundarySetup>("Moving Boundary Setup");
}

MovingBoundarySetup::MovingBoundarySetup(std::istream &is)
	:alternateFrontProvider(nullptr) {
	vcell_persist::Token::check<MovingBoundarySetup>(is);
	vcell_persist::binaryRead(is,frontToNodeRatio);
	vcell_persist::binaryRead(is,maxTime);
	vcell_persist::binaryRead(is,frontTimeStep);
	vcell_persist::binaryRead(is,solverTimeStep);
	vcell_persist::binaryRead(is,hardTime);
	vcell_persist::StdString<>::restore(is,levelFunctionStr);
	vcell_persist::StdString<>::restore(is,advectVelocityFunctionStrX);
	vcell_persist::StdString<>::restore(is,advectVelocityFunctionStrY);
	vcell_persist::StdString<>::restore(is,frontVelocityFunctionStrX);
	vcell_persist::StdString<>::restore(is,frontVelocityFunctionStrY);
	vcell_persist::binaryRead(is,diffusionConstant);
	vcell_persist::restore(is,speciesSpecs);
	bool hasProvider;
	vcell_persist::binaryRead(is,hasProvider);
	if (hasProvider) {
		alternateFrontProvider = restoreFrontProvider(is);
	}
}
MovingBoundarySetup::~MovingBoundarySetup() {
	speciesSpecs.resize(0);
}


void MovingBoundarySetup::persist(std::ostream &os) const {
	vcell_persist::Token::insert<MovingBoundarySetup>(os);
	vcell_persist::binaryWrite(os,frontToNodeRatio);
	vcell_persist::binaryWrite(os,maxTime);
	vcell_persist::binaryWrite(os,frontTimeStep);
	vcell_persist::binaryWrite(os,solverTimeStep);
	vcell_persist::binaryWrite(os,hardTime);
	vcell_persist::StdString<>::save(os,levelFunctionStr);
	vcell_persist::StdString<>::save(os,advectVelocityFunctionStrX);
	vcell_persist::StdString<>::save(os,advectVelocityFunctionStrY);
	vcell_persist::StdString<>::save(os,frontVelocityFunctionStrX);
	vcell_persist::StdString<>::save(os,frontVelocityFunctionStrY);
	vcell_persist::binaryWrite(os,diffusionConstant);
	vcell_persist::save(os,speciesSpecs);
	const bool hasProvider = ( alternateFrontProvider != nullptr );
	vcell_persist::binaryWrite(os,hasProvider);
	if (hasProvider) {
		alternateFrontProvider->persist(os);
	}
}
using tinyxml2::XMLElement;
	//MovingBoundarySetup setupProblem(const XMLElement &root); 

namespace {

	bool convertTrueOrFalse(const char *v) {
		std::string in(v);
		std::transform(in.begin( ),in.end( ),in.begin( ), ::tolower);
		if (in.compare("true") == 0) {
			return true;
		}
		if (in.compare("false") == 0) {
			return false;
		}
		VCELL_EXCEPTION(domain_error,"invalid boolean string " << v << ", must be 'true' or 'false'");
	}

	void readLimits(const tinyxml2::XMLElement &element, spatial::GeoLimit & limits) {
		double low = vcell_xml::convertChildElement<double>(element,"low");
		double high = vcell_xml::convertChildElement<double>(element,"high");
		limits = spatial::GeoLimit(low,high);
	}
}
namespace {
	/**
	* set value to default if it is zero
	* @param value to set
	* @param  deflat default value
	* @return true if substituion made
	*/
	template <typename T>
	void useDefaultIfZero(T & value, T deflt) {
		if (value == 0) {
			value = deflt;
		}
	}
}

moving_boundary::MovingBoundarySetup MovingBoundarySetup::setupProblem(const XMLElement &root) {
	using vcell_xml::convertChildElement;

	moving_boundary::MovingBoundarySetup mbSetup; 
	const XMLElement & prob = vcell_xml::get(root,"problem");

	std::array<spatial::GeoLimit,2> limits;
	const tinyxml2::XMLElement & xlimits = vcell_xml::get(prob,"xLimits"); 
	readLimits(xlimits,limits[0]);
	const tinyxml2::XMLElement & ylimits = vcell_xml::get(prob,"yLimits"); 
	readLimits(ylimits,limits[1]);
	std::array<unsigned short,2> numNodes;
	numNodes[0]  = convertChildElement<unsigned short>(prob,"numNodesX");
	numNodes[1]  = convertChildElement<unsigned short>(prob,"numNodesY");

	moving_boundary::Universe<2>::get( ).init(limits,numNodes);


	mbSetup.frontToNodeRatio = convertChildElement<unsigned int>(prob,"frontToNodeRatio");
	mbSetup.maxTime = convertChildElement<double>(prob,"maxTime");
	mbSetup.advectVelocityFunctionStrX = convertChildElement<std::string>(prob,"advectVelocityFunctionX");
	mbSetup.advectVelocityFunctionStrY = convertChildElement<std::string>(prob,"advectVelocityFunctionY");
	//mbSetup.concentrationFunctionStr = convertChildElement<std::string>(prob,"concentrationFunction");

	std::pair<bool,double> tsPair = vcell_xml::queryElement<double>(prob,"timeStep");
	if (tsPair.first) {
		const double & legacyTS = tsPair.second; 
		mbSetup.frontTimeStep = vcell_xml::convertChildElementWithDefault<double>(prob,"frontTimeStep",0);
		mbSetup.solverTimeStep = vcell_xml::convertChildElementWithDefault<double>(prob,"solverTimeStep",0);
		useDefaultIfZero(mbSetup.frontTimeStep,legacyTS);
		useDefaultIfZero(mbSetup.solverTimeStep,mbSetup.frontTimeStep);
		VCELL_LOG(warn,"legacy timeTime replaced by frontTimeStep and solverTimeStep");
	}
	else {
		mbSetup.frontTimeStep = vcell_xml::convertChildElement<double>(prob,"frontTimeStep");
		mbSetup.solverTimeStep = vcell_xml::convertChildElement<double>(prob,"solverTimeStep");
	}

	mbSetup.hardTime = convertTrueOrFalse(vcell_xml::convertChildElementWithDefault<const char *>(prob,"hardTime","false"));

	using vcell_xml::convertChildElementWithDefault;
	mbSetup.frontVelocityFunctionStrX = 
		convertChildElementWithDefault<std::string>(prob,"frontVelocityFunctionX", mbSetup.advectVelocityFunctionStrX);
	mbSetup.frontVelocityFunctionStrY = 
		convertChildElementWithDefault<std::string>(prob,"frontVelocityFunctionY", mbSetup.advectVelocityFunctionStrY);

	std::pair<bool,std::string> lvq = vcell_xml::queryElement<std::string>(prob,"levelFunction");
	if (lvq.first) {
		 mbSetup.levelFunctionStr = lvq.second;
	}
	const tinyxml2::XMLElement *altFront = prob.FirstChildElement("specialFront");
	if (altFront != nullptr) {
		mbSetup.alternateFrontProvider = ::frontFromXML(*altFront);
		std::cout << mbSetup.alternateFrontProvider->describe( ) << std::endl;
	}
	std::pair<bool,std::string> defaultDiffQ = vcell_xml::queryElement<std::string>(prob,"diffusionConstant");
	const XMLElement & physiologyElement = vcell_xml::get(prob,"physiology");
	const XMLElement *spE = physiologyElement.FirstChildElement("species");
	size_t sNumber = 0;
	std::ostringstream defaultNameS;
	std::string name;
	while (spE != nullptr) {
		//get name, or design default
		const char * n  = spE->Attribute("name");
		if (n != nullptr) {
			name = n;
		}
		else {
			defaultNameS.rdbuf( )->pubseekpos(0); 
			defaultNameS << 'u' << sNumber << std::ends;
			name = defaultNameS.str( );
		}
		++sNumber;

		//get other terms 
		std::string source = vcell_xml::convertChildElement<std::string>(*spE,"source");
		std::string init = vcell_xml::convertChildElementWithDefault<std::string>(*spE,"initial", source);
		std::pair<bool,std::string> dq = vcell_xml::queryElement<std::string>(*spE,"diffusion");
		if (!dq.first && !defaultDiffQ.first) {
			VCELL_EXCEPTION(domain_error, "No diffusion specified for " << name); 
		}
		const std::string & diffusion  = dq.first ? dq.second : defaultDiffQ.second; 
		
		mbSetup.speciesSpecs.push_back(SpeciesSpecification(name,init,source, diffusion));

		spE = spE->NextSiblingElement("species");
	}

	return mbSetup; 
}

