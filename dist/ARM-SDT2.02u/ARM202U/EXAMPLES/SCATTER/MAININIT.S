	AREA MainWithOverlayInit, CODE, READONLY
	IMPORT	|Image$$overlay_init|
        IMPORT  InitialiseApp
	IMPORT  __entry
	EXPORT  __main

	ENTRY
__main
        BL      InitialiseApp
	BL	|Image$$overlay_init|
	BL	__entry
	END
