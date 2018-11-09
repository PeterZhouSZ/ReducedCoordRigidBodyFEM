// SpringDamper 
//		f = k * (l-L)/L - d * v
//		
#pragma once
#ifndef REDUCEDCOORD_SRC_SPRINGDAMPER_H_
#define REDUCEDCOORD_SRC_SPRINGDAMPER_H_

#include "Spring.h"

class Body;
class Node;

class SpringDamper : public Spring
{
public:
	SpringDamper();
	SpringDamper(std::shared_ptr<Body> body0, Vector3d r0, std::shared_ptr<Body> body1, Vector3d r1);
	virtual ~SpringDamper() {}

	void setStiffness(double K) { m_K = K; }
	void setDamping(double damping) { m_damping = damping; }
	void setRestLength(double L) { m_L = L; }

	void init();
	void load(const std::string &RESOURCE_DIR);
	void draw_(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog, const std::shared_ptr<Program> progSimple, std::shared_ptr<MatrixStack> P) const;

	void computeEnergies_(Eigen::Vector3d grav, Energy &ener);

private:
	void computeFKD(Vector12d &f, Matrix12d &K, Matrix12d &D);

protected:
	void computeForceStiffnessDamping_(Vector3d grav, Eigen::VectorXd &f, Eigen::MatrixXd &K, Eigen::MatrixXd &D);
	void computeStiffnessProd_(Eigen::VectorXd x, Eigen::VectorXd &y);
	void computeDampingProd_(Eigen::VectorXd x, Eigen::VectorXd &y);

	double m_L;		// Rest Length
	double m_l;		// Current Length

	double m_K;
	double m_damping;
	std::shared_ptr<Body> m_body0;
	std::shared_ptr<Body> m_body1;
	Vector3d m_r0;
	Vector3d m_r1;

};


#endif // REDUCEDCOORD_SRC_SPRINGSERIAL_H_