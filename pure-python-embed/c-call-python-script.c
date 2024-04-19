#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <libgen.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#ifndef DEFAULT_PY_PATH
static_assert(false, "default python path not detected");
#endif

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "en_US.utf8");

  PyObject *pName, *pModule, *pFunc;
  PyObject *pArgs, *pValue;
  int i;

  if (argc < 3) {
    fprintf(stderr, "Usage: call pythonfile funcname [args]\n");
    return 1;
  }

  wchar_t *program = Py_DecodeLocale(argv[0], NULL);
  if (program == NULL) {
    fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
    exit(1);
  }
  Py_SetProgramName(program);

  char exe_path[1024];
  if (realpath(argv[0], exe_path) == NULL) {
    perror("realpath");
    return 1;
  }

  // Extract the directory component from the executable path
  char *dir = dirname(exe_path);
  char py_path_including_cwd[2048] = "";
  strcat(py_path_including_cwd, DEFAULT_PY_PATH);
  strcat(py_path_including_cwd, ":");
  strcat(py_path_including_cwd, dir);

  wchar_t w_py_path[strlen(py_path_including_cwd) + 1];
  mbstowcs(w_py_path, py_path_including_cwd, strlen(py_path_including_cwd));
  w_py_path[strlen(py_path_including_cwd)] = L'\0';
  fwprintf(stdout, L"setting py path to: %ls\n", w_py_path);

  // ensure it can find python module defined in current directory
  Py_SetPath(w_py_path);
  Py_Initialize();
  pName = PyUnicode_DecodeFSDefault(argv[1]);
  /* Error checking of pName left out */

  pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule != NULL) {
    pFunc = PyObject_GetAttrString(pModule, argv[2]);
    /* pFunc is a new reference */

    if (pFunc && PyCallable_Check(pFunc)) {
      pArgs = PyTuple_New(argc - 3);
      for (i = 0; i < argc - 3; ++i) {
        pValue = PyLong_FromLong(atoi(argv[i + 3]));
        if (!pValue) {
          Py_DECREF(pArgs);
          Py_DECREF(pModule);
          fprintf(stderr, "Cannot convert argument\n");
          return 1;
        }
        /* pValue reference stolen here: */
        PyTuple_SetItem(pArgs, i, pValue);
      }
      pValue = PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);
      if (pValue != NULL) {
        printf("Result of call: %ld\n", PyLong_AsLong(pValue));
        Py_DECREF(pValue);
      } else {
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr, "Call failed\n");
        return 1;
      }
    } else {
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
    }
    Py_XDECREF(pFunc);
    Py_DECREF(pModule);
  } else {
    PyErr_Print();
    fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
    return 1;
  }
  if (Py_FinalizeEx() < 0) {
    return 120;
  }
  return 0;
}
