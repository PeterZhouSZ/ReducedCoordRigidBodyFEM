#include "SolverSparse.h"

#include <Eigen/PardisoSupport>
#include <functional>
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <omp.h>

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
//#include <unsupported/Eigen/src/IterativeSolvers/MINRES.h>
#include <unsupported\Eigen\src\IterativeSolvers\Scaling.h>
#include <unsupported\Eigen\src\IterativeSolvers\GMRES.h>

using namespace std;
using namespace Eigen;

void SolverSparse::initMatrix(int nm, int nr, int nem, int ner, int nim, int nir) {
	ni = nim + nir;
	int nre = nr + ne;

	fm.setZero();
	fr.setZero();
	fr_.setZero();
	tmp.setZero();
	
	Mr_sp.resize(nr, nr);
	//Mr_sp.data().squeeze();
	Mr_sp_temp.resize(nr, nr);

	//Mr_sp_temp.data().squeeze();

	MDKr_sp.resize(nr, nr);
	//MDKr_sp.data().squeeze();
	MDKr_sp_tp.resize(nr, nr);
	//MDKr_sp_tp.data().squeeze();
	lhs_left_tp.resize(nr, nre);
	//lhs_left_tp.data().squeeze();
	lhs_right_tp.resize(ne, nre);
	//lhs_right_tp.data().squeeze();
	lhs_left.resize(nre, nr);
	//lhs_left.data().squeeze();
	lhs_right.resize(nre, ne);
	//lhs_right.data().squeeze();
	LHS_sp.resize(nre, nre);
	//LHS_sp.data().squeeze();

	Kr_sp.resize(nr, nr);
	//Kr_sp.data().squeeze();
	Kr_.clear();

	Dr_sp.resize(nr, nr);
	//Dr_sp.data().squeeze();
	Dr_.clear();
	
	Dm_sp.resize(nm, nm);
	//Dm_sp.data().squeeze();
	Dm_.clear();

	K_sp.resize(nm, nm);
	//K_sp.data().squeeze();
	K_.clear();

	Km_sp.resize(nm, nm);
	//Km_sp.data().squeeze();
	Km_.clear();


	J_dense.setZero();
	Jdot_dense.setZero();

	J_sp.resize(nm, nr);
	J_t_sp.resize(nr, nm);
	//J_sp.data().squeeze();
	//J_.clear();

	Jdot_sp.resize(nm, nr);
	//Jdot_sp.data().squeeze();
	Jdot_.clear();
	
	//Gm_sp.resize(nem, nm);
	//Gm_sp.data().squeeze();
	Gm_.clear();

	//Gmdot_sp.resize(nem, nm);
	//Gmdot_sp.data().squeeze();
	Gmdot_.clear();

	gm.setZero();
	gmdot.setZero();
	gmddot.setZero();

	//Gr_sp.resize(ner, nr);
	//Gr_sp.data().squeeze();
	Gr_.clear();

	//Grdot_sp.resize(ner, nr);
	//Grdot_sp.data().squeeze();
	Grdot_.clear();

	gr.setZero();
	grdot.setZero();
	grddot.setZero();

	g.setZero();
	gdot.setZero();
	gddot.setZero();

	rhsG.setZero();
	rhs.setZero();
	guess.setZero();

	Cm_sp.resize(nim, nm);
	//Cm_sp.data().squeeze();
	Cm_.clear();

	Cmdot_sp.resize(nim, nm);
	//Cmdot_sp.data().squeeze();
	Cmdot_.clear();

	cm.resize(nim);
	cm.setZero();
	cmdot.resize(nim);
	cmdot.setZero();
	cmddot.resize(nim);
	cmddot.setZero();

	Cr_sp.resize(nir, nr);
	//Cr_sp.data().squeeze();
	Cr_.clear();

	Crdot_sp.resize(nir, nr);
	//Crdot_sp.data().squeeze();
	Crdot_.clear();

	cr.resize(nir);
	cr.setZero();
	crdot.resize(nir);
	crdot.setZero();
	crddot.resize(nir);
	crddot.setZero();

	//G_sp.resize(ne, nr);
	//G_sp.data().squeeze();
	//G_sp_tp.resize(nr, ne);
	//G_sp_tp.data().squeeze();

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
			isCollided = false;
			nr = m_world->nr;
			nm = m_world->nm;
			nR = m_world->nR;

			nem = m_world->nem;
			ner = m_world->ner;
			ne = ner + nem;

			nim = m_world->nim;
			nir = m_world->nir;
			ni = nim + nir;

			m_ntets = m_world->m_ntets;

			Mm_sp.resize(nm, nm);
			Mm_sp.data().squeeze();
			Mm_.clear();
			Mm_.reserve(nm);  
			
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
			rhs.resize(nr + ne);
			guess.resize(nr + ne);
			gdot.resize(ne);
			gddot.resize(ne);
			gm.resize(nem);
			gmdot.resize(nem);
			gmddot.resize(nem);
			Grdot_sp.resize(ner, nr);
			Gm_sp.resize(nem, nm);
			Gmdot_sp.resize(nem, nm);
			Gr_sp.resize(ner, nr);

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
			zero.resize(ne, ne);

			deformable0->computeJacobianSparse(J_);		
			softbody0->computeJacobianSparse(J_);
			meshembedding0->computeJacobianSparse(J_);
			J_vec_idx = J_.size();

			// Hyper Reduced 
			JmR.resize(nm, nR);
			JmRdot.resize(nm, nR);
			JrR.resize(nr, nR);
			JrRdot.resize(nr, nR);
			JrR.setZero();
			JrRdot.setZero();
			JrR_select.resize(nr, nR);
			JrR_select.setZero();
			joint0->computeHyperReducedJacobian(JrR, JrR_select);
			//cout << JrR << endl;
			//cout << JrR_select << endl;
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
		
		
		if (meshembedding0->getCoarseMesh()!= nullptr && meshembedding0->getCoarseMesh()->m_isCollided) {
			isCollided = true;
		}

		double t_i = m_world->getTime();
		switch (m_world->m_type) {
		case CROSS:
  			m_world->sceneCross(t_i);
			break;
		case SERIAL_CHAIN:
			m_world->sceneCross(t_i);
			break;
		case STARFISH:
			m_world->sceneStarFish3(t_i);
			//m_world->sceneStarFish2(t_i);
			//m_world->sceneStarFishJump(t_i);
			//m_world->sceneStarFish(t_i);
			break;
		case STARFISH_2:		
			m_world->sceneStarFish2(t_i);	
			break;
		case TEST_MAXIMAL_HYBRID_DYNAMICS:
			m_world->sceneTestMaximalHD(t_i);
			break;
		case FINGERS:
			m_world->sceneFingers(t_i);
			break;
		case STARFISH3:
			//m_world->sceneStarFish2(t_i);
			m_world->sceneStarFishJump(t_i);
			break;
		case TEST_HYPER_REDUCED_COORDS:
			m_world->sceneTestHyperReduced(t_i);
			break;
		case TEST_CONSTRAINT_PRESC_BODY_ATTACH_POINT:
			m_world->sceneAttachPoint(t_i);
		default:
			break;
		}

			
		body0->computeGrav(grav, fm);
		body0->computeForceDampingSparse(tmp, Dm_);
		deformable0->computeForce(grav, fm);
		deformable0->computeForceDampingSparse(grav, tmp, Dm_);

		softbody0->computeForce(grav, fm);
		softbody0->computeStiffnessSparse(K_);

		meshembedding0->computeForce(grav, fm);
		meshembedding0->computeForceDampingSparse(tmp, Dm_);
		meshembedding0->computeStiffnessSparse(K_);
		joint0->computeForceStiffnessSparse(fr, Kr_);
		joint0->computeForceDampingSparse(tmp, Dr_);

		//// First get dense jacobian (only a small part of the matrix)
		joint0->computeJacobian(J_dense, Jdot_dense);
	
		//// Push back the dense part
		if (step == 0) {
			for (int i = 0; i < J_dense.rows(); ++i) {
				for (int j = 0; j < J_dense.cols(); ++j) {
					J_.push_back(T(i, j, J_dense(i, j)));
					Jdot_.push_back(T(i, j, Jdot_dense(i, j)));
				}
			}
		}
		else {
			int idx = J_vec_idx;
			for (int i = 0; i < J_dense.rows(); ++i) {
				for (int j = 0; j < J_dense.cols(); ++j) {
					J_[idx] = (T(i, j, J_dense(i, j)));
					Jdot_.push_back(T(i, j, Jdot_dense(i, j)));
					idx++;
				}
			}
		}

		spring0->computeForceStiffnessDampingSparse(fm, Km_, Dm_);

		Km_sp.setFromTriplets(Km_.begin(), Km_.end());
		Dm_sp.setFromTriplets(Dm_.begin(), Dm_.end());
		Dr_sp.setFromTriplets(Dr_.begin(), Dr_.end());
		K_sp.setFromTriplets(K_.begin(), K_.end()); // check
		//cout << "K" << K_.size() << endl;

		Kr_sp.setFromTriplets(Kr_.begin(), Kr_.end());
		J_sp.setFromTriplets(J_.begin(), J_.end()); // check

		Jdot_sp.setFromTriplets(Jdot_.begin(), Jdot_.end());

		J_t_sp = J_sp.transpose();

		/*MatrixXd JrR;
		JrR.resize(2, 1);
		JrR << 1.0, 2.0;*/

		JmR = MatrixXd(J_sp * JrR);
		JmRdot = MatrixXd(Jdot_sp * JrR);

		Mr_sp = J_t_sp * (Mm_sp - hsquare * K_sp) * J_sp;
		
		//Mr_sp_temp = Mr_sp.transpose();
		//Mr_sp += Mr_sp_temp;
		//Mr_sp *= 0.5;

		fr_ = Mr_sp * qdot0 + h * (J_t_sp * (fm - Mm_sp * Jdot_sp * qdot0) + fr); 
		MDKr_sp = Mr_sp + J_t_sp * (h * Dm_sp - hsquare * Km_sp) * J_sp + h * Dr_sp - hsquare * Kr_sp;
		//cout << MatrixXd(MDKr_sp) << endl << endl;
		//cout << "Mr_sp"<< endl << MatrixXd(Mr_sp) << endl << endl;
		//cout << "J_sp" << endl << MatrixXd(J_sp) << endl << endl;
		//cout << "fr_"<< (fr_) << endl << endl;
		//cout <<"fm"<< fm << endl << endl;
		
		JmR = MatrixXd(J_sp * JrR);
		JmRdot = MatrixXd(Jdot_sp * JrR);

		Mr_sp = J_t_sp * (Mm_sp - hsquare * K_sp) * J_sp;
		MatrixXd JmR_t = JmR.transpose();
		MatrixXd MR = MatrixXd(JmR_t * (Mm_sp - hsquare * K_sp) * JmR);

		VectorXd fR_ = MR * JrR_select.transpose() * qdot0 + h * (JmR_t * (fm - Mm_sp * Jdot_sp * qdot0) + JrR_select.transpose() * fr);
		MatrixXd MDKR_ = MR + JmR_t * (h * Dm_sp - hsquare * Km_sp) * JmR;


		//Mr_sp_temp = Mr_sp.transpose();
		//Mr_sp += Mr_sp_temp;
		//Mr_sp *= 0.5;

		/*	MatrixXd Mrnew = MatrixXd(JmR.transpose() * (Mm_sp - hsquare * K_sp) * JmR);
		VectorXd fnew = fr.segment<1>(0);
		VectorXd qdot0new = qdot0.segment<1>(0);

		VectorXd f_new = Mrnew * qdot0new + h * (JMR.transpose() * (fm - Mm_sp * Jdot_sp * qdot0) + fnew);
		MatrixXd MDKr_new = Mrnew + JMR.transpose() * (h * Dm_sp - hsquare * Km_sp) * JMR;
		qdot1 = JrR * (MDKr_new.ldlt().solve(f_new));
		cout << qdot1 << endl;*/

		if (ne > 0) {
			constraint0->computeJacEqMSparse(Gm_, Gmdot_, gm, gmdot, gmddot);		
			constraint0->computeJacEqRSparse(Gr_, Grdot_, gr, grdot, grddot);
			
			rowsEM.clear();
			rowsER.clear();
			constraint0->getEqActiveList(rowsEM, rowsER);
			nem = rowsEM.size();
			ner = rowsER.size();
			ne = nem + ner;

			if (ne > 0) {
				Eigen::VectorXi m_rowsEM = Eigen::Map<Eigen::VectorXi, Eigen::Unaligned>(rowsEM.data(), rowsEM.size());
				Eigen::VectorXi m_rowsER = Eigen::Map<Eigen::VectorXi, Eigen::Unaligned>(rowsER.data(), rowsER.size());
				Gm_sp.setFromTriplets(Gm_.begin(), Gm_.end());
				Gmdot_sp.setFromTriplets(Gmdot_.begin(), Gmdot_.end());
				Gr_sp.setFromTriplets(Gr_.begin(), Gr_.end());
				Grdot_sp.setFromTriplets(Grdot_.begin(), Grdot_.end());

				MatrixXd m_Gm = MatrixXd(Gm_sp)(m_rowsEM, Eigen::placeholders::all);
				MatrixXd m_Gr = MatrixXd(Gr_sp)(m_rowsER, Eigen::placeholders::all);
				VectorXd m_gm = gm(m_rowsEM);
				VectorXd m_gr = gr(m_rowsER);
				VectorXd m_gmdot = gmdot(m_rowsEM);
				VectorXd m_grdot = grdot(m_rowsER);
				VectorXd m_gmddot = gmddot(m_rowsEM);
				VectorXd m_grddot = grddot(m_rowsER);
				MatrixXd GmJ = m_Gm * MatrixXd(J_sp);
				G.resize(GmJ.rows() + m_Gr.rows(), m_Gr.cols());
				///G_sp.resize(GmJ.rows() + m_Gr.rows(), m_Gr.cols());
				G << GmJ, m_Gr;
				//G_sp = G.sparseView();
				rhsG.resize(G.rows());
				VectorXd g(G.rows());
				g << m_gm, m_gr;
				VectorXd gdot(G.rows());
				gdot << m_gmdot, m_grdot;
				rhsG = -gdot - 5.0 * g;

				GR = G * JrR;
			}

			//Gm_sp.setFromTriplets(Gm_.begin(), Gm_.end());
			//Gmdot_sp.setFromTriplets(Gmdot_.begin(), Gmdot_.end());
			//Gr_sp.setFromTriplets(Gr_.begin(), Gr_.end());
			//Grdot_sp.setFromTriplets(Grdot_.begin(), Grdot_.end());

			//sparse_to_file_as_dense(Gm_sp * J_sp, "Gm_sp * J_sp");
			//G_sp.topRows(nem) = Gm_sp * J_sp;
			//G_sp.bottomRows(ner) = Gr_sp;
			
			//sparse_to_file_as_dense(G_sp, "G_sp");
			/*g.segment(0, nem) = gm;
			g.segment(nem, ner) = gr;
			gdot.segment(0, nem) = gmdot;
			gdot.segment(nem, ner) = grdot;
			gddot.segment(0, nem) = gmddot;
			gddot.segment(nem, ner) = grddot;
			rhsG = - gdot - 5.0 * g ;*/
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
			cg.setMaxIterations(100000);
			cg.setTolerance(1e-10);
			cg.compute(MDKr_sp);
			qdot1 = cg.solveWithGuess(fr_, qdot0);
			
			if (nR < nr) {
				qdot1 = JrR * (MDKR_.ldlt().solve(fR_));
			}

			//cout << qdot1 << endl;
		}
		else if (ne > 0 && ni == 0) {  // Just equality
			//int rows = nr + ne;
			//int cols = nr + ne;

			/*
			//Slow

			MatrixXd LHS(rows, cols);		
			LHS.setZero();
			
			MatrixXd MDKr_ = MatrixXd(MDKr_sp);
			MatrixXd G = MatrixXd(G_sp);

			LHS.block(0, 0, nr, nr) = MDKr_;
			LHS.block(0, nr, nr, ne) = G.transpose();
			LHS.block(nr, 0, ne, nr) = G;
			SparseMatrix<double> LHS_sp(rows, cols);
			LHS_sp = LHS.sparseView(1e-8);
			
			*/
			int nre = nr + ne;
			lhs_left_tp.resize(nr, nre);
			lhs_right_tp.resize(ne, nre);
			lhs_left.resize(nre, nr);
			lhs_right.resize(nre, ne);

			LHS_sp.resize(nre, nre);
			guess.segment(0, nr) = qdot0;
			SparseMatrix<double> Gtemp = G.sparseView();
			SparseMatrix<double> Gtemp_tp = Gtemp.transpose();
			// Assemble sparse matrices
			MDKr_sp_tp = MDKr_sp.transpose();
			//G_sp_tp = G_sp.transpose();

			// Combine MDKr' and G' by column
			lhs_left_tp.leftCols(nr) = MDKr_sp_tp;
			//lhs_left_tp.rightCols(ne) = G_sp_tp;
			lhs_left_tp.rightCols(ne) = Gtemp_tp;

			// Combine G and Z by column
			//lhs_right_tp.leftCols(nr) = G_sp;
			//lhs_right_tp.rightCols(ne) = zero;
			lhs_right_tp.leftCols(nr) = Gtemp;
			zero.resize(ne, ne);
			lhs_right_tp.rightCols(ne) = zero;


			lhs_left = lhs_left_tp.transpose();  // rows x nr
			lhs_right = lhs_right_tp.transpose(); // rows x ne

			LHS_sp.leftCols(nr) = lhs_left;
			LHS_sp.rightCols(ne) = lhs_right;
			
			rhs.resize(nre);
			rhs.segment(0, nr) = fr_;
			rhs.segment(nr, ne) = rhsG;


			//cout << MatrixXd(LHS_sp) << endl << endl;
			//cout << rhs << endl << endl;

			//VectorXd sol = LHS.ldlt().solve(rhs);
			//qdot1 = sol.segment(0, nr);
			//VectorXd l = sol.segment(nr, sol.rows() - nr);
			switch (m_sparse_solver)
			{
			case CG: 
				{
					ConjugateGradient< SparseMatrix<double>, Lower | Upper> cg;
					//cg.setMaxIterations(2000);
					cg.setTolerance(1e-3);
					cg.compute(LHS_sp);
					qdot1 = cg.solveWithGuess(rhs, guess).segment(0, nr);
					
					//std::cout << "#iterations:     " << cg.iterations() << std::endl;
					//std::cout << "estimated error: " << cg.error() << std::endl;
					break;
				}
			case CG_ILUT: 
				{
					ConjugateGradient< SparseMatrix<double>, Lower | Upper, IncompleteLUT<double>> cg;
					cg.preconditioner().setDroptol(0.01);
					cg.setMaxIterations(1000);
					cg.setTolerance(1e-3);
					cg.compute(LHS_sp);
					qdot1 = cg.solveWithGuess(rhs, guess).segment(0, nr);
					break;
				}	
			case MINRES_SOLVER:
				{					
					VectorXd diagAinv(nr);

					for (int j = 0; j< MDKr_sp.outerSize(); ++j)
					{
						typename SparseMatrix<double>::InnerIterator it(MDKr_sp, j);
						while (it && it.index() != j) ++it;
						if (it && it.index() == j && it.value() != 0.0)
							diagAinv(j) = 1.0 / it.value();
						else
							diagAinv(j) = 1.0;
					}

					SparseMatrix<double> B_sp = G_sp * diagAinv.asDiagonal() * G_sp_tp;
					
					//Eigen::SimplicialLDLT< Eigen::SparseMatrix<double, Eigen::ColMajor>> pre_solver;
					//pre_solver.compute(B_sp);
					//SparseMatrix<double> I_sp(ne, ne);
					//I_sp.setIdentity();
					//SparseMatrix<double> D_sp = pre_solver.solve(I_sp);
					if (step == 0) {
						D_sp.reserve(5000);
					}
					D_sp = MatrixXd(B_sp).inverse().sparseView();
					D_sp.makeCompressed();
					//MatrixXd A = diagA.asDiagonal();
					//MatrixXd P(nr + ne, nr + ne);
					//P.setZero();
					//P.block(0, 0, nr, nr) = A;
					//P.block(nr, nr, ne, ne) = B;

					//MINRES<SparseMatrix<double>, Lower, SaddlePointPreconditioner<double> > mr;
					mr.setMaxIterations(1000);
					mr.setTolerance(1e-6);
					mr.compute(LHS_sp);					

					mr.preconditioner().setADiagMatrix(diagAinv);
					
					mr.preconditioner().setDMatrix(D_sp);


					if (step == 0) {
						//mr.preconditioner().precompute();
					}
					//mr.preconditioner().factor();

					qdot1 = mr.solve(rhs).segment(0, nr);

					//std::cout << "#iterations:     " << mr.iterations() << std::endl;
					//std::cout << "estimated error: " << mr.error() << std::endl;
	
					/*for (int i = 1; i < 201; i++) {
						mr.setMaxIterations(i*50);
						qdot1 = mr.solve(rhs).segment(0, nr);
						error_vec(i - 1) = (qdot1 - x_exact).norm();
						std::cout << "#iterations:     " << mr.iterations() << std::endl;
						std::cout << "estimated error: " << mr.error() << std::endl;
					}*/

					break;
				}
				
			case GMRES_SOLVER:
				{
					GMRES<SparseMatrix<double>> gm;
					gm.compute(LHS_sp);
					gm.setTolerance(1e-3);
					qdot1 = gm.solveWithGuess(rhs, guess).segment(0, nr);
					break;
				}
			case BICG:
				{
					BiCGSTAB<SparseMatrix<double> >  BCGST;
					BCGST.compute(LHS_sp);
					BCGST.setTolerance(1e-3);
					qdot1 = BCGST.solveWithGuess(rhs, guess).segment(0, nr);
					break;
				}
			case BICG_ILUT: 
				{
					BiCGSTAB<SparseMatrix<double>, IncompleteLUT<double>>  BCGST;
					BCGST.preconditioner().setDroptol(0.001);
					BCGST.compute(LHS_sp);
					BCGST.setTolerance(1e-3);
					qdot1 = BCGST.solveWithGuess(rhs, guess).segment(0, nr);
					break;
				}
			case SLDLT:
				{
					SimplicialLDLT<SparseMatrix<double>, Lower, NaturalOrdering<int> > sldlt;

					sldlt.compute(LHS_sp);
					qdot1 = sldlt.solve(rhs).segment(0, nr);
					break;
				}			
			case LU: 
				{
					LHS_sp.makeCompressed();
					if (step == 0) {
						solver.analyzePattern(LHS_sp);
					}

					solver.factorize(LHS_sp);
					qdot1 = solver.solve(rhs).segment(0, nr);
					break;
				}		
			case PARDISO_LU:
				{
					PardisoLU<Eigen::SparseMatrix<double>> solver;
					solver.compute(LHS_sp);
					qdot1 = solver.solve(rhs).segment(0, nr);

					if (nR < nr) {
						MatrixXd LHS_hr(nR + ne, nR + ne);
						LHS_hr.setZero();
						LHS_hr.block(0, 0, nR, nR) = MDKR_;
						LHS_hr.block(nR, 0, ne, nR) = GR;
						LHS_hr.block(0, nR, nR, ne) = GR.transpose();

						VectorXd rhs_hr(nR + ne);
						rhs_hr.setZero();

						rhs_hr.segment(0, nR) = fR_;
						rhs_hr.segment(nR, ne) = rhsG;
						PardisoLU<Eigen::SparseMatrix<double>> solver;
						solver.compute(LHS_hr.sparseView());
						qdot1 = JrR * solver.solve(rhs_hr).segment(0, nR);

					}

				}
				break;
			case PARDISO_LDLT:
				{
					PardisoLDLT<SparseMatrix<double>> pldlt;
					pldlt.compute(LHS_sp);
					qdot1 = pldlt.solve(rhs).segment(0, nr);
					break;
				}
			case QR:
				{
					SparseQR< SparseMatrix<double>, COLAMDOrdering<int>> sqr(LHS_sp);
					assert(sqr.info() == Success);
					qdot1 = sqr.solve(rhs).segment(0, nr);
					break;
				}
			default:
				{
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

					program_->setNumberOfEqualities(ne);
					program_->setEqualityMatrix(G_sp);

					program_->setEqualityVector(rhsG);

					bool success = program_->solve();
					VectorXd sol = program_->getPrimalSolution();
					qdot1 = sol.segment(0, nr);	
					VectorXd l = program_->getDualEquality();
					constraint0->scatterForceEqM(MatrixXd(Gm_sp.transpose()), l.segment(0, nem) / h);
					constraint0->scatterForceEqR(MatrixXd(Gr_sp.transpose()), l.segment(nem, l.rows() - nem) / h);
				}
				break;
			}

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
		//cout << qdot1 << endl << endl;
		qddot = (qdot1 - qdot0) / h;
		q1 = q0 + h * qdot1;
		yk.segment(0, nr) = q1;
		yk.segment(nr, nr) = qdot1;

		ydotk.segment(0, nr) = qdot1;
		ydotk.segment(nr, nr) = qddot;

		joint0->scatterDofs(yk, nr);
		joint0->scatterDDofs(ydotk, nr);
		joint0->reparam();
		joint0->gatherDofs(yk, nr);


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

