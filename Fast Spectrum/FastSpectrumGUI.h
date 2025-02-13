#pragma once
#ifndef FAST_SPECTRUM_GUI
#define FAST_SPECTRUM_GUI

#include <imgui/imgui.h>
#include <imgui/imgui.cpp>
#include <imgui/imgui_demo.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui_impl_glfw_gl3.h>

#include <igl/igl_inline.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/ViewerPlugin.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>

#include "FastSpectrum.h"

int					basisToShow		= 0;
int					eigToShow		= 0;
bool				boolShowSamples = false;
static int			numOfSample		= 1000;
static int			sampleType		= Sample_Poisson_Disk; 
static int			dataToShow		= 0;

void showMenu(igl::opengl::glfw::Viewer &viewer, igl::opengl::glfw::imgui::ImGuiMenu &menu, FastSpectrum &fastSpectrum) {
	// MAIN WINDOW 	
	{
		// Variable for Processing
		Eigen::MatrixXd redEigVects, V = viewer.data().V;
		Eigen::VectorXd redEigVals;
		Eigen::MatrixXi F = viewer.data().F;

		// Variables For Viewer visualization
		float	menuWindowLeft		= 0.f,
				menuWindowWidth		= 200.f,
				menuWindowHeight	= 475.f;
		
		Eigen::MatrixXd		vColor;
		Eigen::VectorXd		Z;

		ImGui::SetNextWindowPos(ImVec2(5+menuWindowLeft * menu.menu_scaling(), 5), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(menuWindowWidth, menuWindowHeight), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Fast Spectrum##2", nullptr, ImGuiWindowFlags_NoSavedSettings);

		float w = ImGui::GetContentRegionAvailWidth();
		float p = ImGui::GetStyle().FramePadding.x;

		// Main Algorithm
		if (ImGui::CollapsingHeader("Executing Algorithm", ImGuiTreeNodeFlags_DefaultOpen)){
			if (ImGui::Button("Load##Mesh", ImVec2((w) / 2.f, 0))){
				// Load mesh + display the correct mesh
				int numOfModel = viewer.data_list.size();
				viewer.open_dialog_load_mesh();
				if (viewer.data_list.size() > numOfModel) {
					viewer.data() = viewer.data_list.at(++dataToShow);
					viewer.data_list.at(dataToShow - 1).clear();
				}

				V.resize(viewer.data().V.rows(), viewer.data().V.cols());
				F.resize(viewer.data().F.rows(), viewer.data().F.cols());
				V = viewer.data().V;
				F = viewer.data().F;	
				fastSpectrum.setV(V);
				fastSpectrum.setF(F);

				viewer.core.align_camera_center(V, F);

				viewer.data().show_lines = false;
				printf("A new mesh with %d vertices (%d faces).\n", V.rows(), F.rows());
				viewer.data().set_mesh(V, F);
				//for (int i = 0; i < viewer.data_list.size(); i++) {
				//	printf("Data %d: %d (%d) \n", i, viewer.data_list.at(i).V.rows(), dataToShow);
				//}
			}
			ImGui::SameLine(0, p);
			if (ImGui::Button("Save##Mesh", ImVec2((w - p) / 2.f, 0))){
				viewer.open_dialog_save_mesh();
			}
			if (ImGui::Button("Run Algorithm", ImVec2((w), 30))){
				Eigen::SparseMatrix<double>	U;
				fastSpectrum.computeEigenPairs( V, F, numOfSample, (SamplingType) sampleType, U, redEigVects, redEigVals);
				
				// Show the first non-zero eigenvectors
				eigToShow					= 1;
				Z							= U * redEigVects.col(eigToShow);
				igl::jet(Z, true, vColor);
				viewer.data().set_colors(vColor);
			}
			
			ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

			/* STEP BY STEP CONSTRUCTION */
			if (ImGui::CollapsingHeader("Program Step by Step")) {
				/* Setup Laplace-Beltrami Matrix */
				if (ImGui::Button("[1] Construct Laplacian Matrix", ImVec2(w, 0))) {
					fastSpectrum.constructLaplacianMatrix();
				}
				/* For Sampling */
				if (ImGui::Button("[2] Sampling", ImVec2(w, 0))) {
					Eigen::SparseMatrix<double> M;
					fastSpectrum.getMassMatrix(M);
					fastSpectrum.getV(V);
					if (M.rows() != V.rows() || V.rows() < 1) {
						cout << "Error! Make sure you construct Laplacian matrices before creating samples." << endl;
						return;
					}

					if (numOfSample < 1) {
						cout << "Error! Sample cannot be smaller than 1. The program will assign 1000 as the default sampling." << endl;
						numOfSample = 1000;
					}
					else if (numOfSample > V.rows()) {
						cout << "Error! Sample cannot be larger than the number of vertices in the mesh. The program will assign 1000 as the default sampling.\n" << endl;
						numOfSample = 1000;
					}

					fastSpectrum.setSample(numOfSample, (SamplingType)sampleType);
					fastSpectrum.constructSample();
				}
				ImGui::RadioButton("Poisson Disk", &sampleType, Sample_Poisson_Disk); ImGui::SameLine();
				ImGui::RadioButton("Farthest Point", &sampleType, Sample_Farthest_Point);
				//ImGui::RadioButton("Random", &sampleType, Sample_Random);

				ImGui::Text("Number of sample"); ImGui::SameLine();
				static char numSample[6] = "1000"; ImGui::InputText("", numSample, 6);
				numOfSample = atoi(numSample);

				ImGui::Spacing(); ImGui::Spacing();

				/* For Basis */
				if (ImGui::Button("[3] Construct Basis", ImVec2(w, 0))) {
					Eigen::SparseMatrix<double> M;
					Eigen::VectorXi samples;
					int numSample;
					double avgEdgeLen;
					fastSpectrum.getMassMatrix(M);
					fastSpectrum.getSamples(samples);
					fastSpectrum.getSampleSize(numSample);
					fastSpectrum.getAvgEdgeLength(avgEdgeLen);
					if (numSample < 1 || V.rows() != M.rows() || samples.maxCoeff() >= V.rows() || !(avgEdgeLen > 0.0)) {
						cout << "Error! Make sure you execute previous steps before constructing basis functions." << endl;
						return;
					}

					fastSpectrum.constructBasis();
				}
				/* For Low-Dim Problem */
				if (ImGui::Button("[4] Construct Low-Dim Eigenproblem", ImVec2(w, 0))) {
					Eigen::SparseMatrix<double> Basis;
					int numSample;
					fastSpectrum.getFunctionBasis(Basis);
					fastSpectrum.getSampleSize(numSample);
					if (Basis.cols() != numSample || Basis.rows()!=V.rows() || !(Basis.cols()>0)) {
						cout << "Error! Make sure you execute previous steps before constructing Low-Dim Eigenproblem." << endl;
						return;
					}

					fastSpectrum.constructRestrictedProblem();
				}
				/* Solving Low-Dim Eigenproblem */
				if (ImGui::Button("[5] Solve Low-Dim Eigenproblem", ImVec2(w, 0))) {
					Eigen::SparseMatrix<double> MBar, Basis;
					fastSpectrum.getReducedMassMatrix(MBar);
					fastSpectrum.getFunctionBasis(Basis);
					if (Basis.cols() != MBar.cols() || MBar.cols()<1) {
						cout << "Error! Make sure you execute previous steps before solving the Low-Dim Eigenproblem." << endl;
						return;
					}

					fastSpectrum.solveRestrictedProblem();

					// Show the first non-zero eigenvectors
					Eigen::SparseMatrix<double>	U;
					fastSpectrum.getFunctionBasis(U);
					fastSpectrum.getReducedEigVects(redEigVects);
					printf("Approximated (lifted) eigenvectors %dx%d\n", U.rows(), redEigVects.cols());
					eigToShow					= 1;
					Z							= U * redEigVects.col(eigToShow);
					igl::jet(Z, true, vColor);
					viewer.data().set_colors(vColor);
				}
			}
		}
				
		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		/* VISUALIZATION WINDOW */
		if (ImGui::CollapsingHeader("Visualization##FastSpectrum", ImGuiTreeNodeFlags_DefaultOpen)){
			/* For Sampling */
			if (ImGui::Button("Show Samples", ImVec2((w), 0))) {
				Eigen::VectorXi Sample;
				fastSpectrum.getSamples(Sample);
				boolShowSamples = !boolShowSamples;

				if (Sample.maxCoeff() > V.rows()) {
					cout << "Error! Make sure you compute new samples before viewing them." << endl;
					return;
				}

				if (boolShowSamples) {	
					for (int i = 0; i < Sample.size(); i++) {
						viewer.data().add_points(V.row(Sample(i)), Eigen::RowVector3d(0.7, 0.1, 0.1));
					}
				}
				else {
					viewer.data().clear();
					fastSpectrum.getV(V);
					fastSpectrum.getF(F);
					viewer.data().set_mesh(V, F);
				}

				viewer.data().point_size = 5.0f;
			}

			/* For Basis */			
			ImGui::Text("Show Basis ");
			ImGui::SameLine(0, p);
			if (ImGui::Button("- 1##Basis", ImVec2((w - 85.0f - 5.0*p) /4.f, 0))) {
				Eigen::SparseMatrix<double> U;
				fastSpectrum.getFunctionBasis(U);

				if (U.rows() != V.rows()) {
					cout << "Error! Make sure you have computed basis function on the current model." << endl;
					return;
				}

				if (basisToShow > 0)
					basisToShow--;

				if (boolShowSamples) {
					boolShowSamples = false;
					viewer.data().clear();
					viewer.data().set_mesh(V, F);
				}

				Z = U.col(basisToShow);
				igl::jet(Z, true, vColor);
				viewer.data().set_colors(vColor);
			}
			ImGui::SameLine(0, p);	
			ImGui::Text("[%d]", basisToShow);
			ImGui::SameLine(0, p);

			if (ImGui::Button("+ 1##Basis", ImVec2((w - 85.0f - 5.0*p) / 4.f, 0))) {
				Eigen::SparseMatrix<double> U;
				fastSpectrum.getFunctionBasis(U);

				if (U.rows() != V.rows()) {
					cout << "Error! Make sure you have computed basis function on the current model." << endl;
					return;
				}

				if (basisToShow < U.cols())
					basisToShow++;

				if (boolShowSamples) {
					boolShowSamples = false;
					viewer.data().clear();
					viewer.data().set_mesh(V, F);
				}

				Z = U.col(basisToShow);
				igl::jet(Z, true, vColor);
				viewer.data().set_colors(vColor);
			}
			ImGui::SameLine(0, p);
			if (ImGui::Button("Rand", ImVec2((w - 85.0f - 5.0*p)*2/4.f, 0))) {
				Eigen::SparseMatrix<double> U;
				fastSpectrum.getFunctionBasis(U);

				if (U.rows() != V.rows()) {
					cout << "Error! Make sure you have computed basis function on the current model." << endl;
					return;
				}
				
				basisToShow = rand() % U.cols();

				if (boolShowSamples) {
					boolShowSamples = false;
					viewer.data().clear();
					viewer.data().set_mesh(V, F);
				}

				Z = U.col(basisToShow);
				igl::jet(Z, true, vColor);
				viewer.data().set_colors(vColor);
			}
			/* For Eigenvectors */
			ImGui::Text("Show EigVec");
			ImGui::SameLine(0, p);
			if (ImGui::Button("- 1##EigVec1", ImVec2((w-90.0f-3*p)/2.f, 0))) {
				fastSpectrum.getReducedEigVects(redEigVects);
				Eigen::SparseMatrix<double> U;
				fastSpectrum.getFunctionBasis(U);

				if (U.rows() != V.rows()) {
					cout << "Error! Make sure you have computed the eigenfunctions on the current model." << endl;
					return;
				}

				if (eigToShow > 0)
					eigToShow--;

				Z = U * redEigVects.col(eigToShow);
				if(eigToShow>0)
					igl::jet(Z, true, vColor);
				else 
					igl::jet(Z, false, vColor);		// for zero-th eigenvector, it doesn't make sense to scale, since they're supposed to be constant
				viewer.data().set_colors(vColor);
			}
			ImGui::SameLine(0, p);
			ImGui::Text("[%d]", eigToShow);
			ImGui::SameLine(0, p);

			if (ImGui::Button("+ 1##EigVec1", ImVec2((w-90.0f-3*p)/2.f, 0))) {
				fastSpectrum.getReducedEigVects(redEigVects);
				Eigen::SparseMatrix<double> U;
				fastSpectrum.getFunctionBasis(U);
				
				if (U.rows() != V.rows()) {
					cout << "Error! Make sure you have computed the eigenfunctions on the current model." << endl;
					return;
				}

				if (eigToShow < redEigVects.cols())
					eigToShow++;

				Z = U * redEigVects.col(eigToShow);
				igl::jet(Z, true, vColor);
				viewer.data().set_colors(vColor);
			}

			/* Overlays */
			ImGui::Checkbox("Wireframe", &(viewer.data().show_lines));
			ImGui::Checkbox("Fill Faces", &(viewer.data().show_faces));
		}

		ImGui::End();
	}
}

#endif // !FAST_SPECTRUM_GUI

