/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Element.h>
#include <VCELL/MembraneRegion.h>
#include <Expression.h>
#include <VCELL/SimTool.h>
#include <VCELL/Solver.h>
#include <VCELL/Membrane.h>
#include <VCELL/Mesh.h>
#include <SimpleSymbolTable.h>

MembraneVarContextExpression::MembraneVarContextExpression(Membrane *membrane, MembraneVariable* var)
: MembraneVarContext(membrane, var)
{
}

void MembraneVarContextExpression::resolveReferences(Simulation* sim) {
	VarContext::resolveReferences(sim);
	bindAll((SimulationExpression*)sim);
}

double MembraneVarContextExpression::getInitialValue(MembraneElement *element){
	return evaluateExpression(element, INITIAL_VALUE_EXP);
}

double MembraneVarContextExpression::getMembraneReactionRate(MembraneElement *element){
	return evaluateExpression(element, REACT_RATE_EXP);
}
double MembraneVarContextExpression::getMembraneDiffusionRate(MembraneElement *element){
	return evaluateExpression(element, DIFF_RATE_EXP);
}

double MembraneVarContextExpression::getXmBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_XM_EXP);
}
double MembraneVarContextExpression::getXpBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_XP_EXP);
}
double MembraneVarContextExpression::getYmBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_YM_EXP);
}
double MembraneVarContextExpression::getYpBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_YP_EXP);
}
double MembraneVarContextExpression::getZmBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_ZM_EXP);
}
double MembraneVarContextExpression::getZpBoundaryValue(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_ZP_EXP);
}

double MembraneVarContextExpression::getXmBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_XM_EXP);
}
double MembraneVarContextExpression::getXpBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_XP_EXP);
}
double MembraneVarContextExpression::getYmBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_YM_EXP);
}
double MembraneVarContextExpression::getYpBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_YP_EXP);
}
double MembraneVarContextExpression::getZmBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_ZM_EXP);
}
double MembraneVarContextExpression::getZpBoundaryFlux(MembraneElement *element){
	return evaluateExpression(element, BOUNDARY_ZP_EXP);
}

bool MembraneVarContextExpression::isNullExpressionOK(int expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP) {
		return false;
	}
	Solver* solver = SimTool::getInstance()->getSimulation()->getSolverFromVariable(species);
	if (solver != 0 && solver->isPDESolver()) {
		if (expIndex == DIFF_RATE_EXP) {
			return false;
		}
		int geodim = sim->getMesh()->getDimension();
		if ((geodim >= 1 && (expIndex == BOUNDARY_XM_EXP || expIndex == BOUNDARY_XP_EXP)) 
			|| (geodim >= 2 && (expIndex == BOUNDARY_YM_EXP || expIndex == BOUNDARY_YP_EXP))
			|| (geodim >= 3 && (expIndex == BOUNDARY_ZM_EXP || expIndex == BOUNDARY_ZP_EXP))) {
			return false;
		}
	}
	return true;
}
