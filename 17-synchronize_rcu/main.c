#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/rcupdate.h>

#include <other.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric");
MODULE_DESCRIPTION("The module is only used for test");

#define MAX_KTHREAD  10

static unsigned long reader_bitmap;

static void set_reader_number(int reader)
{
  WARN_ON((MAX_KTHREAD - reader) != 1);
  reader_bitmap = 0;
  
  while (reader){
    reader_bitmap |= (1 <<  --reader); 
  }
}
/*
//static DEFINE_SPINLOCK(threads_lock);
static  spinlock_t threads_lock;

static void threads_lock_init(void)
{
    spin_lock_init(&threads_lock);
}
*/
struct our_data{
  int count1;
  int count2;
  struct rcu_head rhead;
};

static struct our_data my_data; 
static struct our_data __rcu  *pmy_data = &my_data;
static void show_my_data(void)
{
  printk("count1 = %d,count2 = %d.\n",pmy_data->count1,pmy_data->count2);
}

static void reader_do(void)
{
 struct  our_data *data;
 
  rcu_read_lock();
  data  = rcu_dereference(pmy_data);
  printk("read count1 = %d,count2 = %d.\n",data->count1,data->count2);
  rcu_read_unlock(); 
}

static void  rcu_free(struct rcu_head *head)
{
  struct our_data *data;
  
  data = container_of(head,struct our_data,rhead); 
  kfree(data);
}

static void writer_do(void)
{
 struct our_data *data, *tmp = pmy_data;
  
 data = kmalloc(sizeof(*data),GFP_KERNEL);
 if(!data)
   return;
/* read + copy .*/
 memcpy(data,pmy_data,sizeof(*data));
 data->count1++;
 data->count2 += 10;
 
 rcu_assign_pointer(pmy_data,data);
 
 if (tmp != &my_data){
   call_rcu(&tmp->rhead,rcu_free);
   //synchronize_rcu();
   //kfree(tmp);  
  }
}


static struct task_struct *threads[MAX_KTHREAD];

static int thread_do(void *data)
{
 long i =(long)data;
 int reader = (reader_bitmap & (1 << i));
 
 printk("%ld is run...he is %s...\n",i,reader ? "reader" : "writer");
/*
 while(!kthread_should_stop())
 { 
      spin_lock(&threads_lock);
      my_data.count1++;
      my_data.count2 += 10;
      spin_unlock(&threads_lock);

      msleep(10);
 }
*/
 while(!kthread_should_stop())
 {
   if(reader)
    reader_do();
   else 
    writer_do();
   
   msleep(10);
 }

 return 0;
}

static int  creat_threads(void)
{
 int i;
 
 for(i = 0; i < MAX_KTHREAD; i++ )
  {
   struct task_struct *thread;

   thread =  kthread_run(thread_do,(void *)(long)i, "thread-%d",i); 
   if(IS_ERR(thread)){
      return -1;
    }
   
   threads[i] = thread;
   }

 return 0;
}
static void cleanup_threads(void)
{
 int i;
 
 for(i = 0; i < MAX_KTHREAD; i++)
 {
   if(threads[i])
    kthread_stop(threads[i]);
 }

}


static __init int minit(void)
{ 
  printk("call %s.\n", __FUNCTION__);
// threads_lock_init();
  set_reader_number(9);
  
  if(creat_threads())
     goto err;

  return 0;

err:
  cleanup_threads();
  return -1;
}

static __exit void mexit(void)
{
   printk("call %s.\n", __FUNCTION__);
   cleanup_threads();
   show_my_data();
}

module_init(minit)
module_exit(mexit)










