/*
 *  linux/fs/minix/file.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  minix regular file handling primitives
 */

#include "minix.h"
#include <linux/aio.h>
#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/kdev_t.h>
#include <linux/dcache.h>
#include <linux/path.h>
#include <linux/stat.h>
#include <linux/cache.h>
#include <linux/list.h>
#include <linux/list_lru.h>
#include <linux/llist.h>
#include <linux/radix-tree.h>
#include <linux/rbtree.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/capability.h>
#include <linux/semaphore.h>
#include <linux/fiemap.h>
#include <linux/rculist_bl.h>
#include <linux/atomic.h>
#include <linux/shrinker.h>
#include <linux/migrate_mode.h>
#include <linux/uidgid.h>
#include <linux/lockdep.h>
#include <linux/percpu-rwsem.h>
#include <linux/blk_types.h>

#include <asm/byteorder.h>
#include <uapi/linux/fs.h>
#include <linux/kernel.h>
#include <uapi/linux/uio.h>
#include <linux/types.h>

#include <linux/export.h>
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/capability.h>
#include <linux/kernel_stat.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/hash.h>
#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/security.h>
#include <linux/cpuset.h>
#include <linux/hardirq.h> // for BUG_ON(!in_atomic()) only 
#include <linux/hugetlb.h>
#include <linux/memcontrol.h>
#include <linux/cleancache.h>
#include <linux/rmap.h>

#include <trace/events/filemap.h>

//////////////////////
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function

#include <crypto/hash.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <crypto/rng.h>
#include <crypto/md5.h>
#include <crypto/sha.h>
#include <crypto/internal/skcipher.h>
#include <linux/jiffies.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <crypto/scatterwalk.h>
#include <linux/list.h>
#include <linux/string.h>
////////////////////////////

#define  DEVICE_NAME "ebbchar"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "ebb"        ///< The device class -- this is a character device driver


static char *key = "chave"; // passando a chave como parametro

module_param(key, charp, 0000);	
MODULE_PARM_DESC(key, "A character string");


ssize_t write_crypto(struct kiocb *iocb, struct iov_iter *from);
ssize_t read_crypto(struct kiocb *iocb, struct iov_iter *from);

static int criptar(void* buffer);
static int decriptar(void *buffer);
static void test_skcipher_cb(struct crypto_async_request *req, int error);
//static unsigned int test_skcipher_encdec(struct skcipher_def *sk, int enc);

const struct file_operations minix_file_operations = {
	
	.llseek			= generic_file_llseek,
	//.read_iter		= generic_file_read_iter,
	.read_iter		= read_crypto,
	//.write_iter		= generic_file_write_iter,
	.write_iter		= write_crypto,
	.mmap			= generic_file_mmap,
	.fsync			= generic_file_fsync,
	.splice_read	= generic_file_splice_read,
};

static int minix_setattr(struct dentry *dentry, struct iattr *attr)
{
	printk("file %s line 28\n", __PRETTY_FUNCTION__);
	struct inode *inode = d_inode(dentry);
	int error;

	error = setattr_prepare(dentry, attr);
	if (error)
		return error;

	if ((attr->ia_valid & ATTR_SIZE) &&
	    attr->ia_size != i_size_read(inode)) {
		error = inode_newsize_ok(inode, attr->ia_size);
		if (error)
			return error;

		truncate_setsize(inode, attr->ia_size);
		minix_truncate(inode);
	}

	setattr_copy(inode, attr);
	mark_inode_dirty(inode);
	return 0;
}

ssize_t read_crypto(struct kiocb *iocb, struct iov_iter *from){

	//(int *)from->iov->iov_base - 560;
	//decriptar((void __user *)from->iov->iov_base);
	
	return generic_file_read_iter(iocb,from); // Como se fosse o Super do java!

}

