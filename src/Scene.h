#pragma once
#ifndef MUSCLEMASS_SRC_SCENE_H_
#define MUSCLEMASS_SRC_SCENE_H_

#include <vector>
#include <memory>
#include <string>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <json.hpp>
#include "MLCommon.h"

class Particle;
class MatrixStack;
class Program;
class Shape;
class Rigid;
class Solver;
class Spring;
class Joint;
class Vector;
class WrapSphere;
class WrapCylinder;
class WrapDoubleCylinder;
class SymplecticIntegrator;
class RKF45Integrator;

class Scene
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW	
	Scene();
	virtual ~Scene();
	
	void load(const std::string &RESOURCE_DIR);
	void init();
	void tare();
	void reset();
	void step();

	std::shared_ptr<Rigid> addBox(nlohmann::json R, nlohmann::json p, nlohmann::json dimension, nlohmann::json scale, nlohmann::json mass, const std::shared_ptr<Shape> shape, nlohmann::json isReduced, int id, std::shared_ptr<Rigid> parent = nullptr);
	std::shared_ptr<Joint> addJoint(std::shared_ptr<Rigid> parent, std::shared_ptr<Rigid> child, nlohmann::json jE_P_J, nlohmann::json jmin_theta, nlohmann::json jmax_theta);
	std::shared_ptr<Spring> addSpring(nlohmann::json jp_x, std::shared_ptr<Rigid> p_parent, nlohmann::json js_x, std::shared_ptr<Rigid> s_parent, nlohmann::json jmass);
	std::shared_ptr<WrapSphere> addSphere(nlohmann::json jp_x, std::shared_ptr<Rigid> p_parent, nlohmann::json js_x, std::shared_ptr<Rigid> s_parent, nlohmann::json jo_x, std::shared_ptr<Rigid> o_parent, nlohmann::json jradius);
	std::shared_ptr<WrapCylinder> addWrapCylinder(nlohmann::json jp_x, std::shared_ptr<Rigid> p_parent, nlohmann::json js_x, std::shared_ptr<Rigid> s_parent, nlohmann::json jo_x, std::shared_ptr<Rigid> o_parent, nlohmann::json jradius, nlohmann::json jzdir);
	std::shared_ptr<WrapDoubleCylinder> addWrapDoubleCylinder(nlohmann::json jp_x, std::shared_ptr<Rigid> p_parent, nlohmann::json js_x, std::shared_ptr<Rigid> s_parent, nlohmann::json ju_x, std::shared_ptr<Rigid> u_parent, nlohmann::json jv_x, std::shared_ptr<Rigid> v_parent, nlohmann::json juradius, nlohmann::json jvradius, nlohmann::json jzudir, nlohmann::json jzvdir);
	
	void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog, const std::shared_ptr<Program> prog2, std::shared_ptr<MatrixStack> P) const;
	void computeEnergy();
	void saveData(int num_steps);
	double getTime() const { return t; }

private:
	double t;
	double h;
	int step_i;
	Eigen::Vector3d grav;
	int n_step;
	Eigen::VectorXd theta_list;
	double t_start;
	double t_stop;

	double V; // potential energy
	double K; // kinetic energy
	double V0;
	double K0;

	Vector12d phi; // twist
	std::vector < double > Kvec;
	std::vector < double > Vvec;
	std::vector < double > Tvec;
	std::vector < Vector12d > Twist_vec;

	std::vector<double> y;
	std::vector<double> yp;

	std::shared_ptr<Shape> sphereShape;
	std::shared_ptr<Shape> boxShape;
	std::shared_ptr<Shape> cylinderShape;

	std::vector< std::shared_ptr<Rigid> > boxes;
	std::vector< std::shared_ptr<Joint> > joints;

	std::shared_ptr<Solver> solver;	

	std::vector< std::shared_ptr<Spring> > springs;
	std::vector< std::shared_ptr<WrapCylinder> > wrap_cylinders;
	std::vector< std::shared_ptr<WrapDoubleCylinder> > wrap_doublecylinders;
	std::vector< std::shared_ptr<WrapSphere> > wrap_spheres;

	std::shared_ptr<SymplecticIntegrator> symplectic_solver;
	std::shared_ptr<RKF45Integrator> rkf45_solver;
	nlohmann::json js;
	Integrator time_integrator;
};

#endif // MUSCLEMASS_SRC_SCENE_H_