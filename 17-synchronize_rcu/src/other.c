#include <linux/module.h>

void other_function(void)
{
  printk("call %s.\n", __FUNCTION__);
}
EXPORT_SYMBOL(other_function);
