/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/MembraneRegion.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>

MembraneRegionVarContextExpression:: MembraneRegionVarContextExpression(Membrane *membrane, MembraneRegionVariable* var) 
: MembraneRegionVarContext(membrane, var)
{
}

double MembraneRegionVarContextExpression::getMembraneReactionRate(MembraneElement* element) {
	return evaluateExpression(element, REACT_RATE_EXP);
}

void MembraneRegionVarContextExpression::resolveReferences(Simulation* sim) {
	VarContext::resolveReferences(sim);
	bindAll((SimulationExpression*)sim);
}

double MembraneRegionVarContextExpression::getInitialValue(long regionIndex) {
	return evaluateMembraneRegionExpression(regionIndex, INITIAL_VALUE_EXP);
}

double MembraneRegionVarContextExpression::getUniformRate(MembraneRegion *region){
	return evaluateMembraneRegionExpression(region->getIndex(), UNIFORM_RATE_EXP);
}

bool MembraneRegionVarContextExpression::isNullExpressionOK(int expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP || expIndex == UNIFORM_RATE_EXP) {
		return false;
	}
	return true;
}
