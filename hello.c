// SPDX-License-Identifier: GPL-2.0
/*
 * "Hello, world!" kernel module with a uint parameter and
 * a linked list of print timestamps (AK-2, lab 4).
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/slab.h>

MODULE_AUTHOR("Rentsevych D., IO-45");
MODULE_DESCRIPTION("Hello, world with a print counter and event list");
MODULE_LICENSE("GPL");

#define HELLO_MAX_COUNT	10
#define HELLO_WARN_MIN	5

static uint count = 1;
module_param(count, uint, 0444);
MODULE_PARM_DESC(count,
		 "Number of 'Hello, world!' lines (default 1; 0 or 5..10 warn; >10 is an error)");

struct hello_event {
	struct list_head list;
	ktime_t time;
};

static LIST_HEAD(hello_events);

static void hello_free_events(bool print)
{
	struct hello_event *ev, *tmp;

	list_for_each_entry_safe(ev, tmp, &hello_events, list) {
		if (print)
			pr_info("event time: %lld ns\n", ktime_to_ns(ev->time));
		list_del(&ev->list);
		kfree(ev);
	}
}

static int __init hello_init(void)
{
	struct hello_event *ev;
	uint i;

	if (count > HELLO_MAX_COUNT) {
		pr_err("count=%u is too big (max %d), refusing to load\n",
		       count, HELLO_MAX_COUNT);
		return -EINVAL;
	}

	if (count == 0 || count >= HELLO_WARN_MIN)
		pr_warn("count=%u is 0 or between %d and %d, continuing\n",
			count, HELLO_WARN_MIN, HELLO_MAX_COUNT);

	for (i = 0; i < count; i++) {
		ev = kmalloc_obj(*ev, GFP_KERNEL);
		if (!ev) {
			hello_free_events(false);
			return -ENOMEM;
		}

		ev->time = ktime_get();
		list_add_tail(&ev->list, &hello_events);

		pr_emerg("Hello, world!\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	hello_free_events(true);
}

module_init(hello_init);
module_exit(hello_exit);
