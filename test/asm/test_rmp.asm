; generate ramp on l/r
	skp RUN, start
	wldr 0, -32768, 4096
	wldr 1, 1, 4096
start:	cho rdal, rmp0
	wrax DACL, 0
	cho rdal, rmp1
	wrax DACR, 0
