#ifndef _QX_LIB_DEF_H_
#define _QX_LIB_DEF_H_

#ifndef QXLIB_API
#ifdef QXLIB_DLL
	#define QXLIB_API  _declspec(dllexport)
#else
	#define QXLIB_API  _declspec(dllimport)
#endif
#endif

#endif