ssize_t write_crypto(struct kiocb *iocb, struct iov_iter *from)
{
	printk("file %s line 178\n", __PRETTY_FUNCTION__);

	//printk("file %d line 180\n", from->type);
	//printk("file %u line 181\n", from->iov_offset);
	//printk("file %u line 182\n", from->count);
	printk("entrando ############ %llu \n", from->iov->iov_base);
	printk("len do write %d \n", from->iov->iov_len);
	//printk("file %d line 185\n", iocb->ki_pos);
	//printk("Key = %s line 185\n", key);

	//int i;
	//void __user *base = from->iov->iov_base;
	//int count = 0;
	//while(count < 4096){
	//printk("base %d",base);
	criptar((void __user *)from->iov->iov_base);
	//base = (char *)base+16;
	//count = count+16;
	//}
	


	return generic_file_write_iter(iocb,from); // Como se fosse o Super do java!

}

struct tcrypt_result { // struct para necessaria para a criptografia e decriptografia
    struct completion completion;
    int err;
};

struct skcipher_def { // struct para necessaria para a criptografia e decriptografia
    struct scatterlist sg;
    struct crypto_skcipher *tfm;
    struct skcipher_request *req;
    struct tcrypt_result result;
};

static void test_skcipher_cb(struct crypto_async_request *req, int error) // funcao para necessaria para a criptografia e decriptografia
{
    struct tcrypt_result *result = req->data;

    if (error == -EINPROGRESS)
        return;
    result->err = error;
    complete(&result->completion);
    pr_info("Encryption finished successfully\n");
}

static unsigned int test_skcipher_encdec(struct skcipher_def *sk, int enc) // funcao para necessaria para a criptografia e decriptografia
{
	int rc = 0;

	if (enc)
		rc = crypto_skcipher_encrypt(sk->req);
	else
		rc = crypto_skcipher_decrypt(sk->req);

	switch (rc) {
		case 0:
		break;
		case -EINPROGRESS:
		case -EBUSY:
		rc = wait_for_completion_interruptible(
		&sk->result.completion);
		if (!rc && !sk->result.err) {
			reinit_completion(&sk->result.completion);
			break;
		}
		default:
			pr_info("skcipher encrypt returned with %d result %d\n",
			rc, sk->result.err);
		break;
	}
	init_completion(&sk->result.completion);

	return rc;
}

static int criptar(void __user *buffer){
    struct skcipher_def sk;
    struct crypto_skcipher *skcipher = NULL;
    struct skcipher_request *req = NULL;
    void *scratchpad = NULL;
    unsigned char keyLocal[33];
    
    char *ivdata = NULL;
    int ret = -EFAULT;
    int i = 0;
    
    //sprintf(message, "\n\nOP1\n\n");

    skcipher = crypto_alloc_skcipher("ecb(aes)", 0, 0);
    if (IS_ERR(skcipher)) {
        pr_info("could not allocate skcipher handle\n");
        return PTR_ERR(skcipher);
    }

    req = skcipher_request_alloc(skcipher, GFP_KERNEL); 
    if (!req) {
        pr_info("could not allocate skcipher request\n");
        ret = -ENOMEM;
        goto out;
    }

    skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, test_skcipher_cb, &sk.result);

    
     /* AES 256 with random key */
    //get_random_bytes(&keyLocal, 32);
    // manset para zerar a variavel key
    strncpy(keyLocal, key,32); //passando a key para keyLocal, para ser usada na criptografia, onde seu maximo eh 32 caracteres
    keyLocal[32] = '\0';
    printk(KERN_INFO "Key Local = %s \n", keyLocal);
    
    if (crypto_skcipher_setkey(skcipher, keyLocal, 32)) {
        pr_info("key could not be set\n");
        ret = -EAGAIN;
        goto out;
    }
   
     /* IV will be random */
    ivdata = kmalloc(16, GFP_KERNEL);
    if (!ivdata) {
        pr_info("could not allocate ivdata\n");
        goto out;
    }
   get_random_bytes(ivdata, 16);
   
    /* Input data will be random */
    scratchpad = kmalloc(16, GFP_KERNEL); // variavel que vai conter a string para criptografia, com tamanho 16
    if (!scratchpad) {
        pr_info("could not allocate scratchpad\n");
        goto out;
    }
    memcpy(scratchpad, buffer, 16); // passando o buffer para a variavel scratchpad com tamanho maximo de 16
    //scratchpad[16] = '\0'; // colocando \0 na ultima posicao da string, para saber quando parar de criptografar
    sk.tfm = skcipher;
    sk.req = req;
    
    // printk(KERN_INFO "Original: %s\n", scratchpad);
    //for(i = 0; i < 16; i++)
	//printk(KERN_INFO "Original: %02x\n", (unsigned char)scratchpad[i]);
    
    /* We encrypt one block */
    sg_init_one(&sk.sg, scratchpad, 16); // passando a string q vai ser criptografada para sk.sg, com tamanho de 16
    skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata); // sk.sg sendo criptografada e o tamanho eh 16
    init_completion(&sk.result.completion);

    /* encrypt data */
    ret = test_skcipher_encdec(&sk, 1); // 1 para criptar e 0 para decriptar
    if (ret)
        goto out; 
    
    
    pr_info("Encryption triggered successfully\n");
    sg_copy_to_buffer(&sk.sg, 1, buffer, 16); // passando a resposta que esta em sk.sg para a variavel com tamanho de 16
	decriptar(buffer);


