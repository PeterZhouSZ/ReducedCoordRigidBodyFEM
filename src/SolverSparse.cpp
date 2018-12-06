#include "SolverSparse.h"
#include "World.h"
#include "Body.h"
#include "SoftBody.h"
#include "Joint.h"
#include "Spring.h"
#include "SpringDamper.h"
#include "Deformable.h"
#include "DeformableSpring.h"
#include "ConstraintJointLimit.h"
#include "ConstraintLoop.h"
#include "ConstraintAttachSpring.h"
#include "QuadProgMosek.h"
#include "MatlabDebug.h"
#include "MeshEmbedding.h"
#include <iostream>
#include <fstream>
#include <json.hpp>

using namespace std;
using namespace Eigen;

void SolverSparse::initMatrix(int nm, int nr, int nem, int ner, int nim, int nir) {
	ni = nim + nir;
	fm.setZero();
	fr.setZero();
	fr_.setZero();
	tmp.setZero();
	
	Mr_sp.resize(nr, nr);
	Mr_sp.data().squeeze();

	Mr_sp_temp.resize(nr, nr);
	Mr_sp_temp.data().squeeze();

	MDKr_sp.resize(nr, nr);
	MDKr_sp.data().squeeze();

	Kr_sp.resize(nr, nr);
	Kr_sp.data().squeeze();
	Kr_.clear();

	Dr_sp.resize(nr, nr);
	Dr_sp.data().squeeze();
	Dr_.clear();
	
	Dm_sp.resize(nm, nm);
	Dm_sp.data().squeeze();
	Dm_.clear();

	K_sp.resize(nm, nm);
	K_sp.data().squeeze();
	K_.clear();

	Km_sp.resize(nm, nm);
	Km_sp.data().squeeze();
	Km_.clear();

	J_dense.setZero();
	Jdot_dense.setZero();

	J_sp.resize(nm, nr);
	J_sp.data().squeeze();
	J_.clear();

	Jdot_sp.resize(nm, nr);
	Jdot_sp.data().squeeze();
	Jdot_.clear();
	
	Gm_sp.resize(nem, nm);
	Gm_sp.data().squeeze();
	Gm_.clear();

	Gmdot_sp.resize(nem, nm);
	Gmdot_sp.data().squeeze();
	Gmdot_.clear();

	gm.setZero();
	gmdot.setZero();
	gmddot.setZero();

	Gr_sp.resize(ner, nr);
	Gr_sp.data().squeeze();
	Gr_.clear();

	Grdot_sp.resize(ner, nr);
	Grdot_sp.data().squeeze();
	Grdot_.clear();

	gr.setZero();
	grdot.setZero();
	grddot.setZero();

	g.setZero();
	gdot.setZero();
	rhsG.setZero();

	Cm_sp.resize(nim, nm);
	Cm_sp.data().squeeze();
	Cm_.clear();

	Cmdot_sp.resize(nim, nm);
	Cmdot_sp.data().squeeze();
	Cmdot_.clear();

	cm.resize(nim);
	cm.setZero();
	cmdot.resize(nim);
	cmdot.setZero();
	cmddot.resize(nim);
	cmddot.setZero();

	Cr_sp.resize(nir, nr);
	Cr_sp.data().squeeze();
	Cr_.clear();

	Crdot_sp.resize(nir, nr);
	Crdot_sp.data().squeeze();
	Crdot_.clear();

	cr.resize(nir);
	cr.setZero();
	crdot.resize(nir);
	crdot.setZero();
	crddot.resize(nir);
	crddot.setZero();

	G_sp.resize(ne, nr);
	G_sp.data().squeeze();

	Cm.resize(nim, nm);
	Cm.setZero();
	Cmdot.resize(nim, nm);
	Cmdot.setZero();
	Cr.resize(nir, nr);
	Cr.setZero();
	Crdot.resize(nir, nr);
	Crdot.setZero();
}

