/*
 Alex Venturini  - RA: 15294739 
 Luan Bonomi - RA: 15108780
 Pedro Catalini - RA: 15248354
 Matheus Nishida - RA: 12212692
 Daniel Toloto  - RA: 16436065	
 Leonardo Guissi - RA: 15108244
*/

/**
 * @file   ebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief   An introductory character driver to support the second article of my series on
 * Linux loadable kernel module (LKM) development. This module maps to /dev/ebbchar and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 */

/*
	Verificar essas bibliotecas
	linux/string
	linux/crypto
*/


#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function

/////////////////////////////////////

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



#define  DEVICE_NAME "ebbchar"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "ebb"        ///< The device class -- this is a character device driver


static char *key = "chave";

module_param(key, charp, 0000);
MODULE_PARM_DESC(key, "A character string");

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Derek Molloy");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the BBB");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static struct class*  ebbcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int criptar(char* buffer, size_t len);
static int decriptar(char *buffer, size_t len);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .read = dev_read,
   .write = dev_write,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init ebbchar_init(void){
   printk(KERN_INFO "EBBChar: Initializing the EBBChar LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "EBBChar failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(ebbcharClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "EBBChar: device class registered correctly\n");

   // Register the device driver
   ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(ebbcharDevice)){               // Clean up if there is an error
      class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(ebbcharDevice);
   }
   printk(KERN_INFO "EBBChar: device class created correctly\n"); // Made it! device was initialized
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebbchar_exit(void){
   device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(ebbcharClass);                          // unregister the device class
   class_destroy(ebbcharClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

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


/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
     len virou opcao
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){            // if true then have success
      //printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      //printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
     len virou opcao
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
     char kernelBuffer[len];
     char dadosBuffer[len - 4];
     int i = 0;
     copy_from_user(kernelBuffer, buffer, len);
     //strcpy(kernelBuffer, buffer[2]);
   sprintf(message, "%s(%zu lettersAqui eh a resposta!)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
	
	for(i = 0; i<(len - 4); i++){     // c "oi"
             dadosBuffer[i] = kernelBuffer[i + 3];
	}
	dadosBuffer[i] = '\0';
	
	//printk(KERN_INFO "kernelBuff = %s", dadosBuffer);
	
	//printk(KERN_INFO "EBBChar: ANTES DO IF...\n");
	
     if(kernelBuffer[0] == 'c') { // se for igual a c, o usuario escolheu a opcao Cifrar
           sprintf(message, "\n\nOP1\n\n");
	  criptar(dadosBuffer, strlen(dadosBuffer));
     } 
     if(kernelBuffer[0] == 'd') { // se for igual a d, o usuario escolheu a opcao Decodificar
          sprintf(message, "\n\nOP2\n\n");
          decriptar(dadosBuffer, strlen(dadosBuffer));
     } 
     if(kernelBuffer[0] == 'h') { // se for igual a h, o usuario escolheu a opcao Hash   
          sprintf(message, "\n\nOP3\n\n");
          //calc_hash(dadosBuffer, strlen(dadosBuffer));
     }
     

   return len;
}


static int decriptar(char *buffer, size_t len){
struct skcipher_def sk;
    struct crypto_skcipher *skcipher = NULL;
    struct skcipher_request *req = NULL;
    char *scratchpad = NULL;
    unsigned char keyLocal[32];
    char retorno2[16];
    int ret = -EFAULT;
    int i = 0;

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
    get_random_bytes(&keyLocal, 32);
    strcpy(keyLocal, key);
    
    if (crypto_skcipher_setkey(skcipher, keyLocal, 32)) {
        pr_info("key could not be set\n");
        ret = -EAGAIN;
        goto out;
    }
   
    /* Input data will be random */
    scratchpad = kmalloc(16, GFP_KERNEL);
    if (!scratchpad) {
        pr_info("could not allocate scratchpad\n");
        goto out;
    }
    strcpy(scratchpad, buffer);
	
    sk.tfm = skcipher;
    sk.req = req;
    /* We encrypt one block */
    sg_init_one(&sk.sg, scratchpad, 16);
    skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, NULL);
    init_completion(&sk.result.completion);

    /* Dencrypt data */
    ret = test_skcipher_encdec(&sk, 0); // 1 para criptar e 0 para decriptar
    if (ret)
        goto out;

    pr_info("Decryption triggered successfully\n");

    sg_copy_to_buffer(&sk.sg, 1, &retorno2, 16);
    //printk(KERN_INFO "Retorno 2 = %s \n", retorno2); // posicao da memoria
    for(i = 0; i < 16; i++)
	printk(KERN_INFO "Decriptar: %02x\n", (unsigned char)retorno2[i]);
	
    printk(KERN_INFO "Decriptar: %s\n", retorno2);

out:
    if (skcipher)
        crypto_free_skcipher(skcipher);
    if (req)
        skcipher_request_free(req);
    if (scratchpad)
        kfree(scratchpad);
    return ret;
}

static int criptar(char *buffer, size_t len){
    struct skcipher_def sk;
    struct crypto_skcipher *skcipher = NULL;
    struct skcipher_request *req = NULL;
    char *scratchpad = NULL;
    unsigned char keyLocal[32];
    char retorno[16];
    char retorno2[16];
    int ret = -EFAULT;
    int i = 0;

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
    get_random_bytes(&keyLocal, 32);
    strcpy(keyLocal, key);
    printk(KERN_INFO "Key Local = %s \n", keyLocal);
    
    if (crypto_skcipher_setkey(skcipher, keyLocal, 32)) {
        pr_info("key could not be set\n");
        ret = -EAGAIN;
        goto out;
    }
   
   
    /* Input data will be random */
    scratchpad = kmalloc(16, GFP_KERNEL);
    if (!scratchpad) {
        pr_info("could not allocate scratchpad\n");
        goto out;
    }
    strcpy(scratchpad, buffer);
	
    sk.tfm = skcipher;
    sk.req = req;
    
    printk(KERN_INFO "Original: %s\n", scratchpad);
    for(i = 0; i < 16; i++)
	printk(KERN_INFO "Original: %02x\n", (unsigned char)scratchpad[i]);
    
    /* We encrypt one block */
    sg_init_one(&sk.sg, scratchpad, 16);
    skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, NULL);
    init_completion(&sk.result.completion);

    /* encrypt data */
    ret = test_skcipher_encdec(&sk, 1); // 1 para criptar e 0 para decriptar
    if (ret)
        goto out; 
    
    
    pr_info("Encryption triggered successfully\n");
    sg_copy_to_buffer(&sk.sg, 1, &retorno, 16);

    for(i = 0; i < 16; i++)
	printk(KERN_INFO "Criptar: %02x\n", (unsigned char)retorno[i]);
	//sprintf(message, "Criptar: %02x\n", (unsigned char)retorno[i]);

	
	
	
	// Decriptar 
    sg_init_one(&sk.sg, scratchpad, 16);
    skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, NULL);
    init_completion(&sk.result.completion);

     // decrypt data 
    ret = test_skcipher_encdec(&sk, 0); // 1 para criptar e 0 para decriptar
    if (ret)
        goto out;
    
    pr_info("Decryption triggered successfully\n");
    sg_copy_to_buffer(&sk.sg, 1, &retorno2, 16);
    for(i = 0; i < 16; i++)
	printk(KERN_INFO "Decriptar: %02x\n", (unsigned char)retorno2[i]);
	
    printk(KERN_INFO "Decriptar: %s\n", retorno2);

out:
    if (skcipher)
        crypto_free_skcipher(skcipher);
    if (req)
        skcipher_request_free(req);
    if (scratchpad)
        kfree(scratchpad);
    return ret;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(ebbchar_init);
module_exit(ebbchar_exit);
