/** Synchron generator (EMT)
*
* @file
* @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
* @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
*
* DPsim
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************/

#pragma once

#include "Base_SynchronGenerator.h"
#include "EMT_VoltageSource_Ideal.h"

namespace DPsim {
namespace Components {
namespace EMT {

	/// Synchronous generator model
	/// If parInPerUnit is not set, the parameters have to be given with their respective stator or rotor
	/// referred values. The calculation to per unit is performed in the initialization.
	/// The case where parInPerUnit is not set will be implemented later.
	/// parameter names include underscores and typical variables names found in literature instead of
	/// descriptive names in order to shorten formulas and increase the readability

	class SynchronGeneratorSimplifiedCurrentSource : public SynchronGeneratorBase {

	protected:

		//##### Standard parameters #######
		Real mXd;
		Real mXq;
		Real mXd_t;
		Real mXq_t;
		Real mTd0_t;
		Real mTq0_t;
		Real mEq_t;
		Real mEd_t;
		Real mEf;

		/// Determinant of Ld (inductance matrix of d axis)
		Real detLd;
		/// Determinant of Lq (inductance matrix of q axis)
		Real detLq;

		/// voltage vector q d 0 kq1 kq2 df kd
		Matrix mVoltages;
		/// flux linkage vector
		Matrix mFluxes;
		/// current vector
		Matrix mCurrents;

		/// Interface voltage phase a
		Real mVa;
		/// Interface voltage phase b
		Real mVb;
		/// Interface voltage phase c
		Real mVc;

		/// Interface curent phase a
		Real mIa;
		/// Interface curent phase b
		Real mIb;
		/// Interface curent phase c
		Real mIc;

		// ### Useful Matrices ###
		/// reactance matrix
		Matrix mReactanceMat;
		/// omega - flux matrix
		Matrix mOmegaFluxMat;
		/// matrix for reversing stator current directions in calculations with respect to other currents
		Matrix mReverseCurrents;

		Components::EMT::VoltageSourceIdeal va;
		Components::EMT::VoltageSourceIdeal vb;
		Components::EMT::VoltageSourceIdeal vc;

	public:
		~SynchronGeneratorSimplifiedCurrentSource();

		/// Initializes the per unit or stator referred machine parameters with the machine parameters given in per unit or
		/// stator referred parameters depending on the setting of parameter type.
		/// The initialization mode depends on the setting of state type.
		SynchronGeneratorSimplifiedCurrentSource(String name, Int node1, Int node2, Int node3,
			Real nomPower, Real nomVolt, Real nomFreq, Int poleNumber, Real nomFieldCur,
			Real Rs, Real Ll, Real Lmd, Real Lmd0, Real Lmq, Real Lmq0,
			Real Rfd, Real Llfd, Real Rkd, Real Llkd,
			Real Rkq1, Real Llkq1, Real Rkq2, Real Llkq2,
			Real inertia, Logger::Level logLevel = Logger::Level::NONE);

		/// Initializes states in per unit or stator referred variables depending on the setting of the state type.
		/// Function parameters have to be given in real units.
		void initialize(Real om, Real dt,
			Real initActivePower, Real initReactivePower, Real initTerminalVolt,
			Real initVoltAngle, Real initFieldVoltage, Real initMechPower);

		/// Performs an Euler forward step with the state space model of a synchronous generator
		/// to calculate the flux and current from the voltage vector.
		void step(SystemModel& system, Real time);

		/// Performs an Euler forward step with the state space model of a synchronous generator
		/// to calculate the flux and current from the voltage vector in per unit.
		void stepInPerUnit(Real om, Real dt, Real time, NumericalMethod numMethod);

		/// Retrieves calculated voltage from simulation for next step
		void postStep(SystemModel& system);

		/// Park transform as described in Krause
		//Matrix parkTransform(Real theta, Matrix& in);
		Matrix parkTransform2(Real theta, Real a, Real b, Real c);

		/// Inverse Park transform as described in Krause
		//Matrix inverseParkTransform(Real theta, Matrix& in);
		Matrix inverseParkTransform2(Real theta, Real d, Real q, Real zero);

		Matrix& getVoltages() { return mVoltages; }
		Matrix& getCurrents() { return mCurrents; }
		Matrix& getFluxes() { return mFluxes; }
		Real getElectricalTorque() { return mElecTorque*mBase_T; }
		Real getRotationalSpeed() { return mOmMech*mBase_OmMech; }
		Real getRotorPosition() { return mThetaMech; }

		// Methods for network integrated components
		void initialize(SystemModel& system) { }
		void applySystemMatrixStamp(SystemModel& system);
		void applyRightSideVectorStamp(SystemModel& system);
	};
}
}
}