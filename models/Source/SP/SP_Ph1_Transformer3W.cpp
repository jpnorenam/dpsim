/* Copyright 2017-2020 Institute for Automation of Complex Power Systems,
 *                     EONERC, RWTH Aachen University
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *********************************************************************************/

#include <cps/SP/SP_Ph1_Transformer3W.h>

using namespace CPS;

// #### General ####
SP::Ph1::Transformer3W::Transformer3W(String uid, String name, Logger::Level logLevel, Bool withResistiveLosses)
	: SimPowerComp<Complex>(uid, name, logLevel) {
	if (withResistiveLosses)
		setVirtualNodeNumber(4);
	else
		setVirtualNodeNumber(3);

	mSLog->info("Create {} {}", this->type(), name);
	mIntfVoltage = MatrixComp::Zero(3, 1);
	mIntfCurrent = MatrixComp::Zero(3, 1);
	setTerminalNumber(3);

	addAttribute<Real>("nominal_voltage_end1", &mNominalVoltageEnd1, Flags::read | Flags::write);
	addAttribute<Real>("nominal_voltage_end2", &mNominalVoltageEnd2, Flags::read | Flags::write);
	addAttribute<Real>("nominal_voltage_end3", &mNominalVoltageEnd3, Flags::read | Flags::write);
	addAttribute<Real>("base_voltage", &mBaseVoltage, Flags::read | Flags::write);
	addAttribute<Real>("S1", &mRatedPower1, Flags::write | Flags::read);
	addAttribute<Real>("S2", &mRatedPower2, Flags::write | Flags::read);
	addAttribute<Real>("S3", &mRatedPower3, Flags::write | Flags::read);
	addAttribute<Complex>("ratio1", &mRatio1, Flags::write | Flags::read);
	addAttribute<Complex>("ratio2", &mRatio2, Flags::write | Flags::read);
	addAttribute<Complex>("ratio3", &mRatio3, Flags::write | Flags::read);
	addAttribute<Real>("R1", &mResistance1, Flags::write | Flags::read);
	addAttribute<Real>("R2", &mResistance2, Flags::write | Flags::read);
	addAttribute<Real>("R3", &mResistance3, Flags::write | Flags::read);
	addAttribute<Real>("L1", &mInductance1, Flags::write | Flags::read);
	addAttribute<Real>("L2", &mInductance2, Flags::write | Flags::read);
	addAttribute<Real>("L3", &mInductance3, Flags::write | Flags::read);
	addAttribute<Bool>("nodal_injection_stored", &mStoreNodalPowerInjection, Flags::read | Flags::write);
	addAttribute<Real>("p_inj", &mActivePowerInjection, Flags::read | Flags::write);
	addAttribute<Real>("q_inj", &mReactivePowerInjection, Flags::read | Flags::write);
	addAttribute<Complex>("current", &mCurrent(0), Flags::read | Flags::write);
	addAttribute<Complex>("current_1", &mCurrent(1), Flags::read | Flags::write);
	addAttribute<Complex>("current_2", &mCurrent(2), Flags::read | Flags::write);
	addAttribute<Real>("p_branch", &mActivePowerBranch(0), Flags::read | Flags::write);
	addAttribute<Real>("q_branch", &mReactivePowerBranch(0), Flags::read | Flags::write);
	addAttribute<Real>("p_branch_1", &mActivePowerBranch(1), Flags::read | Flags::write);
	addAttribute<Real>("q_branch_1", &mReactivePowerBranch(1), Flags::read | Flags::write);
	addAttribute<Real>("p_branch_2", &mActivePowerBranch(2), Flags::read | Flags::write);
	addAttribute<Real>("q_branch_2", &mReactivePowerBranch(2), Flags::read | Flags::write);
}