out:
    if (skcipher)
        crypto_free_skcipher(skcipher);
    if (req)
        skcipher_request_free(req);
    if (scratchpad)
        kfree(scratchpad);
    return ret;
}


static int decriptar(void __user *buffer){
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	char *scratchpad = NULL;
	char *ivdata = NULL;
	unsigned char keyLocal[33];
	int ret = -EFAULT;
	int i = 0;
	
	skcipher = crypto_alloc_skcipher("ecb(aes)", 0, 0); // ecb eh o tipo de criptacao que estamos usando
	if (IS_ERR(skcipher)) {
		pr_info("could not allocate skcipher handle\n");
		return PTR_ERR(skcipher);
	}

	req = skcipher_request_alloc(skcipher, GFP_KERNEL); 
	if (!req) {
		pr_info("could not allocate skcipher request\n");
		ret = -ENOMEM;
		goto out;
	}

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, test_skcipher_cb, &sk.result);

	
		/* AES 256 with random key */
	strncpy(keyLocal, key, 32); // passando a chave para KeyLocal para ser utilizada na decriptacao
	keyLocal[32] = '\0';
	printk(KERN_INFO "Key Local = %s \n", keyLocal);
	
	if (crypto_skcipher_setkey(skcipher, keyLocal, 32)) { // 32 eh o tamanho da chave
		pr_info("key could not be set\n");
		ret = -EAGAIN;
		goto out;
	}
	
		/* IV will be random */
	ivdata = kmalloc(16, GFP_KERNEL);
	if (!ivdata) {
		pr_info("could not allocate ivdata\n");
		goto out;
	}
	get_random_bytes(ivdata, 16);
	
	/* Input data will be random */
	scratchpad = kmalloc(16, GFP_KERNEL); // scratchpad eh a mensagem que vai ser descriptada, 16 eh o tamanho
	if (!scratchpad) {
		pr_info("could not allocate scratchpad\n");
		goto out;
	}
	memcpy(scratchpad, buffer, 16); // 16 eh o tamanho maximo do buffer, se quiser mais, tem q modificar o programa
	//scratchpad[16] = '\0'; // colocando um \0 no final da string
	sk.tfm = skcipher;
	sk.req = req;
	
	// Decriptar 
	sg_init_one(&sk.sg, scratchpad, 16); // passando a string para decriptar para sk.sg e o tamanho dela
	skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata); // sk.sg sendo decriptografada e o tamanho dela
	init_completion(&sk.result.completion);

		// decrypt data 
	ret = test_skcipher_encdec(&sk, 0); // 1 para criptar e 0 para decriptar
	if (ret)
		goto out;
	
	pr_info("Decryption triggered successfully\n");
	sg_copy_to_buffer(&sk.sg, 1, buffer, 16);
	

out:
	if (skcipher)
		crypto_free_skcipher(skcipher);
	if (req)
		skcipher_request_free(req);
	if (scratchpad)
		kfree(scratchpad);
	return ret;
}
	

const struct inode_operations minix_file_inode_operations = {
	.setattr	= minix_setattr,
	.getattr	= minix_getattr,
};
