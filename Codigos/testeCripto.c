/*
 *  hello-5.c - Demonstrates command line argument passing to a module.
 
 * Alex Venturini  - RA: 15294739
 * Luan Bonomi - RA: 15108780
 * Pedro Catalini - RA: 15248354
 * Matheus Nishida - RA: 12212692
 * Daniel Toloto  - RA: 16436065	
 * Leonardo Guissi - RA: 15108244
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

MODULE_LICENSE("GPL");


static char *chave = "";

module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, "A character string");



static int __init hello_5_init(void)
{
    printk(KERN_INFO "mystring is a string: %s\n", mystring);
    struct tcrypt_result {
		struct completion completion;
		int err;
	};

/* tie all data structures together */
	struct skcipher_def {
		struct scatterlist sg;
		struct crypto_skcipher *tfm;
		struct skcipher_request *req;
		struct tcrypt_result result;
	};

/* Callback function */
	static void test_skcipher_cb(struct crypto_async_request *req, int error)
	{
		struct tcrypt_result *result = req->data;
	
		if (error == -EINPROGRESS)
			return;
		result->err = error;
		complete(&result->completion);
		pr_info("Encryption finished successfully\n");
	}
	
	/* Perform cipher operation */
	static unsigned int test_skcipher_encdec(struct skcipher_def *sk,
						int enc)
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
			rc = wait_for_completion_interruptible(&sk->result.completion);
			if (!rc && !sk->result.err) {
				reinit_completion(&sk->result.completion);
				break;
			}
		default:
			pr_info("skcipher encrypt returned with %d result %d\n",rc, sk->result.err);
			break;
		}
		init_completion(&sk->result.completion);
	
		return rc;
	}
	
	/* Initialize and trigger cipher operation */
	static int test_skcipher(void)
	{
		struct skcipher_def sk;
		struct crypto_skcipher *skcipher = NULL;
		struct skcipher_request *req = NULL;
		char *scratchpad = NULL;
		char *ivdata = NULL;
		unsigned char key[32];
		int ret = -EFAULT;
	
		skcipher = crypto_alloc_skcipher("aes-aesni", 0, 0);
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
	
		skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG,
						test_skcipher_cb,
						&sk.result);
	
	
		if (crypto_skcipher_setkey(skcipher, chave, 32)) {
			pr_info("key could not be set\n");
			ret = -EAGAIN;
			goto out;
		}
	
		/* IV will be STATIC! */
		ivdata = kmalloc(16, GFP_KERNEL);
		if (!ivdata) {
			pr_info("could not allocate ivdata\n");
			goto out;
		}
		ivdata = 256;
		printk(KERN_INFO "ivdata: %s\n", ivdata);
	
		/* Input data will be random */
		scratchpad = kmalloc(16, GFP_KERNEL);
		if (!scratchpad) {
			pr_info("could not allocate scratchpad\n");
			goto out;
		}
		scratchpad = "teste";
		printk(KERN_INFO "scratchad: %s\n", scratchad);
		
		sk.tfm = skcipher;
		sk.req = req;
	
		/* We encrypt one block */
		sg_init_one(&sk.sg, scratchpad, 16);
		skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, ivdata);
		init_completion(&sk.result.completion);
	
		/* encrypt data */
		ret = test_skcipher_encdec(&sk, 1);
		if (ret)
			goto out;
	
		pr_info("Encryption triggered successfully\n");
	
	out:
		if (skcipher)
			crypto_free_skcipher(skcipher);
		if (req)
			skcipher_request_free(req);
		if (ivdata)
			kfree(ivdata);
		if (scratchpad)
			kfree(scratchpad);
		return ret;
	}
    return 0;
}

static void __exit hello_5_exit(void)
{
    printk(KERN_INFO "Goodbye, world 5\n");
}

module_init(hello_5_init);
module_exit(hello_5_exit);