void SP::Ph1::Transformer3W::setParameters(
		Real nomVoltageEnd1, Real nomVoltageEnd2, Real nomVoltageEnd3, 
		Real ratioAbs1, Real ratioAbs2, Real ratioAbs3, 
		Real ratioPhase1, Real ratioPhase2, Real ratioPhase3, 
		Real resistance1, Real resistance2, Real resistance3,
		Real inductance1, Real inductance2, Real inductance3
	) {

	// Note: to be consistent impedance values must be referred to high voltage side (and base voltage set to higher voltage)
	Base::Ph1::Transformer3W::setParameters(
			nomVoltageEnd1, nomVoltageEnd2, nomVoltageEnd3, 
			ratioAbs1, ratioAbs2, ratioAbs3, 
			ratioPhase1, ratioPhase2, ratioPhase3,
			resistance1, resistance2, resistance3, 
			inductance1, inductance2, inductance3
		);
	
	mSLog->info("Nominal Voltage End 1={} [V] Nominal Voltage End 2={} [V] Nominal Voltage End 2={} [V]", 
				mNominalVoltageEnd1, mNominalVoltageEnd2, mNominalVoltageEnd3);
	mSLog->info("Resistance={} [Ohm] Inductance={} [H] (primary side)", mResistance1, mInductance1);
	mSLog->info("Resistance={} [Ohm] Inductance={} [H] (secondary side)", mResistance2, mInductance2);
	mSLog->info("Resistance={} [Ohm] Inductance={} [H] (tetrary side)", mResistance3, mInductance3);
    mSLog->info("Tap Ratio 1={} [/] Phase Shift 1={} [deg]", std::abs(mRatio1), std::arg(mRatio1));
	mSLog->info("Tap Ratio 2={} [/] Phase Shift 2={} [deg]", std::abs(mRatio2), std::arg(mRatio2));
	mSLog->info("Tap Ratio 3={} [/] Phase Shift 3={} [deg]", std::abs(mRatio3), std::arg(mRatio3));

	mRatioAbs1 = std::abs(mRatio1);
	mRatioAbs2 = std::abs(mRatio2);
	mRatioAbs3 = std::abs(mRatio3);
	mRatioPhase1 = std::arg(mRatio1);
	mRatioPhase2 = std::arg(mRatio2);
	mRatioPhase3 = std::arg(mRatio3);
	mConductance1 = 1 / mResistance1;
	mConductance2 = 1 / mResistance2;
	mConductance3 = 1 / mResistance3;

	mParametersSet = true;
}

void SP::Ph1::Transformer3W::setParameters(
	Real nomVoltageEnd1, Real nomVoltageEnd2, Real nomVoltageEnd3,
	Real ratedPower1, Real ratedPower2, Real ratedPower3,
	Real ratioAbs1, Real ratioAbs2, Real ratioAbs3, 
	Real ratioPhase1, Real ratioPhase2, Real ratioPhase3, 
	Real resistance1, Real resistance2, Real resistance3,
	Real inductance1, Real inductance2, Real inductance3
	) {

	mRatedPower1 = ratedPower1;
	mRatedPower2 = ratedPower2;
	mRatedPower3 = ratedPower3;
	mSLog->info("Rated Power ={} [W]", mRatedPower1);
	mSLog->info("Rated Power ={} [W]", mRatedPower2);
	mSLog->info("Rated Power ={} [W]", mRatedPower3);

	SP::Ph1::Transformer3W::setParameters(
		nomVoltageEnd1, nomVoltageEnd2, nomVoltageEnd3, 
		ratioAbs1, ratioAbs2, ratioAbs3, 
		ratioPhase1, ratioPhase2, ratioPhase3, 
		resistance1, resistance2, resistance3,
		inductance1, inductance2, inductance3);
}


SimPowerComp<Complex>::Ptr SP::Ph1::Transformer3W::clone(String name) {
	auto copy = Transformer3W::make(name, mLogLevel);
	copy->setParameters(
			mNominalVoltageEnd1, mNominalVoltageEnd2, mNominalVoltageEnd3,
			mRatedPower1, mRatedPower2, mRatedPower3,
			std::abs(mRatio1), std::abs(mRatio2), std::abs(mRatio3), 
			std::arg(mRatio1), std::arg(mRatio2), std::arg(mRatio3), 
			mResistance1, mResistance2, mResistance3,
			mInductance1, mInductance2, mInductance3
		);
	return copy;
}

