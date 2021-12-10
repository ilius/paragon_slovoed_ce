//#include <Python.h>
#include <pybind11/pybind11.h>
#include "SldDictionary.h"

namespace py = pybind11;

PYBIND11_MODULE(example, m) {
	// optional module docstring
	m.doc() = "sdc_engine: Slovoed Dictionary Container Engine";

	// define a function
	// m.def("add", &add, "A function which adds two numbers");

	// bindings to CSldDictionary class
	py::class_<CSldDictionary>(m, "CSldDictionary")
		.def(py::init<const std::string &, int>())
		.def("open", &CSldDictionary::Open)
		.def("close", &CSldDictionary::Close)

		.def("getTotalWordCount", &CSldDictionary::GetTotalWordCount)
		.def("getWordByIndex", &CSldDictionary::GetWordByIndex)
		.def("getMorphology", &CSldDictionary::GetMorphology)

		//.def("setCurrentWordlist", &CSldDictionary::SetCurrentWordlist)
		//.def("getCurrentWordList", &CSldDictionary::GetCurrentWordList)
		//.def("setBase", &CSldDictionary::SetBase)

		//.def("getWordByGlobalIndex", &CSldDictionary::GetWordByGlobalIndex)
		//.def("getWordByText", &CSldDictionary::GetWordByText)
		//.def("getWordByTextExtended", &CSldDictionary::GetWordByTextExtended)
		//.def("getWordSetByTextExtended", &CSldDictionary::GetWordSetByTextExtended)
	;

}

/*
static PyObject *OpenDictionary(PyObject *self, PyObject *args)
{
	const char *filename;
	if (!PyArg_ParseTuple(args, "s", &filename)) {return NULL;}

	ESldError error;
	ISldLayerAccess *aLayerAccess;
	ISDCFile *aFile;
	CSldDictionary *dic;

	//aLayerAccess = new ISldLayerAccess;
	//*aFile = new ISDCFile;
	// FIXME: what to do with filename?

	dic = new CSldDictionary;
	error = dic->Open(aFile, aLayerAccess);
	if (error != eOK) {
		PyErr_SetString(PyExc_IOError, "failed to open sdc file");
		return NULL;
	}

	// Py_INCREF(dic);

	PyObject *pyDic = PyObject_New(dic, CSldDictionary);
	// use PyObject_GC_new if it has cyclic references

	return pyDic;
}
static PyMethodDef PythonMethods[] = {
	{"OpenDictionary", OpenDictionary, METH_VARARGS, ""},
	//{"funcname",  func, METH_VARARGS, "comment"},
	{NULL, NULL, 0, NULL}        // Sentinel
};

*/

