#pragma once
#define EIGEN_USE_MKL_ALL

#include <vector>
#include <memory>
#include <string>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <nlohmann/json.hpp>
#include "MLCommon.h"

class MatrixStack;
class Program;
class FaceTriangle;
class Node;

class Surface
{

public:
	Surface() {
		m_color << 1.0f, 1.0f, 0.0f;
		m_isCollisionWithFloor = false;
	}

	virtual ~Surface() {}

	void load(const std::string &RESOURCE_DIR, const std::string &MESH_NAME, const std::string &TETGEN_FLAGS);
	void init();
	void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> P) const;
	void updatePosNor();
	inline void setColor(Vector3f color) { m_color = color; }
	void transform(Matrix4d E);
	inline const std::vector<std::shared_ptr<FaceTriangle> > & getFaces() const { return m_trifaces; }
	inline const std::vector<std::shared_ptr<Node> > & getNodes() const { return m_nodes; }
	inline void setFloor(double floor_y) { m_floor_y = floor_y; m_isCollisionWithFloor = true; }
	bool m_isCollisionWithFloor;
	double m_floor_y;

	void exportObj(std::ofstream& outfile);
protected:
	Vector3f m_color;


	std::vector<std::shared_ptr<Node> > m_nodes;
	std::vector<std::shared_ptr<FaceTriangle> > m_trifaces;

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;

	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
};
