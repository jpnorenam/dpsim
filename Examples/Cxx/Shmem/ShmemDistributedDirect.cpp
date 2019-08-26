/** Example of shared memory interface
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

#include <DPsim.h>

using namespace DPsim;
using namespace CPS::DP;
using namespace CPS::DP::Ph1;

int main(int argc, char *argv[]) {
	// Testing the interface with a simple circuit,
	// but the load is simulated in a different instance.
	// Values are exchanged using the ideal transformator model: an ideal
	// current source on the supply side and an ideal voltage source on the
	// supply side, whose values are received from the respective other circuit.
	// Here, the two instances directly communicate with each other without using
	// VILLASnode in between.

	if (argc < 2) {
		std::cerr << "not enough arguments (either 0 or 1 for the test number)" << std::endl;
		std::exit(1);
	}

	String in, out;

	if (String(argv[1]) == "0") {
		in  = "/dpsim10";
		out = "/dpsim01";
	}
	else if (String(argv[1]) == "1") {
		in  = "/dpsim01";
		out = "/dpsim10";
	}

	Interface intf(in, out);

	Real timeStep = 0.001;

	if (String(argv[1]) == "0") {
		// Nodes
		auto n1 = Node::make("n1");
		auto n2 = Node::make("n2");

		// Components
		auto evs = VoltageSource::make("v_intf", Logger::Level::debug);
		auto vs1 = VoltageSource::make("vs_1", Logger::Level::debug);
		auto r01 = Resistor::make("r_0_1", Logger::Level::debug);

		evs->setParameters(Complex(5, 0));
		vs1->setParameters(Complex(10, 0));
		r01->setParameters(1);

		evs->connect({ Node::GND, n2 });
		vs1->connect({ Node::GND, n1 });
		r01->connect({ n1, n2 });

		evs->setAttributeRef("V_ref", intf.importComplex(0));
		intf.exportComplex(evs->attributeMatrixComp("i_intf")->coeff(0,0), 0);

		auto sys = SystemTopology(50, SystemNodeList{n1, n2}, SystemComponentList{evs, vs1, r01});
		Simulation sim("ShmemDistributedDirect_1", sys, timeStep, 0.1);

		sim.addInterface(&intf);
		sim.run();
	}
	else if (String(argv[1]) == "1") {
		// Nodes
		auto n1 = Node::make("n1");

		// Components
		auto ecs = CurrentSource::make("i_intf", Logger::Level::debug);
		ecs->setParameters(Complex(5, 0));
		auto rgnd0 = Resistor::make("r_gnd_0", Logger::Level::debug);
		rgnd0->setParameters(1);

		ecs->connect({ Node::GND, n1 });
		rgnd0->connect({ Node::GND, n1 });

		//auto ecs_switch = CurrentSource::make("i_switch", GND, 1, Complex(0, 0));
		//auto r01 = Resistor::make("r_0_1", 0, 1, 1);

		ecs->setAttributeRef("I_ref", intf.importComplex(0));
		intf.exportComplex(ecs->attributeMatrixComp("i_intf")->coeff(0, 0), 0);

		auto sys = SystemTopology(50, SystemNodeList{n1}, SystemComponentList{ecs, rgnd0});
		Simulation sim("ShmemDistributedDirect_2", sys, timeStep, 0.1);

		sim.addInterface(&intf);
		sim.run();
	}
	else {
		std::cerr << "invalid test number" << std::endl;
		std::exit(1);
	}

	return 0;
}