void SP::Ph1::Transformer3W::initializeFromNodesAndTerminals(Real frequency) {
	mNominalOmega = 2. * PI * frequency;
	mReactance1 = mNominalOmega * mInductance1;
	mReactance2 = mNominalOmega * mInductance2;
	mReactance3 = mNominalOmega * mInductance3;
	mSLog->info("Reactance={} [Ohm] (primary side)", mReactance1);
	mSLog->info("Reactance={} [Ohm] (secondary side)", mReactance2);
	mSLog->info("Reactance={} [Ohm] (tetrary side)", mReactance3);


	// Component parameters are referred to high voltage side.
	// Switch terminals if transformer is connected the other way around.
	// Without modifying the mNominalVoltageEnd value.
	if (mNominalVoltageEnd1 < mNominalVoltageEnd2 && 
		mNominalVoltageEnd1 < mNominalVoltageEnd3) {
		std::shared_ptr<SimTerminal<Complex>> tmp = mTerminals[0];
		if (mNominalVoltageEnd2 > mNominalVoltageEnd3) {
			mTerminals[0] = mTerminals[1];
			mTerminals[1] = mTerminals[2];
		}
		else {
			mTerminals[0] = mTerminals[2];
		}
		mTerminals[2] = tmp;
	}
	else if (mNominalVoltageEnd1 < mNominalVoltageEnd2) {
		std::shared_ptr<SimTerminal<Complex>> tmp = mTerminals[0];
		mTerminals[0] = mTerminals[1];
		mTerminals[1] = tmp;
	}
	else if (mNominalVoltageEnd1 < mNominalVoltageEnd3) {
		std::shared_ptr<SimTerminal<Complex>> tmp = mTerminals[0];
		mTerminals[0] = mTerminals[2];
		mTerminals[2] = mTerminals[1];
		mTerminals[1] = tmp;
	}
	else if (mNominalVoltageEnd2 < mNominalVoltageEnd3) {
		std::shared_ptr<SimTerminal<Complex>> tmp = mTerminals[1];
		mTerminals[1] = mTerminals[2];
		mTerminals[2] = tmp;
	}
	
	// Set initial voltage of virtual node in between
	mVirtualNodes[0]->setInitialVoltage(initialSingleVoltage(1) * mRatio1);

	// Static calculations from load flow data
	Complex impedance1 = { mResistance1, mReactance1 };
	Complex impedance2 = { mResistance2, mReactance2 };
	Complex impedance3 = { mResistance3, mReactance3 };

	mIntfVoltage(0, 0) = mVirtualNodes[0]->initialSingleVoltage() - initialSingleVoltage(1);
	mIntfCurrent(0, 0) = mIntfVoltage(0, 0) / impedance1;

	mIntfVoltage(1, 0) = mVirtualNodes[0]->initialSingleVoltage() - initialSingleVoltage(2);
	mIntfCurrent(1, 0) = mIntfVoltage(1, 0) / impedance2;

	mIntfVoltage(2, 0) = mVirtualNodes[0]->initialSingleVoltage() - initialSingleVoltage(3);
	mIntfCurrent(2, 0) = mIntfVoltage(2, 0) / impedance3;

	// Create series sub components
	mSubInductor1 = std::make_shared<SP::Ph1::Inductor>(mUID + "_ind1", mName + "_ind1", Logger::Level::off);
	mSubInductor1->setParameters(mInductance1);
	mSubInductor2 = std::make_shared<SP::Ph1::Inductor>(mUID + "_ind2", mName + "_ind2", Logger::Level::off);
	mSubInductor2->setParameters(mInductance2);
	mSubInductor3 = std::make_shared<SP::Ph1::Inductor>(mUID + "_ind3", mName + "_ind3", Logger::Level::off);
	mSubInductor3->setParameters(mInductance3);


	if (mNumVirtualNodes == 4) {
		mVirtualNodes[3]->setInitialVoltage(initialSingleVoltage(0));
		mSubResistor1 = std::make_shared<SP::Ph1::Resistor>(mUID + "_res1", mName + "_res1", Logger::Level::off);
		mSubResistor1->setParameters(mResistance1);
		mSubResistor1->connect({ node(0), mVirtualNodes[2] });
		mSubResistor1->initialize(mFrequencies);
		mSubResistor1->initializeFromNodesAndTerminals(frequency);
		mSubInductor1->connect({ mVirtualNodes[2], mVirtualNodes[0] });
	} else {
		mSubInductor1->connect({ node(0), mVirtualNodes[0] });
	}
	mSubInductor->initialize(mFrequencies);
	mSubInductor->initializeFromNodesAndTerminals(frequency);

	// Create parallel sub components
	// A snubber conductance is added on the low voltage side (resistance scaled with lower voltage side)
	mSnubberResistance = mNominalVoltageEnd1 > mNominalVoltageEnd2 ? std::abs(mNominalVoltageEnd2)*1e6 : std::abs(mNominalVoltageEnd1)*1e6;
	mSubSnubResistor = std::make_shared<SP::Ph1::Resistor>(mUID + "_snub_res", mName + "_snub_res", Logger::Level::off);
	mSubSnubResistor->setParameters(mSnubberResistance);
	mSubSnubResistor->connect({ node(1), SP::SimNode::GND });
	mSubSnubResistor->initialize(mFrequencies);
	mSubSnubResistor->initializeFromNodesAndTerminals(frequency);
	mSLog->info("Snubber Resistance={} [Ohm] (connected to LV side)", mSnubberResistance);

	mSLog->info(
		"\n--- Initialization from powerflow ---"
		"\nVoltage across: {:s}"
		"\nCurrent: {:s}"
		"\nTerminal 0 voltage: {:s}"
		"\nTerminal 1 voltage: {:s}"
		"\nVirtual Node 1 voltage: {:s}"
		"\n--- Initialization from powerflow finished ---",
		Logger::phasorToString(mIntfVoltage(0, 0)),
		Logger::phasorToString(mIntfCurrent(0, 0)),
		Logger::phasorToString(initialSingleVoltage(0)),
		Logger::phasorToString(initialSingleVoltage(1)),
		Logger::phasorToString(mVirtualNodes[0]->initialSingleVoltage()));
}


