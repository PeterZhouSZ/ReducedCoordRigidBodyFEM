#include "SoftBodyIsotropicHyperelasticFEM.h"

#include <iostream>

#include "Node.h"
#include "Shape.h"
#include "Program.h"
#include "Body.h"
#include "MatrixStack.h"
#include "Tetrahedron.h"

using namespace std;
using namespace Eigen;

SoftBodyIsotropicHyperelasticFEM::SoftBodyIsotropicHyperelasticFEM() {
	m_isInverted = false;
}

SoftBodyIsotropicHyperelasticFEM::SoftBodyIsotropicHyperelasticFEM(double density, double young, double poisson, Material material) :
	SoftBody(density, young, poisson, material)
{
	m_isInverted = false;
	m_isGravity = true;
}

void SoftBodyIsotropicHyperelasticFEM::computeForce_(Vector3d grav, VectorXd &f) {
	// Computes force vector
	if (m_isGravity) {
		for (int i = 0; i < (int)m_nodes.size(); i++) {
			int idxM = m_nodes[i]->idxM;
			double m = m_nodes[i]->m;
			f.segment<3>(idxM) += m * grav;
		}
	}
	m_isInverted = false;
	// Elastic Forces
	if (m_isElasticForce) {
		for (int i = 0; i < (int)m_tets.size(); i++) {
			auto tet = m_tets[i];
			f = tet->computeInvertibleElasticForces(f);
			if (tet->m_isInverted) {
				m_isInverted = true;
				cout << "tet " << i << " is inverted!" << endl;
			}
		}
	}
}

void SoftBodyIsotropicHyperelasticFEM::computeStiffness_(MatrixXd &K) {
	VectorXd df(3 * m_nodes.size());
	VectorXd temp = df;
	VectorXd Dx = df;

	for (int i = 0; i < (int)m_tets.size(); i++) {
		auto tet = m_tets[i];
		if (tet->m_isInverted) {
			cout << "tet " << i << " is inverted!" << endl;
		}

		for (int ii = 0; ii < 4; ii++) {
			auto node = tet->m_nodes[ii];
			int id = node->i;
			int col = node->idxM;

			for (int iii = 0; iii < 3; iii++) {
				df.setZero();
				temp.setZero();
				Dx.setZero();
				Dx(3 * id + iii) = 1.0;
				//tet->computeInvertibleForceDifferentials(Dx, df);
				int irow = col - 3 * id;
				int icol = col + iii;

				tet->computeInvertibleForceDifferentials(Dx, irow, icol, K);

				//tet->computeForceDifferentials(Dx, temp);
				//tet->computeInvertibleForceDifferentials(Dx, temp);
				//cout << "diff" << (df - temp).norm() << endl;

				//K.block(col - 3 * id, col + iii, 3 * m_nodes.size(), 1) += df;
			}
		}
	}

}

void SoftBodyIsotropicHyperelasticFEM::computeStiffnessSparse_(vector<T> &K_) {
	VectorXd df(3 * m_nodes.size());
	VectorXd Dx = df;
	for (int i = 0; i < (int)m_tets.size(); i++) {
		auto tet = m_tets[i];
		for (int ii = 0; ii < 4; ii++) {
			auto node = tet->m_nodes[ii];
			int id = node->i;
			int col = node->idxM;

			for (int iii = 0; iii < 3; iii++) {
				df.setZero();
				Dx.setZero();
				Dx(3 * id + iii) = 1.0;


				//tet->computeForceDifferentials(Dx, df);
				int irow = col - 3 * id;
				int icol = col + iii;

				tet->computeInvertibleForceDifferentialsSparse(Dx, irow, icol, K_);

				//tet->computeInvertibleForceDifferentials(Dx, df);
				//cout << df << endl;
			}
		}
	}
}