//
#ifndef _KC_IGES_EXPORT_H_
#define _KC_IGES_EXPORT_H_

#include "kcBaseExport.h"

class QXLIB_API kcIGESExport : public kcBaseExport
{
public:
	kcIGESExport(kcModel *pModel);
	~kcIGESExport(void);

	virtual void			DoExport();

protected:
	bool					WriteIGES(const Standard_CString& aFileName);
};

#endif