VectorXd SolverSparse::dynamics(VectorXd y)
{
	//SparseMatrix<double, RowMajor> G_sp;
	switch (m_integrator)
	{
	case REDMAX_EULER:
	{
		if (step == 0) {
			// constant during simulation
			nr = m_world->nr;
			nm = m_world->nm;
			nem = m_world->nem;
			ner = m_world->ner;
			ne = ner + nem;

			nim = m_world->nim;
			nir = m_world->nir;
			ni = nim + nir;

			Mm_sp.resize(nm, nm);
			Mm_sp.data().squeeze();
			Mm_.clear();

			m_dense_nm = m_world->m_dense_nm;
			m_dense_nr = m_world->m_dense_nr;

			yk.resize(2 * nr);
			ydotk.resize(2 * nr);

			fm.resize(nm);
			fr.resize(nr);
			fr_.resize(nr);

			tmp.resize(nm);

			J_dense.resize(m_dense_nm, m_dense_nr);
			Jdot_dense.resize(m_dense_nm, m_dense_nr);

			gr.resize(ner);
			grdot.resize(ner);
			grddot.resize(ner);
			g.resize(ne);
			rhsG.resize(ne);
			gdot.resize(ne);
			gm.resize(nem);
			gmdot.resize(nem);
			gmddot.resize(nem);

			body0 = m_world->getBody0();
			joint0 = m_world->getJoint0();
			deformable0 = m_world->getDeformable0();
			softbody0 = m_world->getSoftBody0();
			constraint0 = m_world->getConstraint0();
			spring0 = m_world->getSpring0();
			meshembedding0 = m_world->getMeshEmbedding0();

			t = m_world->getTspan()(0);
			h = m_world->getH();
			hsquare = h * h;
			this->grav = m_world->getGrav();

		}

		nim = m_world->nim;
		nir = m_world->nir;
		ni = nim + nir;	

		q0 = y.segment(0, nr);
		qdot0 = y.segment(nr, nr);

		initMatrix(nm, nr, nem, ner, nim, nir);

		if (step == 0) {
			body0->computeMassSparse(Mm_);
			deformable0->computeMassSparse(Mm_);
			softbody0->computeMassSparse(Mm_);
			meshembedding0->computeMassSparse(Mm_);
			Mm_sp.setFromTriplets(Mm_.begin(), Mm_.end());
		}

		body0->computeGrav(grav, fm);
		body0->computeForceDampingSparse(tmp, Dm_);
		deformable0->computeForce(grav, fm);
		deformable0->computeForceDampingSparse(grav, tmp, Dm_);

		softbody0->computeForce(grav, fm);
		softbody0->computeStiffnessSparse(K_);

		meshembedding0->computeForce(grav, fm);
		meshembedding0->computeStiffnessSparse(K_);

		joint0->computeForceStiffnessSparse(fr, Kr_);
		joint0->computeForceDampingSparse(tmp, Dr_);

		//// First get dense jacobian (only a small part of the matrix)
		joint0->computeJacobian(J_dense, Jdot_dense);

		//// Push back the dense part
		for (int i = 0; i < J_dense.rows(); ++i) {
			for (int j = 0; j < J_dense.cols(); ++j) {

				J_.push_back(T(i, j, J_dense(i, j)));
				Jdot_.push_back(T(i, j, Jdot_dense(i, j)));
			}
		}

		deformable0->computeJacobianSparse(J_, Jdot_);
		softbody0->computeJacobianSparse(J_);
		meshembedding0->computeJacobianSparse(J_);

		spring0->computeForceStiffnessDampingSparse(fm, Km_, Dm_);

		Km_sp.setFromTriplets(Km_.begin(), Km_.end());
		Dm_sp.setFromTriplets(Dm_.begin(), Dm_.end());
		Dr_sp.setFromTriplets(Dr_.begin(), Dr_.end());
		K_sp.setFromTriplets(K_.begin(), K_.end()); // check
		Kr_sp.setFromTriplets(Kr_.begin(), Kr_.end());
		J_sp.setFromTriplets(J_.begin(), J_.end()); // check
		Jdot_sp.setFromTriplets(Jdot_.begin(), Jdot_.end());
		
		Mr_sp = J_sp.transpose() * (Mm_sp - hsquare * K_sp) * J_sp;
		//Mr_sp_temp = Mr_sp.transpose();
		//Mr_sp += Mr_sp_temp;
		//Mr_sp *= 0.5;

		fr_ = Mr_sp * qdot0 + h * (J_sp.transpose() * (fm - Mm_sp * Jdot_sp * qdot0) + fr); // check
		MDKr_sp = Mr_sp + J_sp.transpose() * (h * Dm_sp - hsquare * Km_sp) * J_sp + h * Dr_sp - hsquare * Kr_sp;
		/*sparse_to_file_as_dense(Mr_sp, "Mr_sp");
		sparse_to_file_as_dense(J_sp, "J_sp");
		sparse_to_file_as_dense(Jdot_sp, "Jdot_sp");
		sparse_to_file_as_dense(Km_sp, "Km_sp");
		sparse_to_file_as_dense(Dr_sp, "Dr_sp");
		sparse_to_file_as_dense(Kr_sp, "Kr_sp");
		sparse_to_file_as_dense(K_sp, "K_sp");
		sparse_to_file_as_dense(Mm_sp, "Mm_sp");
*/
		
		if (ne > 0) {
			constraint0->computeJacEqMSparse(Gm_, Gmdot_, gm, gmdot, gmddot);
			constraint0->computeJacEqRSparse(Gr_, Grdot_, gr, grdot, grddot);

			Gm_sp.setFromTriplets(Gm_.begin(), Gm_.end());
			Gmdot_sp.setFromTriplets(Gmdot_.begin(), Gmdot_.end());
			Gr_sp.setFromTriplets(Gr_.begin(), Gr_.end());
			Grdot_sp.setFromTriplets(Grdot_.begin(), Grdot_.end());

			//sparse_to_file_as_dense(Gm_sp * J_sp, "Gm_sp * J_sp");
			G_sp.topRows(nem) = Gm_sp * J_sp;
			G_sp.bottomRows(ner) = Gr_sp;
			
			//sparse_to_file_as_dense(G_sp, "G_sp");

			g.segment(0, nem) = gm;
			g.segment(nem, ner) = gr;
			rhsG = -gdot - 100.0 * g;// todo!!!!!
		}

		if (ni > 0) {
			// Check for active inequality constraint
			//constraint0->computeJacIneqMSparse(Cm, Cmdot, cm, cmdot, cmddot);
			//constraint0->computeJacIneqRSparse(Cr, Crdot, cr, crdot, crddot);
			constraint0->computeJacIneqM(Cm, Cmdot, cm, cmdot, cmddot);
			constraint0->computeJacIneqR(Cr, Crdot, cr, crdot, crddot);

			rowsR.clear();
			rowsM.clear();

			constraint0->getActiveList(rowsM, rowsR);
			nim = rowsM.size();
			nir = rowsR.size();
			ni = nim + nir;

			if (ni > 0) {
				Eigen::VectorXi m_rowsM = Eigen::Map<Eigen::VectorXi, Eigen::Unaligned>(rowsM.data(), rowsM.size());
				Eigen::VectorXi m_rowsR = Eigen::Map<Eigen::VectorXi, Eigen::Unaligned>(rowsR.data(), rowsR.size());

				MatrixXd m_Cm = Cm(m_rowsM, Eigen::placeholders::all);
				MatrixXd m_Cr = Cr(m_rowsR, Eigen::placeholders::all);
				VectorXd m_cm = cm(m_rowsM);
				VectorXd m_cr = cr(m_rowsR);
				VectorXd m_cmdot = cmdot(m_rowsM);
				VectorXd m_crdot = crdot(m_rowsR);

				MatrixXd CmJ = m_Cm * MatrixXd(J_sp);
				C.resize(CmJ.rows() + m_Cr.rows(), m_Cr.cols());
				C << CmJ, m_Cr;
				rhsC.resize(C.rows());
				VectorXd c(C.rows());
				c << m_cm, m_cr;
				VectorXd cdot(C.rows());
				cdot << m_cmdot, m_crdot;
				rhsC = -cdot - 5.0 * c;
			}
		}

		if (ne == 0 && ni == 0) {	// No constraints
			ConjugateGradient< SparseMatrix<double> > cg;
			cg.setMaxIterations(100);
			cg.setTolerance(1e-6);
			cg.compute(MDKr_sp);
			qdot1 = cg.solveWithGuess(fr_, qdot0);

		}
		else if (ne > 0 && ni == 0) {  // Just equality
			int rows = nr + ne;
			int cols = nr + ne;
			//cout << rows << endl;
			//cout << cols << endl;

			MatrixXd LHS(rows, cols);
			VectorXd rhs(rows);
			LHS.setZero();
			rhs.setZero();
			VectorXd guess = rhs;
			guess.segment(0, nr) = qdot0;

			MatrixXd MDKr_ = MatrixXd(MDKr_sp);
			MatrixXd G = MatrixXd(G_sp);

			LHS.block(0, 0, nr, nr) = MDKr_;
			LHS.block(0, nr, nr, ne) = G.transpose();
			LHS.block(nr, 0, ne, nr) = G;
			SparseMatrix<double> LHS_sp(rows, cols);
			LHS_sp = LHS.sparseView(1e-8);
			
			rhs.segment(0, nr) = fr_;
			rhs.segment(nr, ne) = rhsG;
			//cout << LHS << endl;
			//cout << rhs << endl;
		/*	VectorXd sol = LHS.ldlt().solve(rhs);
			qdot1 = sol.segment(0, nr);
			VectorXd l = sol.segment(nr, sol.rows() - nr);*/
			//cout << qdot1.segment<2>(0) << endl << endl;

			ConjugateGradient< SparseMatrix<double> > cg;
			cg.setMaxIterations(200);
			cg.setTolerance(1e-6);
			cg.compute(LHS_sp);
			qdot1 = cg.solveWithGuess(rhs, guess).segment(0, nr);

			//shared_ptr<QuadProgMosek> program_ = make_shared <QuadProgMosek>();
			//program_->setParamInt(MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
			//program_->setParamInt(MSK_IPAR_LOG, 10);
			//program_->setParamInt(MSK_IPAR_LOG_FILE, 1);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_DFEAS, 1e-8);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_INFEAS, 1e-10);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_MU_RED, 1e-8);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_NEAR_REL, 1e3);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_PFEAS, 1e-8);
			//program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_REL_GAP, 1e-8);
			//program_->setNumberOfVariables(nr);
			//program_->setObjectiveMatrix(MDKr_sp);

			//program_->setObjectiveVector(-fr_);

			//program_->setNumberOfEqualities(ne);
			//program_->setEqualityMatrix(G_sp);

			//program_->setEqualityVector(rhsG);


			//bool success = program_->solve();
			//VectorXd sol = program_->getPrimalSolution();
			//qdot1 = sol.segment(0, nr);
			//VectorXd l = program_->getDualEquality();
			//constraint0->scatterForceEqM(MatrixXd(Gm_sp.transpose()), l.segment(0, nem) / h);
			//constraint0->scatterForceEqR(MatrixXd(Gr_sp.transpose()), l.segment(nem, l.rows() - nem) / h);

		}
		else if (ne == 0 && ni > 0) {  // Just inequality
			shared_ptr<QuadProgMosek> program_ = make_shared <QuadProgMosek>();
			program_->setParamInt(MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
			program_->setParamInt(MSK_IPAR_LOG, 10);
			program_->setParamInt(MSK_IPAR_LOG_FILE, 1);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_DFEAS, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_INFEAS, 1e-10);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_MU_RED, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_NEAR_REL, 1e3);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_PFEAS, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_REL_GAP, 1e-8);

			program_->setNumberOfVariables(nr);
			program_->setObjectiveMatrix(MDKr_sp);
			program_->setObjectiveVector(-fr_);
			program_->setNumberOfInequalities(ni);
			program_->setInequalityMatrix(C.sparseView());

			VectorXd cvec(ni);
			cvec.setZero();
			program_->setInequalityVector(cvec);

			bool success = program_->solve();
			VectorXd sol = program_->getPrimalSolution();
			qdot1 = sol.segment(0, nr);
		}
		else {  // Both equality and inequality
			shared_ptr<QuadProgMosek> program_ = make_shared <QuadProgMosek>();
			program_->setParamInt(MSK_IPAR_OPTIMIZER, MSK_OPTIMIZER_INTPNT);
			program_->setParamInt(MSK_IPAR_LOG, 10);
			program_->setParamInt(MSK_IPAR_LOG_FILE, 1);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_DFEAS, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_INFEAS, 1e-10);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_MU_RED, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_NEAR_REL, 1e3);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_PFEAS, 1e-8);
			program_->setParamDouble(MSK_DPAR_INTPNT_QO_TOL_REL_GAP, 1e-8);
			program_->setNumberOfVariables(nr);

			program_->setObjectiveMatrix(MDKr_sp);
			program_->setObjectiveVector(-fr_);
			program_->setNumberOfInequalities(ni);
			program_->setInequalityMatrix(C.sparseView());
			program_->setNumberOfEqualities(ne);
			VectorXd cvec(ni);
			cvec.setZero();

			program_->setInequalityVector(cvec);
			program_->setEqualityMatrix(G_sp);

			VectorXd gvec(ne);
			gvec.setZero();
			program_->setEqualityVector(rhsG);

			bool success = program_->solve();
			VectorXd sol = program_->getPrimalSolution();
			qdot1 = sol.segment(0, nr);
		}
		
		qddot = (qdot1 - qdot0) / h;
		q1 = q0 + h * qdot1;
		yk.segment(0, nr) = q1;
		yk.segment(nr, nr) = qdot1;

		ydotk.segment(0, nr) = qdot1;
		ydotk.segment(nr, nr) = qddot;

		joint0->scatterDofs(yk, nr);
		joint0->scatterDDofs(ydotk, nr);

		deformable0->scatterDofs(yk, nr);
		deformable0->scatterDDofs(ydotk, nr);

		softbody0->scatterDofs(yk, nr);
		softbody0->scatterDDofs(ydotk, nr);

		meshembedding0->scatterDofs(yk, nr);
		meshembedding0->scatterDDofs(ydotk, nr);

		//Energy ener = m_world->computeEnergy();
		/*cout << "V" << ener.V << endl;
		cout << "K" << ener.K << endl;
		cout << " sum " << ener.V + ener.K << endl;*/
		step++;
		return yk;
	}
	break;

	case REDUCED_ODE45:
		break;
	case REDMAX_ODE45:
		break;
	default:
		break;
	}
}

