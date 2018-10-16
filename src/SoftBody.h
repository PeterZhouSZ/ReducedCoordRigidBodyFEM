#pragma once
#ifndef MUSCLEMASS_SRC_SOFTBODY_H_
#define MUSCLEMASS_SRC_SOFTBODY_H_

#include <vector>
#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include "MLCommon.h"

class MatrixStack;
class Program;
class Node;
class Body;
class FaceTriangle;
class Tetrahedron;

class SoftBody {

public:
	SoftBody();
	SoftBody(double density, double young, double poisson, Material material);
	virtual ~SoftBody();

	virtual void load(const std::string &RESOURCE_DIR, const std::string &MESH_NAME);
	virtual void init();
	virtual void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog, const std::shared_ptr<Program> progSimple, std::shared_ptr<MatrixStack> P) const;

	virtual void countDofs(int &nm, int &nr);
	void updatePosNor();
	void computeEnergies(Eigen::Vector3d grav, double &T, double &V);
	virtual Eigen::MatrixXd computeJacobian(Eigen::MatrixXd J);
	virtual Eigen::MatrixXd computeMass(Eigen::Vector3d grav, Eigen::MatrixXd M);
	virtual Eigen::VectorXd computeForce(Eigen::Vector3d grav, Eigen::VectorXd f);
	virtual Eigen::MatrixXd computeStiffness(Eigen::MatrixXd K);
	virtual Eigen::VectorXd gatherDofs(Eigen::VectorXd y, int nr);
	virtual Eigen::VectorXd gatherDDofs(Eigen::VectorXd ydot, int nr);
	virtual void scatterDofs(Eigen::VectorXd &y, int nr);
	virtual void scatterDDofs(Eigen::VectorXd &ydot, int nr);

	void setAttachments(int id, std::shared_ptr<Body> body);
	void setAttachmentsByLine(Eigen::Vector3d dir, Eigen::Vector3d orig, std::shared_ptr<Body> body);

	void transform(Eigen::Vector3d dx);
	void setColor(Eigen::Vector3f color) { m_color = color; }
	std::vector<std::shared_ptr<Node> > m_attach_nodes;
	std::vector<std::shared_ptr<Body> > m_attach_bodies;
	std::vector<Eigen::Vector3d> m_r;

	std::shared_ptr<SoftBody> next;
	std::vector<std::shared_ptr<FaceTriangle> > m_trifaces;
private:
	Material m_material;
	Eigen::Vector3f m_color;

	std::vector<std::shared_ptr<Node> > m_nodes;
	
	std::vector<std::shared_ptr<Tetrahedron> > m_tets;

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;

	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;

	double m_young;
	double m_poisson;
	double m_density;
	double m_mass;

};


#endif // MUSCLEMASS_SRC_SOFTBODY_H_