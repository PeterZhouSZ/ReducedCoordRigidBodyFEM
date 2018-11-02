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

	void computeJacEqM(Eigen::MatrixXd &Gm, Eigen::MatrixXd &Gmdot, Eigen::VectorXd &gm, Eigen::VectorXd &gmdot, Eigen::VectorXd &gmddot);	
	void computeJacEqR(Eigen::MatrixXd &Gr, Eigen::MatrixXd &Grdot, Eigen::VectorXd &gr);
	void computeJacIneqM(Eigen::MatrixXd &Cm, Eigen::MatrixXd &Cmdot, Eigen::VectorXd &cm);
	void computeJacIneqR(Eigen::MatrixXd &Cr, Eigen::MatrixXd &Crdot, Eigen::VectorXd &cr);

	void countDofs(int &nem, int &ner, int &nim, int &nir);
	void getActiveList(std::vector<int> &listM, std::vector<int> &listR);

	void scatterForceEqM(Eigen::MatrixXd Gmt, Eigen::VectorXd lm);
	void scatterForceEqR(Eigen::MatrixXd Grt, Eigen::VectorXd lr);
	void scatterForceIneqR(Eigen::MatrixXd Crt, Eigen::VectorXd lr);
	void scatterForceIneqM(Eigen::MatrixXd Cmt, Eigen::VectorXd lm);

	virtual void computeJacIneqR_(Eigen::MatrixXd &Cr, Eigen::MatrixXd &Crdot, Eigen::VectorXd &cr);
	virtual void computeJacEqM_(Eigen::MatrixXd &Gm, Eigen::MatrixXd &Gmdot, Eigen::VectorXd &gm, Eigen::VectorXd &gmdot, Eigen::VectorXd &gmddot);
	virtual void computeJacEqR_(Eigen::MatrixXd &Gr, Eigen::MatrixXd &Grdot, Eigen::VectorXd &gr);

	int nconEM;								// Number of maximal equality constraints
	int nconER;								// Number of reduced equality constraints
	int nconIM;								// Number of maximal inequality constraints
	int nconIR;								// Number of reduced inequality constraints
	int nQ;								
	int idxEM;								// Maximal equality constraint indices
	int idxER;								// Reduced equality constraint indices
	int idxIM;								// Maximal inequality constraint indices
	int idxIR;								// Reduced inequality constraint indices
	Eigen::MatrixXi idxQ;								// Associated DOF indices
	bool activeM;							// Whether the maximal inequality constraint is active
	bool activeR;							// Whether the reduced inequality constraint is active
	Eigen::VectorXd fcon;							// Computed constraint force
	std::shared_ptr<Constraint> next;		// Next constraint in traversal order
	std::string m_name;
	int m_uid;

protected:
	
	void computeJacIneqM_(Eigen::MatrixXd &Cm, Eigen::MatrixXd &Cmdot, Eigen::VectorXd &cm);
	

	void scatterForceEqM_();
	void scatterForceEqR_();
	void scatterForceIneqR_();
	void scatterForceIneqM_();
};



#endif MUSCLEMASS_SRC_CONSTRAINT_H_