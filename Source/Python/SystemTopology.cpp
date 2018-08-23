/** Python system tolopogy
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2018, Institute for Automation of Complex Power Systems, EONERC
 *
 * CPowerSystems
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

#include "Python/SystemTopology.h"
#include "Python/Component.h"
#include "Python/Node.h"

using namespace DPsim;

const char *DocSystemTopologyAddComponent =
"add_component(comp)\n"
"Add a component to this system topology\n";
PyObject* Python::SystemTopology::addComponent(PyObject *self, PyObject *args)
{
	Python::SystemTopology *pySys = (Python::SystemTopology*) self;
	PyObject* pyObj;
	Python::Component* pyComp;

	if (!PyArg_ParseTuple(args, "O", &pyObj))
		return nullptr;

	if (!PyObject_TypeCheck(pyObj, &Python::ComponentType)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be dpsim.Component");
		return nullptr;
	}

	pyComp = (Component*) pyObj;
	pySys->sys->addComponent(pyComp->comp);
	Py_INCREF(pyObj);

	pySys->refs.push_back(pyObj);
	Py_INCREF(Py_None);

	return Py_None;
}

#ifdef WITH_GRAPHVIZ
const char *DocSystemTopologyReprSVG =
"_repr_svg_(comp)\n"
"Return a SVG graph rendering of the system Topology\n";
PyObject* Python::SystemTopology::reprSVG(PyObject *self, PyObject *args)
{
	Python::SystemTopology *pySys = (Python::SystemTopology*) self;

	auto graph = pySys->sys->getTopologyGraph();

	std::stringstream ss;

	graph.render(ss, "neato", "svg");

	return PyUnicode_FromString(ss.str().c_str());
}
#endif

PyObject* Python::SystemTopology::newfunc(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Python::SystemTopology *self;

	self = (Python::SystemTopology*) type->tp_alloc(type, 0);
	if (self) {
		using SharedSysPtr = std::shared_ptr<CPS::SystemTopology>;
		using PyObjectsList = std::vector<PyObject *>;

		new (&self->sys) SharedSysPtr();
		new (&self->refs) PyObjectsList();
	}

	return (PyObject*) self;
}

int Python::SystemTopology::init(Python::SystemTopology *self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = {"frequency", "nodes", "components", NULL};
	double frequency = 50;

	PyObject *pyCompList = NULL;
	PyObject *pyNodeList = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "d|OO", kwlist, &frequency, &pyNodeList, &pyCompList)) {
		return -1;
	}

	CPS::ComponentBase::List compList;
	CPS::TopologicalNode::List nodeList;

	if (pyNodeList) {
		try {
			nodeList = nodesFromPython(pyNodeList);
		} catch (...) {
			PyErr_Format(PyExc_AttributeError, "Parameter 'nodes' must be a list of dpsim.Node");
			return -1;
		}
	}

	if (pyCompList) {
		try {
			compList = compsFromPython(pyCompList);
		} catch (...) {
			PyErr_Format(PyExc_AttributeError, "Parameter 'components' must be a list of dpsim.Component");
			return -1;
		}
	}

	self->sys = std::make_shared<CPS::SystemTopology>(frequency, nodeList, compList);

	return 0;
};

void Python::SystemTopology::dealloc(Python::SystemTopology *self)
{
	for (auto it : self->refs) {
		Py_DECREF(it);
	}

	// Since this is not a C++ destructor which would automatically call the
	// destructor of its members, we have to manually call the destructor of
	// the vectors here to free the associated memory.

	// This is a workaround for a compiler bug: https://stackoverflow.com/a/42647153/8178705
	using SharedSysPtr = std::shared_ptr<CPS::SystemTopology>;
	using PyObjectsList = std::vector<PyObject *>;

	if (self->sys)
		self->sys.~SharedSysPtr();

	self->refs.~PyObjectsList();

	Py_TYPE(self)->tp_free((PyObject*) self);
}


static PyMethodDef SystemTopology_methods[] = {
	{"add_component", DPsim::Python::SystemTopology::addComponent, METH_VARARGS, DocSystemTopologyAddComponent},
#ifdef WITH_GRAPHVIZ
	{"_repr_svg_", DPsim::Python::SystemTopology::reprSVG, METH_NOARGS, DocSystemTopologyReprSVG},
#endif
	{NULL, NULL, 0, NULL}
};

static const char* DocSystemTopology =
"A system topology.\n"
"\n"
"Proper ``__init__`` signature:\n"
"\n"
"``__init__(self, frequency=50.0, components)``.\n\n"
"``frequency`` is the nominal system frequency in Hz.\n\n"
"``components`` must be a list of `Component` that are to be simulated.\n\n";
PyTypeObject DPsim::Python::SystemTopologyType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"dpsim.SystemTopology",                  /* tp_name */
	sizeof(DPsim::Python::SystemTopology),     /* tp_basicsize */
	0,                                       /* tp_itemsize */
	(destructor)DPsim::Python::SystemTopology::dealloc, /* tp_dealloc */
	0,                                       /* tp_print */
	0,                                       /* tp_getattr */
	0,                                       /* tp_setattr */
	0,                                       /* tp_reserved */
	0,                                       /* tp_repr */
	0,                                       /* tp_as_number */
	0,                                       /* tp_as_sequence */
	0,                                       /* tp_as_mapping */
	0,                                       /* tp_hash  */
	0,                                       /* tp_call */
	0,                                       /* tp_str */
	0,                                       /* tp_getattro */
	0,                                       /* tp_setattro */
	0,                                       /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	DocSystemTopology,                       /* tp_doc */
	0,                                       /* tp_traverse */
	0,                                       /* tp_clear */
	0,                                       /* tp_richcompare */
	0,                                       /* tp_weaklistoffset */
	0,                                       /* tp_iter */
	0,                                       /* tp_iternext */
	SystemTopology_methods,                  /* tp_methods */
	0,                                       /* tp_members */
	0,                                       /* tp_getset */
	0,                                       /* tp_base */
	0,                                       /* tp_dict */
	0,                                       /* tp_descr_get */
	0,                                       /* tp_descr_set */
	0,                                       /* tp_dictoffset */
	(initproc)DPsim::Python::SystemTopology::init,/* tp_init */
	0,                                       /* tp_alloc */
	DPsim::Python::SystemTopology::newfunc,    /* tp_new */
};