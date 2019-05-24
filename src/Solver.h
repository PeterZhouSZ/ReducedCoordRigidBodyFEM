#pragma once
#ifndef MUSCLEMASS_SRC_SOLVER_H_
#define MUSCLEMASS_SRC_SOLVER_H_
#define EIGEN_USE_MKL_ALL

#include <vector>
#include <memory>
#include <string>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/src/Core/util/IndexedViewHelper.h>

#include <nlohmann/json.hpp>
#include "MLCommon.h"

class World;
class Body;
class Joint;
class Deformable;
class SoftBody;
class Spring;
class Constraint;
class MeshEmbedding;

typedef Eigen::Triplet<double> T;

struct Solution {
	Eigen::VectorXd t;
	Eigen::MatrixXd y;

	Solution(){}

	Eigen::VectorXd step(int time_step) {
		return y.row(time_step);
	}

	int getNsteps() {
		return y.rows();
	}

	void searchTime(double ti, int search_index, int &output_index, double &s) {
		// Finds the index of the time interval around ti
		
		while (ti > t(search_index))
		{
			search_index++;
		}
		output_index = search_index - 1;
		if (output_index < 0) {
			// Beginning of time
			output_index = 0;
			s = 0.0;
		}
		else {
			if (output_index == y.rows() - 1) {
				// End of time
				output_index -= 1;
				s = 1.0;
			}
			else {
				// Somewhere between
				double t0 = t(output_index);
				double t1 = t(output_index + 1);
				s = (ti - t0) / (t1 - t0);
			}
		}
	}

};

class Solver 
{
public:
	Solver();
	Solver(std::shared_ptr<World> world, Integrator integrator);
	virtual ~Solver() {}
	virtual std::shared_ptr<Solution> solve() { return m_solutions; }
	virtual Eigen::VectorXd dynamics(Eigen::VectorXd y) { Eigen::Vector3d z; z.setZero(); return z; }
	virtual void initMatrix(int nm, int nr, int nem, int ner, int nim, int nir) {}
	virtual void reset();

protected:
	std::shared_ptr<Solution> m_solutions;
	std::shared_ptr<World> m_world;
	Integrator m_integrator;
	int nr;
	int nm;
	int nR;
	int ne;
	int ni;
	int ner;
	int nem;
	int nim;
	int nir;
	Eigen::Vector3d grav;
	Eigen::VectorXd yk;
	Eigen::VectorXd ydotk;
	std::shared_ptr<Body> body0;
	std::shared_ptr<Joint> joint0;
	std::shared_ptr<Deformable> deformable0;
	std::shared_ptr<SoftBody> softbody0;
	std::shared_ptr<Constraint> constraint0;
	std::shared_ptr<Spring> spring0;
	std::shared_ptr<MeshEmbedding> meshembedding0;

	int m_ntets;


	double t;
	double h;
	double hsquare;
	int step;
};

#endif // MUSCLEMASS_SRC_SOLVER_H_
