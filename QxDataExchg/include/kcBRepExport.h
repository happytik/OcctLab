#pragma once

#include "kcBaseExport.h"

class QXLIB_API kcBRepExport : public kcBaseExport
{
public:
	kcBRepExport(kcModel *pModel);
	~kcBRepExport(void);

	virtual void			DoExport();
};

