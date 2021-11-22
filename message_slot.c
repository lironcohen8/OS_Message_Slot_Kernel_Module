#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/module.h>
#include <linux/init.h>

// loader
static int load_module(void) 
{
    // initializing data structure using kmalloc()
    // doing checks
    printk("Loading Module\n");
    return 0;
    // non 0 means load did not succeed
}

// unloader
static void unload_module(void)
{
    // free memory using kfree()
    printk("Unloading Module\n");
}

module_init(load_module);
module_exit(unload_module);