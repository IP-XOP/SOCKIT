#include "XOPStandardHeaders.r"

resource 'vers' (1) {						/* XOP version info */
	0x01, 0x11, release, 0x00, 0,			/* version bytes and country integer */
	"1.11",
	"1.11, © 2021 Andrew Nelson, all rights reserved."
};

resource 'vers' (2) {						/* Igor version info */
	0x08, 0x00, release, 0x00, 0,			/* version bytes and country integer */
	"8.00",
	"(for Igor Pro 8.00 or later)"
};

resource 'STR#' (1100) {					/* custom error messages */
	{
		/* [1] */
		"SOCKIT requires Igor Pro 8.00 or later.",
		/* [2] */
		"SOCKIT XOP was called to execute an unknown function.",
		/* [3] */
		"Input string is non-existent.",
        /* [4] */
        "Unable to resolve host.",
        /* [5] */
        "Unable to allocate socket.",
        /* [6] */
        "Unable to connect to server.",
        /* [7] */
        "Unable to send data to server.",
		/* [8] */
		"Wave in use.",
		/* [9] */
		"Socket not connected to anything.",
		/* [10] */
		"Processor function not compiled, or not available.",
		/* [11] */
		"Problem writing to file.",
		/* [12] */
		"No socket with that descriptor.",
		/* [13] */
		"Could not start thread to collect messages, SOCKIT won't work.",
		/* [14] */
		"The supplied string is not an exact multiple of the bytes required for that wavetype.",
		/* [15] */
		"Couldn't lock Mutex.",
		/* [16] */
		"Couldn't find wavebufferinfo.",
		/* [17] */
		"Couldn't create the logfile requested.",
		
	}
};

/* no menu item */

resource 'XOPI' (1100) {
    XOP_VERSION,                            // XOP protocol version.
    DEV_SYS_CODE,                           // Code for development system used to make XOP
    XOP_FEATURE_FLAGS,                      // Tells Igor about XOP features
    XOPI_RESERVED,                          // Reserved - must be zero.
    XOP_TOOLKIT_VERSION,                    // XOP Toolkit version.
};

resource 'STR#' (1101) {					// Misc strings for XOP.
	{
		"-1",								// This item is no longer supported by the Carbon XOP Toolkit.
		"No Menu Item",						// This item is no longer supported by the Carbon XOP Toolkit.
		"SOCKIT Help",					    // Name of XOP's help file.
	}
};

resource 'XOPF' (1100) {
	{
		"SOCKITcloseConnection",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		NT_FP64,
		{
		NT_FP64,
		},
		"SOCKITisItOpen",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		NT_FP64,
		{
		NT_FP64,
		},
		"SOCKITregisterProcessor",			/* function name */
		F_IO | F_EXTERNAL,					/* function category (string) */
		NT_FP64,						    /* return value type */
		{
			NT_FP64,						/* socket number */
			HSTRING_TYPE,					/* processor */
		},
		"SOCKITpeek",						/* function name */
		F_IO | F_THREADSAFE| F_EXTERNAL,	/* function category (string) */
		HSTRING_TYPE,						/* return value type */			
		{
			NT_FP64,						/* socket number */
		},
		"SOCKITsendmsgF",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		NT_FP64,
		{
		NT_FP64,
		HSTRING_TYPE,
		},
		"SOCKITsendnrecvF",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		HSTRING_TYPE,
		{
		NT_FP64,
		HSTRING_TYPE,
		NT_FP64,
		NT_FP64,
		},
		"SOCKITopenconnectionF",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		NT_FP64,
		{
		HSTRING_TYPE,
		NT_FP64,
		NT_FP64,
		},
		"SOCKITtotalOpened",
		F_IO | F_EXTERNAL,
		NT_FP64,
		{
		},
		"SOCKITcurrentOpened",
		F_IO | F_EXTERNAL,
		NT_FP64,
		{
		},
		"SOCKITinfo",
		F_IO | F_THREADSAFE | F_EXTERNAL,
		HSTRING_TYPE,
		{
		NT_FP64,
		},
	}
};

resource 'XOPC' (1100) {
	{
		"SOCKITopenconnection",					// Name of operation.
		XOPOp+UtilOP+compilableOp,			    // Operation's category.
		"SOCKITsendnrecv",						// Name of operation.
		XOPOp+UtilOP+compilableOp,			    // Operation's category.
		"SOCKITsendmsg",						// Name of operation.
		XOPOp+UtilOP+compilableOp,			    // Operation's category.
		"SOCKITstringToWave",					// Name of operation.
		XOPOp+UtilOP+compilableOp+threadSafeOp,	// Operation's category.
		"SOCKITwaveToString",					// Name of operation.
		XOPOp+UtilOP+compilableOp+threadSafeOp,	// Operation's category.
		"SOCKITlist",							// Name of operation.
		XOPOp+UtilOP+compilableOp,			    // Operation's category.
	}
};
