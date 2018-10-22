#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Node.h"
#include "Shape.h"
#include "Program.h"
#include "Body.h"
#include "MatrixStack.h"

using namespace std;
using namespace Eigen;

Node::Node() :
	r(0.3),
	m(1.0),
	i(-1),
	x(0.0, 0.0, 0.0),
	v(0.0, 0.0, 0.0),
	m_nfaces(0),
	fixed(false),
	attached(false)
{
	
}

Node::Node(const shared_ptr<Shape> s) :
	r(1.0),
	m(1.0),
	i(-1),
	x(0.0, 0.0, 0.0),
	v(0.0, 0.0, 0.0),
	fixed(false),
	attached(false),
	normal(0.0,0.0,0.0),
	m_nfaces(0),
	sphere(s)
{
	
}

void Node::load(const std::string &RESOURCE_DIR) {

	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere2.obj");

}

void Node::init() {
	sphere->init();
}

Node::~Node()
{
}

void Node::tare()
{
	x0 = x;
	v0 = v;
}

void Node::reset()
{
	x = x0;
	v = v0;
}

void Node::update(Matrix4d E) {
	Vector4d pos;
	pos.segment<3>(0) = this->x0;
	pos(3) = 1.0;

	pos = E * pos;
	this->x = pos.segment<3>(0);
}

void Node::update() {
	update(this->parent->E_wi);
}

double Node::computePotentialEnergy(Vector3d grav) {
	this->V = this->m * grav.transpose() * this->x;
	return this->V;
}

void Node::clearNormals() {
	m_normals.clear();
	normal.setZero();
}

Vector3d Node::computeNormal() {
	normal.setZero();
	for (int i = 0; i < m_normals.size(); i++) {
		normal += m_normals[i];
	}

	normal /= m_normals.size();
	normal.normalized();
	return normal;
}


void Node::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> prog) const
{	
	if(sphere) {
		MV->pushMatrix();
		MV->translate(x(0), x(1), x(2));
		MV->scale(r);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		sphere->draw(prog);
		MV->popMatrix();
	}
}

void Node::drawNormal(shared_ptr<MatrixStack> MV, shared_ptr<MatrixStack> P, const shared_ptr<Program> prog) const
{
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glColor3f(0.8, 0.7, 0.0);
	glLineWidth(2);
	glBegin(GL_LINES);
	Vector3f p0 = x.cast<float>();
	glVertex3f(p0(0), p0(1), p0(2));
	Vector3f p1 = (p0 + this->normal.cast<float>());
	glVertex3f(p1(0), p1(1), p1(2));
	glEnd();
	prog->unbind();
}