// #### Powerflow section ####

void SP::Ph1::Transformer::setBaseVoltage(Real baseVoltage) {
	// Note: to be consistent set base voltage to higher voltage (and impedance values must be referred to high voltage side)
	// TODO: use attribute setter for setting base voltage
    mBaseVoltage = baseVoltage;
}

void SP::Ph1::Transformer::calculatePerUnitParameters(Real baseApparentPower, Real baseOmega) {
	mSLog->info("#### Calculate Per Unit Parameters for {}", mName);
    mBaseApparentPower = baseApparentPower;
	mBaseOmega = baseOmega;
    mSLog->info("Base Power={} [VA]  Base Omega={} [1/s]", baseApparentPower, baseOmega);

	mBaseImpedance = mBaseVoltage * mBaseVoltage / mBaseApparentPower;
	mBaseAdmittance = 1.0 / mBaseImpedance;
	mBaseCurrent = baseApparentPower / (mBaseVoltage*sqrt(3)); // I_base=(S_threephase/3)/(V_line_to_line/sqrt(3))
	mSLog->info("Base Voltage={} [V]  Base Impedance={} [Ohm]", mBaseVoltage, mBaseImpedance);

	mResistancePerUnit = mResistance / mBaseImpedance;
	mReactancePerUnit = mReactance / mBaseImpedance;
    mSLog->info("Resistance={} [pu]  Reactance={} [pu]", mResistancePerUnit, mReactancePerUnit);

	mBaseInductance = mBaseImpedance / mBaseOmega;
	mInductancePerUnit = mInductance / mBaseInductance;
	mMagnetizing = mMagnetizingReactance / mBaseImpedance;
	// omega per unit=1, hence 1.0*mInductancePerUnit.
	mLeakagePerUnit = Complex(mResistancePerUnit,1.*mInductancePerUnit);
	mMagnetizingPerUnit = mMagnetizing / mBaseImpedance;

    mRatioAbsPerUnit = mRatioAbs / mNominalVoltageEnd1 * mNominalVoltageEnd2;
    mSLog->info("Tap Ratio={} [pu]", mRatioAbsPerUnit);

	// set snubber resistance
	if (mBehaviour == Behaviour::Initialization) {
		Real snubberResistance = 1e3;
		mSubSnubResistor = std::make_shared<SP::Ph1::Resistor>(mUID + "_snub_res", mName + "_snub_res", mLogLevel);
		mSubSnubResistor->setParameters(snubberResistance);
		mSubSnubResistor->connect({ node(1), SP::SimNode::GND });
		mSubSnubResistor->initializeFromNodesAndTerminals(mBaseOmega);
	}
	if (mSubSnubResistor) {
		mSubSnubResistor->setBaseVoltage(mBaseVoltage);
		mSubSnubResistor->calculatePerUnitParameters(baseApparentPower);
	}

	mSLog->info("Leakage Impedance Per Unit={} [Ohm] ", mLeakagePerUnit);
}

