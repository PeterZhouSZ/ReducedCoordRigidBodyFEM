// Constraint Generic constraint
// A constraint can be applied in reduced or maximal coordinates.
// Reduced: keeping a quaternion to be of unit length.
// Maximal: holding two bodies together in world space.

#pragma once
#ifndef MUSCLEMASS_SRC_CONSTRAINT_H_
#define MUSCLEMASS_SRC_CONSTRAINT_H_

#include <vector>
#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include <iostream>

#include "MLCommon.h"

class SE3;
class Body;

class Constraint
{

public:
	Constraint();
	Constraint(int _nconEM, int _nconER, int _nconIM, int _nconIR);

	virtual ~Constraint();

	virtual void update();
	virtual void draw();


	int nconEM;								// Number of maximal equality constraints
	int nconER;								// Number of reduced equality constraints
	int nconIM;								// Number of maximal inequality constraints
	int nconIR;								// Number of reduced inequality constraints
	int idxEM;								// Maximal equality constraint indices
	int idxER;								// Reduced equality constraint indices
	int idxIM;								// Maximal inequality constraint indices
	int idxIR;								// Reduced inequality constraint indices
	int idxQ;								// Associated DOF indices
	bool activeM;							// Whether the maximal inequality constraint is active
	bool activeR;							// Whether the reduced inequality constraint is active
	Vector6d fcon;							// Computed constraint force
	std::shared_ptr<Constraint> next;		// Next constraint in traversal order
	std::string m_name;
	int m_uid;
};



#endif MUSCLEMASS_SRC_CONSTRAINT_H_