#pragma once

#ifndef MUSCLEMASS_SRC_BODY_H_
#define MUSCLEMASS_SRC_BODY_H_

#include <vector>
#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include "MLCommon.h"

class Shape;
class Wrench;
class Joint;
class Program;
class MatrixStack;

class Body 
{
public:
	Body(double _density, Eigen::Vector3d _sides);
	virtual ~Body();


	void setTransform(Eigen::Matrix4d E);
	void computeInertia();
	MatrixXd computeMass(Eigen::Vector3d grav, MatrixXd M);
	VectorXd computeForce(Eigen::Vector3d grav, MatrixXd f);

	Energy computeEnergies(Eigen::Vector3d grav, Energy energies);

	void load(const std::string &RESOURCE_DIR);
	void init();
	void update();
	void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> P)const;

	double density;			// Mass/volume
	Eigen::Vector3d sides;

	Matrix6d I_j;			// Inertia at the parent joint
	Vector6d I_i;			// Inertia at body center

	Eigen::Matrix4d E_ji;	// Where the body is wrt joint
	Eigen::Matrix4d E_ij;	// Where the joint is wrt body

	Eigen::Matrix4d E_wi;	// Where the body is wrt world
	Eigen::Matrix4d E_iw;	// Where the world is wrt body
	Eigen::Matrix4d E_ip;   // Where the parent is wrt body
	Matrix6d Ad_ji;			// Adjoint of E_ji
	Matrix6d Ad_ij;			// Adjoint of E_ij
	Matrix6d Ad_iw;			// Adjoint of E_iw
	Matrix6d Ad_wi;			// Adjoint of E_wi
	Matrix6d Ad_ip;			// Adjoint of E_ip
	Matrix6d Addot_wi;		// Adjoint dot of E_wi

	Vector6d V;				// Twist at parent joint
	Vector6d Vdot;			// Acceleration at parent joint
	Vector6d phi;			// Twist at body center
	Joint *joint;			// Joint to parent
	int idxM;				// Maximal indices
	Body *next;				// Next body in traversal order

private:
	const std::shared_ptr<Shape> box;

	void computeInertiaBody();
	void computeInertiaJoint();



};



#endif // MUSCLEMASS_SRC_BODY_H_
