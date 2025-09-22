/*
	Header files
*/
#include "gpu.h"
/*
	Start up GPU CLASS
*/
int gpu_load_drivers(void) {
	/*
		VBOX
	*/
	vboxsvga_spawn_driver();
	/*
		VGA
	*/
	vga_spawn_driver();
	/*
		DONE!
	*/
	return 0;
}