void SP::Ph1::Transformer::pfApplyAdmittanceMatrixStamp(SparseMatrixCompRow & Y) {
	// calculate matrix stamp
	mY_element = MatrixComp(2, 2);
	Complex y = Complex(1, 0) / mLeakagePerUnit;
	Complex ys = Complex(1, 0) / mMagnetizingPerUnit;
	mY_element(0, 0) = (y + ys);
	mY_element(0, 1) = -y*mRatioAbsPerUnit;
	mY_element(1, 0) = -y*mRatioAbsPerUnit;
	mY_element(1, 1) = (y + ys)*std::pow(mRatioAbsPerUnit, 2);

	//check for inf or nan
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			if (std::isinf(mY_element.coeff(i, j).real()) || std::isinf(mY_element.coeff(i, j).imag())) {
				std::cout << mY_element << std::endl;
				std::cout << "Zm:" << mMagnetizing << std::endl;
				std::cout << "Zl:" << mLeakage << std::endl;
				std::cout << "tap:" << mRatioAbsPerUnit << std::endl;
				std::stringstream ss;
				ss << "Transformer>>" << this->name() << ": infinite or nan values in the element Y at: " << i << "," << j;
				throw std::invalid_argument(ss.str());
			}

	//set the circuit matrix values
	Y.coeffRef(this->matrixNodeIndex(0), this->matrixNodeIndex(0)) += mY_element.coeff(0, 0);
	Y.coeffRef(this->matrixNodeIndex(0), this->matrixNodeIndex(1)) += mY_element.coeff(0, 1);
	Y.coeffRef(this->matrixNodeIndex(1), this->matrixNodeIndex(1)) += mY_element.coeff(1, 1);
	Y.coeffRef(this->matrixNodeIndex(1), this->matrixNodeIndex(0)) += mY_element.coeff(1, 0);

	mSLog->info("#### Y matrix stamping: {}", mY_element);
}


void SP::Ph1::Transformer::updateBranchFlow(VectorComp& current, VectorComp& powerflow) {
	mCurrent = current * mBaseCurrent;
	mActivePowerBranch = powerflow.real()*mBaseApparentPower;
	mReactivePowerBranch = powerflow.imag()*mBaseApparentPower;
}


void SP::Ph1::Transformer::storeNodalInjection(Complex powerInjection) {
	mActivePowerInjection = std::real(powerInjection)*mBaseApparentPower;
	mReactivePowerInjection = std::imag(powerInjection)*mBaseApparentPower;
	mStoreNodalPowerInjection = true;
}

MatrixComp SP::Ph1::Transformer::Y_element() {
	return mY_element;
}

// #### MNA Section ####

void SP::Ph1::Transformer::mnaInitialize(Real omega, Real timeStep, Attribute<Matrix>::Ptr leftVector) {
	MNAInterface::mnaInitialize(omega, timeStep);
	updateMatrixNodeIndices();

	mRightVector = Matrix::Zero(leftVector->get().rows(), 1);
	auto subComponents = MNAInterface::List({ mSubInductor, mSubSnubResistor });

	if (mSubResistor)
		subComponents.push_back(mSubResistor);

	for (auto comp : subComponents) {
		comp->mnaInitialize(omega, timeStep, leftVector);

		for (auto task : comp->mnaTasks())
			mMnaTasks.push_back(task);

	}
	mMnaTasks.push_back(std::make_shared<MnaPostStep>(*this, leftVector));

	mSLog->info(
		"\nTerminal 0 connected to {:s} = sim node {:d}"
		"\nTerminal 1 connected to {:s} = sim node {:d}",
		mTerminals[0]->node()->name(), mTerminals[0]->node()->matrixNodeIndex(),
		mTerminals[1]->node()->name(), mTerminals[1]->node()->matrixNodeIndex());
}


