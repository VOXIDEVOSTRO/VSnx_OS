/*
	Headerz
*/
#include "_pci_.h"
/*
	Main PCI init
	just load whatever
*/
int pci_load_drivers(void) {
	/*
		Is hook order present?
	*/
    if (hookorder_count() == 0) {
		/*
			Load is not
		*/
        int loaded = hookorder_load(NULL);
        if (loaded < 0) {
			/*
				Doesnt exist
			*/
            #ifdef DEBUG
            printf("[pci] hookorder load failed (%d)\n", loaded);
            #endif
            return -1;
        }
    }
	/*
		Look for its tokens:
		CLASS:BUS:PCI
	*/
    const hookorder_entry_t* entry = hookorder_find_exact("BUS", "PCI");
    if (!entry) {
		/*
			Not found
		*/
        #ifdef DEBUG
        printf("[pci] No hookorder entry for CLASS:BUS:PCI\n");
        #endif
        return -2;
    }
	/*
		Spawn it
	*/
    process_t* proc = NULL;
    int rc = hookorder_spawn_for("BUS", "PCI", &proc);
    if (rc < 0 || !proc) {
		/*
			Failed to do so
		*/
        #ifdef DEBUG
        printf("[pci] Failed to spawn PCI driver (rc=%d)\n", rc);
        #endif
        return -3;
    }
	/*
		Done
	*/
    #ifdef DEBUG
    printf("[pci] PCI driver spawned successfully (pid=%u)\n", proc->pid);
    #endif
    return 0;
}
