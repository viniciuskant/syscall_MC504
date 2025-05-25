#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MC504_SUFFIX "MC504 TESTE"
#define MC504_SUFFIX_LEN 11  // strlen("MC504 TESTE")

SYSCALL_DEFINE4(read_mc504, int, fd, char __user *, buf, size_t, count, loff_t __user *, pos)
{
    struct file *file;
    char *kbuf;
    ssize_t ret, total_len;
    loff_t kpos = 0;

    // Allocate kernel buffer with space for suffix and null terminator
    kbuf = kmalloc(count + MC504_SUFFIX_LEN + 1, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    // Get file struct from fd
    file = fget(fd);
    if (!file) {
        kfree(kbuf);
        return -EBADF;
    }

    // Read from file
    if (pos && copy_from_user(&kpos, pos, sizeof(loff_t))) {
        fput(file);
        kfree(kbuf);
        return -EFAULT;
    }

    ret = kernel_read(file, kbuf, count, &kpos);

    if (ret < 0) {
        fput(file);
        kfree(kbuf);
        return ret;
    }

    // Append "MC504 TESTE" and null terminator
    memcpy(kbuf + ret, MC504_SUFFIX, MC504_SUFFIX_LEN);
    kbuf[ret + MC504_SUFFIX_LEN] = '\0';

    total_len = ret + MC504_SUFFIX_LEN + 1;

    // Copy to user buffer
    if (copy_to_user(buf, kbuf, total_len)) {
        fput(file);
        kfree(kbuf);
        return -EFAULT;
    }

    if (pos && copy_to_user(pos, &kpos, sizeof(loff_t))) {
        fput(file);
        kfree(kbuf);
        return -EFAULT;
    }

    fput(file);
    kfree(kbuf);
    return total_len;
}