/* ************************************************************************
 *       Filename:  hello.c
 *    Description:  this is a character device driver
 *        Version:  1.0
 *        Created:  12/14/2019 09:23:51 PM
 *         Author:  zhangzk, zongkai.zhang@foxmail.com
 * ************************************************************************/
#include <linux/init.h>         /* printk */
#include <linux/module.h>       /* init/exit */
#include <linux/moduleparam.h>  /* parm */
#include <linux/fs.h>           /* devno. for /proc/devices/module */
#include <linux/device.h>       /* /dev/device */
#include <linux/kdev_t.h>       /* /dev/device */
#include <linux/cdev.h>         /* for cdev operation */

//#define PARM_DEBUG
//#define STATIC_NO
/* ***********************************************************************
 *  for device
 * **********************************************************************/
/* device number */
#ifdef STATIC_NO
dev_t dev = MKDEV(235, 0);
#else
dev_t dev = 0;
static struct class *dev_class;
#endif

/* character device */
static struct cdev hello_cdev;

#ifdef PARM_DEBUG
/* ***********************************************************************
 *  for param
 * **********************************************************************/
int  value, arr_value[4];
char *name;
int  myvalue = 0;

module_param(value,           int,         S_IRUSR|S_IWUSR);   //integer value
module_param(name,            charp,       S_IRUSR|S_IWUSR);   //string
module_param_array(arr_value, int,   NULL, S_IRUSR|S_IWUSR);   //array of integers

int notify_param(const char *val, const struct kernel_param *kp)
{
    int res = param_set_int(val, kp); // use helper for write variable
    if(res==0)
    {
        printk(KERN_INFO "call back function called...\n");
        printk(KERN_INFO "new value of myvalue = %d\n", myvalue);
        return 0;
    }
    return -1;
}

const struct kernel_param_ops my_param_ops =
{
    .set = &notify_param,  // use our setter ...
    .get = &param_get_int, // .. and standard getter
};
module_param_cb(myvalue, &my_param_ops, &myvalue, S_IRUGO|S_IWUSR );
#endif
/* ***********************************************************************
 *  for character device operation
 * **********************************************************************/
static int     my_open   (struct inode *inode, struct file *file);
static int     my_release(struct inode *inode, struct file *file);
static ssize_t my_read   (struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t my_write  (struct file *filp, const char __user *buf, size_t len,loff_t * off);

static struct file_operations fops =
{
    .owner   = THIS_MODULE,
    .read    = my_read,
    .write   = my_write,
    .open    = my_open,
    .release = my_release,
};

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "%s called...\n", __func__);
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "%s called...\n", __func__);
    return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t len,loff_t * off)
{
    printk(KERN_INFO "%s called...\n", __func__);
    return 0;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t len,loff_t * off)
{
    printk(KERN_INFO "%s called...\n", __func__);
    return len;
}
/* ***********************************************************************
 *  for module
 * **********************************************************************/
static int __init hello_init(void)
{
    int i = 0;
#ifdef STATIC_NO
    register_chrdev_region(dev, 1, "hello_dev");
#else
    /* allocating major number */
    if((alloc_chrdev_region(&dev, 0, 1, "hello_dev")) < 0)
    {
        printk(KERN_INFO "cannot allocate major number for device\n");
        return -1;
    }

    /* creating cdev structure */
    cdev_init(&hello_cdev,&fops);
    /* adding character device to the system */
    if((cdev_add(&hello_cdev,dev,1)) < 0)
    {
        printk(KERN_INFO "cannot add the device to the system\n");
        goto r_class;
    }
    /* creating struct class */
    if((dev_class = class_create(THIS_MODULE, "hello_class")) == NULL)
    {
        printk(KERN_INFO "cannot create the struct class for device\n");
        goto r_class;
    }
    /* creating device */
    if((device_create(dev_class,NULL,dev,NULL,"hello_dev")) == NULL)
    {
        printk(KERN_INFO "cannot create the device\n");
        goto r_device;
    }
#endif
#ifdef PARM_DEBUG
    printk(KERN_INFO "major = %d minor = %d \n",MAJOR(dev), MINOR(dev));
    printk(KERN_INFO "value = %d  \n", value);
    printk(KERN_INFO "myvalue = %d  \n", myvalue);
    printk(KERN_INFO "name = %s \n", name);
    for (i = 0; i < (sizeof arr_value / sizeof (int)); i++)
    {
        printk(KERN_INFO "arr_value[%d] = %d\n", i, arr_value[i]);
    }
#endif
    printk(KERN_INFO "module insertd successfully...\n");
    return 0;

#ifndef STATIC_NO
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
#endif
}

static void __exit hello_exit(void)
{
#ifndef STATIC_NO
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&hello_cdev);
#endif
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "module removed successfully...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kai");
MODULE_DESCRIPTION("hello character module");
MODULE_VERSION("1:1.0");
