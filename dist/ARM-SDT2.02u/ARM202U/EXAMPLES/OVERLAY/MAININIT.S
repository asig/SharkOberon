	AREA MainWithOverlayInit, CODE, READONLY
	IMPORT	|Image$$overlay_init|
	IMPORT	__main

	ENTRY
	BL	|Image$$overlay_init|
	BL	__main
	END