void SP::Ph1::Transformer::mnaApplySystemMatrixStamp(Matrix& systemMatrix) {
	// Ideal transformer equations
	if (terminalNotGrounded(0)) {
		Math::setMatrixElement(systemMatrix, mVirtualNodes[0]->matrixNodeIndex(), mVirtualNodes[1]->matrixNodeIndex(), Complex(-1.0, 0));
		Math::setMatrixElement(systemMatrix, mVirtualNodes[1]->matrixNodeIndex(), mVirtualNodes[0]->matrixNodeIndex(), Complex(1.0, 0));
	}
	if (terminalNotGrounded(1)) {
		Math::setMatrixElement(systemMatrix, matrixNodeIndex(1), mVirtualNodes[1]->matrixNodeIndex(), mRatio);
		Math::setMatrixElement(systemMatrix, mVirtualNodes[1]->matrixNodeIndex(), matrixNodeIndex(1), -mRatio);
	}

	// Add inductive part to system matrix
	mSubInductor->mnaApplySystemMatrixStamp(systemMatrix);
	mSubSnubResistor->mnaApplySystemMatrixStamp(systemMatrix);

	if (mNumVirtualNodes == 3) {
		mSubResistor->mnaApplySystemMatrixStamp(systemMatrix);
	}

	if (terminalNotGrounded(0)) {
		mSLog->info("Add {:s} to system at ({:d},{:d})", Logger::complexToString(Complex(-1.0, 0)),
			mVirtualNodes[0]->matrixNodeIndex(), mVirtualNodes[1]->matrixNodeIndex());
		mSLog->info("Add {:s} to system at ({:d},{:d})", Logger::complexToString(Complex(1.0, 0)),
			mVirtualNodes[1]->matrixNodeIndex(), mVirtualNodes[0]->matrixNodeIndex());
	}
	if (terminalNotGrounded(1)) {
		mSLog->info("Add {:s} to system at ({:d},{:d})", Logger::complexToString(mRatio),
			matrixNodeIndex(1), mVirtualNodes[1]->matrixNodeIndex());
		mSLog->info("Add {:s} to system at ({:d},{:d})", Logger::complexToString(-mRatio),
			mVirtualNodes[1]->matrixNodeIndex(), matrixNodeIndex(1));
	}
}


void SP::Ph1::Transformer::mnaAddPostStepDependencies(AttributeBase::List &prevStepDependencies, AttributeBase::List &attributeDependencies, AttributeBase::List &modifiedAttributes, Attribute<Matrix>::Ptr &leftVector) {
	// add post-step dependencies of subcomponents
	if (mSubResistor)
		this->mSubResistor->mnaAddPostStepDependencies(prevStepDependencies, attributeDependencies, modifiedAttributes, leftVector);
	this->mSubInductor->mnaAddPostStepDependencies(prevStepDependencies, attributeDependencies, modifiedAttributes, leftVector);
	this->mSubSnubResistor->mnaAddPostStepDependencies(prevStepDependencies, attributeDependencies, modifiedAttributes, leftVector);
	// add post-step dependencies of component itself
	attributeDependencies.push_back(leftVector);
	modifiedAttributes.push_back(this->attribute("v_intf"));
	modifiedAttributes.push_back(this->attribute("i_intf"));
}

void SP::Ph1::Transformer::mnaPostStep(Real time, Int timeStepCount, Attribute<Matrix>::Ptr &leftVector) {
	// post-step of subcomponents
	if (mSubResistor)
		this->mSubResistor->mnaPostStep(time, timeStepCount, leftVector);
	this->mSubInductor->mnaPostStep(time, timeStepCount, leftVector);
	this->mSubSnubResistor->mnaPostStep(time, timeStepCount, leftVector);
	// post-step of component itself
	this->mnaUpdateVoltage(*leftVector);
	this->mnaUpdateCurrent(*leftVector);
}

void SP::Ph1::Transformer::mnaUpdateCurrent(const Matrix& leftVector) {
	mIntfCurrent(0, 0) = mSubInductor->intfCurrent()(0, 0);
	mSLog->debug("Current {:s}", Logger::phasorToString(mIntfCurrent(0, 0)));

}

void SP::Ph1::Transformer::mnaUpdateVoltage(const Matrix& leftVector) {
	// v1 - v0
	mIntfVoltage(0, 0) = 0;
	mIntfVoltage(0, 0) = Math::complexFromVectorElement(leftVector, matrixNodeIndex(1));
	mIntfVoltage(0, 0) = mIntfVoltage(0, 0) - Math::complexFromVectorElement(leftVector, mVirtualNodes[0]->matrixNodeIndex());
	mSLog->debug("Voltage {:s}", Logger::phasorToString(mIntfVoltage(0, 0)));
}
