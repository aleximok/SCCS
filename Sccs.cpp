#include "stdafx.h"

#include "CSccsApplication.h"

//
//	main
//

int main (int argc, char *argv [])
{
	CSccsApplication app (argc, argv);
	app.run ();

	return app.getReturnCode ();
}
