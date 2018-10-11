#pragma once
#ifndef MUSCLEMASS_SRC_NODE_H_
#define MUSCLEMASS_SRC_NODE_H_
#include <vector>
#include <memory>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

class Shape;
class Program;
class MatrixStack;
class Body;

class Node
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	Node();
	Node(const std::shared_ptr<Shape> shape);
	virtual ~Node();

	void load(const std::string &RESOURCE_DIR);
	void init();
	void tare();
	void reset();
	void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p) const;
	void drawNormal(std::shared_ptr<MatrixStack> MV, std::shared_ptr<MatrixStack> P, const std::shared_ptr<Program> p) const;
	Eigen::Vector3d computeNormal();
	void addNormal(Eigen::Vector3d normal) { m_normals.push_back(normal); }
	void clearNormals();
	void update(Eigen::Matrix4d E);

	std::shared_ptr<Body> getParent() const { return this->parent; }

	double getPotentialEnergy() const { return this->V; }
	double getKineticEnergy() const { return this->K; }

	void setParent(std::shared_ptr<Body> _parent) { this->parent = _parent; }

	double computePotentialEnergy(Eigen::Vector3d grav);
	void setColor(Eigen::Vector3f color) { m_color = color; }

	int idxR;
	int idxM;
		std::vector<Eigen::Vector3d> m_normals;

	bool fixed;					// is fixed?
	double r;					// radius
	double m;					// mass
	int i;						// starting index
	Eigen::Vector3d x0;			// initial position
	Eigen::Vector3d v0;			// initial velocity
	Eigen::Vector3d x;			// position
	Eigen::Vector3d v;			// velocity
	Eigen::Vector3d a;			// acceleration
	Eigen::Vector3d normal;	
	int m_nfaces;				// # of adjacent faces
	double s;					// non-dimensional material coordinate [0,1]
								// 0 at muscle origin and 1 at insertion; remain fixed.
	Eigen::Vector3f m_color; 
	double L;
private:
	std::shared_ptr<Shape> sphere;
	double V;					// potential energy
	double K;					// kinetic energy

	std::shared_ptr<Body> parent;	// body attached to
};

#endif // MUSCLEMASS_SRC_NODE_H_
