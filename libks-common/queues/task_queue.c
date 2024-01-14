/* Copyright  (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (task_queue.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <queues/task_queue.h>

#include <features/features_cpu.h>

#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#define SLOCK_LOCK(x) slock_lock(x)
#define SLOCK_UNLOCK(x) slock_unlock(x)
#else
#define SLOCK_LOCK(x)
#define SLOCK_UNLOCK(x)
#endif

typedef struct
{
   ks_task_t *front;
   ks_task_t *back;
} task_queue_t;

struct ks_task_impl
{
   ks_task_queue_msg_t msg_push;
   void (*push_running)(ks_task_t *);
   void (*cancel)(void *);
   void (*reset)(void);
   void (*wait)(ks_task_condition_fn_t, void *);
   void (*gather)(void);
   bool (*find)(ks_task_finder_t, void*);
   void (*retrieve)(task_retriever_data_t *data);
   void (*init)(void);
   void (*deinit)(void);
};

/* TODO/FIXME - static globals */
static ks_task_queue_msg_t msg_push_bak  = NULL;
static task_queue_t tasks_running           = {NULL, NULL};
static task_queue_t tasks_finished          = {NULL, NULL};

static struct ks_task_impl *impl_current = NULL;
static bool task_threaded_enable            = false;

#ifdef HAVE_THREADS
static slock_t *running_lock                = NULL;
static slock_t *finished_lock               = NULL;
static slock_t *property_lock               = NULL;
static slock_t *queue_lock                  = NULL;
static scond_t *worker_cond                 = NULL;
static sthread_t *worker_thread             = NULL;
static bool worker_continue                 = true; 
/* use running_lock when touching it */
#endif

static void task_queue_msg_push(ks_task_t *task,
      unsigned prio, unsigned duration,
      bool flush, const char *fmt, ...)
{
   char buf[1024];
   va_list ap;

   buf[0] = '\0';

   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);

   if (impl_current->msg_push)
      impl_current->msg_push(task, buf, prio, duration, flush);
}

static void task_queue_push_progress(ks_task_t *task)
{
   if (task->title && !task->mute)
   {
      if (task->finished)
      {
         if (task->error)
            task_queue_msg_push(task, 1, 60, true, "%s: %s",
               "Task failed", task->title);
         else
            task_queue_msg_push(task, 1, 60, false, "100%%: %s", task->title);
      }
      else
      {
         if (task->progress >= 0 && task->progress <= 100)
            task_queue_msg_push(task, 1, 60, true, "%i%%: %s",
                  task->progress, task->title);
         else
            task_queue_msg_push(task, 1, 60, false, "%s...", task->title);
      }

      if (task->progress_cb)
         task->progress_cb(task);
   }
}

static void task_queue_put(task_queue_t *queue, ks_task_t *task)
{
   task->next                   = NULL;

   if (queue->front)
   {
      /* Make sure to insert in order - the queue is 
       * sorted by 'when' so items that aren't scheduled
       * to run immediately are at the back of the queue. 
       * Items with the same 'when' are inserted after
       * all the other items with the same 'when'. 
       * This primarily affects items with a 'when' of 0.
       */
      if (queue->back)
      {
         if (queue->back->when > task->when)
         {
            ks_task_t** prev = &queue->front;
            while (*prev && (*prev)->when <= task->when)
               prev             = &((*prev)->next);

            task->next          = *prev;
            *prev               = task;
            return;
         }

         queue->back->next      = task;
      }
   }
   else
      queue->front              = task;

   queue->back                  = task;
}

static ks_task_t *task_queue_get(task_queue_t *queue)
{
   ks_task_t *task = queue->front;

   if (task)
   {
      queue->front = task->next;
      task->next   = NULL;
   }

   return task;
}

static void ks_task_internal_gather(void)
{
   ks_task_t *task = NULL;
   while ((task = task_queue_get(&tasks_finished)))
   {
      task_queue_push_progress(task);

      if (task->callback)
         task->callback(task, task->task_data, task->user_data, task->error);

      if (task->cleanup)
          task->cleanup(task);

      if (task->error)
         free(task->error);

      if (task->title)
         free(task->title);

      free(task);
   }
}

static void ks_task_regular_push_running(ks_task_t *task)
{
   task_queue_put(&tasks_running, task);
}

static void ks_task_regular_cancel(void *task)
{
   ks_task_t *t = (ks_task_t*)task;
   t->cancelled    = true;
}

static void ks_task_regular_gather(void)
{
   ks_task_t *task  = NULL;
   ks_task_t *queue = NULL;
   ks_task_t *next  = NULL;

   while ((task = task_queue_get(&tasks_running)))
   {
      task->next = queue;
      queue = task;
   }

   for (task = queue; task; task = next)
   {
      next = task->next;

      if (!task->when || task->when < cpu_features_get_time_usec())
      {
         task->handler(task);

         task_queue_push_progress(task);
      }

      if (task->finished)
         task_queue_put(&tasks_finished, task);
      else
         ks_task_regular_push_running(task);
   }

   ks_task_internal_gather();
}

static void ks_task_regular_wait(ks_task_condition_fn_t cond, void* data)
{
   while ((tasks_running.front && !tasks_running.front->when) && (!cond || cond(data)))
      ks_task_regular_gather();
}

static void ks_task_regular_reset(void)
{
   ks_task_t *task = tasks_running.front;

   for (; task; task = task->next)
      task->cancelled = true;
}

static void ks_task_regular_init(void) { }
static void ks_task_regular_deinit(void) { }

static bool ks_task_regular_find(ks_task_finder_t func, void *user_data)
{
   ks_task_t *task = tasks_running.front;

   for (; task; task = task->next)
   {
      if (func(task, user_data))
         return true;
   }

   return false;
}

static void ks_task_regular_retrieve(task_retriever_data_t *data)
{
   ks_task_t *task          = NULL;
   task_retriever_info_t *tail = NULL;

   /* Parse all running tasks and handle matching handlers */
   for (task = tasks_running.front; task != NULL; task = task->next)
   {
      task_retriever_info_t *info = NULL;
      if (task->handler != data->handler)
         continue;

      /* Create new link */
      info       = (task_retriever_info_t*)
         malloc(sizeof(task_retriever_info_t));
      info->data = malloc(data->element_size);
      info->next = NULL;

      /* Call retriever function and fill info-specific data */
      if (!data->func(task, info->data))
      {
         free(info->data);
         free(info);
         continue;
      }

      /* Add link to list */
      if (data->list)
      {
         if (tail)
         {
            tail->next = info;
            tail       = tail->next;
         }
         else
            tail       = info;
      }
      else
      {
         data->list    = info;
         tail          = data->list;
      }
   }
}

static struct ks_task_impl impl_regular = {
   NULL,
   ks_task_regular_push_running,
   ks_task_regular_cancel,
   ks_task_regular_reset,
   ks_task_regular_wait,
   ks_task_regular_gather,
   ks_task_regular_find,
   ks_task_regular_retrieve,
   ks_task_regular_init,
   ks_task_regular_deinit
};

#ifdef HAVE_THREADS

/* 'queue_lock' must be held for the duration of this function */
static void task_queue_remove(task_queue_t *queue, ks_task_t *task)
{
   ks_task_t     *t = NULL;
   ks_task_t *front = NULL;

   front = queue->front;

   /* Remove first element if needed */
   if (task == front)
   {
      queue->front = task->next;
      if (queue->back == task) /* if only element, also update back */
         queue->back = NULL;
      task->next   = NULL;
      return;
   }

   /* Parse queue */
   t = front;

   while (t && t->next)
   {
      /* Remove task and update queue */
      if (t->next == task)
      {
         t->next    = task->next;
         task->next = NULL;

         /* When removing the tail of the queue, update the tail pointer */
         if (queue->back == task)
         {
            if (queue->back == task)
               queue->back = t;
         }
         break;
      }

      /* Update iterator */
      t = t->next;
   }
}

static void ks_task_threaded_push_running(ks_task_t *task)
{
   slock_lock(running_lock);
   slock_lock(queue_lock);
   task_queue_put(&tasks_running, task);
   scond_signal(worker_cond);
   slock_unlock(queue_lock);
   slock_unlock(running_lock);
}

static void ks_task_threaded_cancel(void *task)
{
   ks_task_t *t;

   slock_lock(running_lock);

   for (t = tasks_running.front; t; t = t->next)
   {
      if (t == task)
      {
        t->cancelled = true;
        break;
      }
   }

   slock_unlock(running_lock);
}

static void ks_task_threaded_gather(void)
{
   ks_task_t *task = NULL;

   slock_lock(property_lock);
   slock_lock(running_lock);
   for (task = tasks_running.front; task; task = task->next)
      task_queue_push_progress(task);

   slock_unlock(running_lock);

   slock_lock(finished_lock);
   ks_task_internal_gather();
   slock_unlock(finished_lock);
   slock_unlock(property_lock);
}

static void ks_task_threaded_wait(ks_task_condition_fn_t cond, void* data)
{
   bool wait = false;

   do
   {
      ks_task_threaded_gather();

      slock_lock(running_lock);
      wait = (tasks_running.front && !tasks_running.front->when);
      slock_unlock(running_lock);
   } while (wait && (!cond || cond(data)));
}

static void ks_task_threaded_reset(void)
{
   ks_task_t *task = NULL;

   slock_lock(running_lock);
   for (task = tasks_running.front; task; task = task->next)
      task->cancelled = true;
   slock_unlock(running_lock);
}

static bool ks_task_threaded_find(
      ks_task_finder_t func, void *user_data)
{
   ks_task_t *task = NULL;
   bool        result = false;

   slock_lock(running_lock);
   for (task = tasks_running.front; task; task = task->next)
   {
      if (func(task, user_data))
      {
         result = true;
         break;
      }
   }
   slock_unlock(running_lock);

   return result;
}

static void ks_task_threaded_retrieve(task_retriever_data_t *data)
{
   /* Protect access to running tasks */
   slock_lock(running_lock);

   /* Call regular retrieve function */
   ks_task_regular_retrieve(data);

   /* Release access to running tasks */
   slock_unlock(running_lock);
}

static void threaded_worker(void *userdata)
{
   (void)userdata;

   for (;;)
   {
      ks_task_t *task  = NULL;
      bool       finished = false;

      if (!worker_continue)
         break; /* should we keep running until all tasks finished? */

      slock_lock(running_lock);

      /* Get first task to run */
      task = tasks_running.front;
      if (!task)
      {
         scond_wait(worker_cond, running_lock);
         slock_unlock(running_lock);
         continue;
      }

      if (task->when)
      {
         ks_time_t now   = cpu_features_get_time_usec();
         ks_time_t delay = task->when - now - 500; /* allow half a millisecond for context switching */
         if (delay > 0)
         {
            scond_wait_timeout(worker_cond, running_lock, delay);
            slock_unlock(running_lock);
            continue;
         }
      }

      slock_unlock(running_lock);

      task->handler(task);

      slock_lock(property_lock);
      finished = task->finished;
      slock_unlock(property_lock);

      /* Update queue */
      if (!finished)
      {
         /* Move the task to the back of the queue */
         /* mimics ks_task_threaded_push_running, 
          * but also includes a task_queue_remove */
         slock_lock(running_lock);
         slock_lock(queue_lock);

         /* do nothing if only item in queue */
         if (task->next) 
         {
            task_queue_remove(&tasks_running, task);
            task_queue_put(&tasks_running, task);
            scond_signal(worker_cond);
         }
         slock_unlock(queue_lock);
         slock_unlock(running_lock);
      }
      else
      {
         /* Remove task from running queue */
         slock_lock(running_lock);
         slock_lock(queue_lock);
         task_queue_remove(&tasks_running, task);
         slock_unlock(queue_lock);
         slock_unlock(running_lock);

         /* Add task to finished queue */
         slock_lock(finished_lock);
         task_queue_put(&tasks_finished, task);
         slock_unlock(finished_lock);
      }
   }
}

static void ks_task_threaded_init(void)
{
   running_lock    = slock_new();
   finished_lock   = slock_new();
   property_lock   = slock_new();
   queue_lock      = slock_new();
   worker_cond     = scond_new();

   slock_lock(running_lock);
   worker_continue = true;
   slock_unlock(running_lock);

   worker_thread   = sthread_create(threaded_worker, NULL);
}

static void ks_task_threaded_deinit(void)
{
   slock_lock(running_lock);
   worker_continue = false;
   scond_signal(worker_cond);
   slock_unlock(running_lock);

   sthread_join(worker_thread);

   scond_free(worker_cond);
   slock_free(running_lock);
   slock_free(finished_lock);
   slock_free(property_lock);
   slock_free(queue_lock);

   worker_thread   = NULL;
   worker_cond     = NULL;
   running_lock    = NULL;
   finished_lock   = NULL;
   property_lock   = NULL;
   queue_lock      = NULL;
}

static struct ks_task_impl impl_threaded = {
   NULL,
   ks_task_threaded_push_running,
   ks_task_threaded_cancel,
   ks_task_threaded_reset,
   ks_task_threaded_wait,
   ks_task_threaded_gather,
   ks_task_threaded_find,
   ks_task_threaded_retrieve,
   ks_task_threaded_init,
   ks_task_threaded_deinit
};
#endif

/* Deinitializes the task system.
 * This deinitializes the task system.
 * The tasks that are running at
 * the moment will stay on hold */
void task_queue_deinit(void)
{
   if (impl_current)
      impl_current->deinit();
   impl_current = NULL;
}

void task_queue_init(bool threaded, ks_task_queue_msg_t msg_push)
{
   impl_current = &impl_regular;

#ifdef HAVE_THREADS
   if (threaded)
   {
      task_threaded_enable = true;
      impl_current         = &impl_threaded;
   }
#endif

   msg_push_bak            = msg_push;

   impl_current->msg_push  = msg_push;
   impl_current->init();
}

void task_queue_set_threaded(void)
{
   task_threaded_enable = true;
}

void task_queue_unset_threaded(void)
{
   task_threaded_enable = false;
}

bool task_queue_is_threaded(void)
{
   return task_threaded_enable;
}

bool task_queue_find(task_finder_data_t *find_data)
{
   if (!impl_current->find(find_data->func, find_data->userdata))
      return false;
   return true;
}

void task_queue_retrieve(task_retriever_data_t *data)
{
   impl_current->retrieve(data);
}

void task_queue_check(void)
{
#ifdef HAVE_THREADS
   bool current_threaded = (impl_current == &impl_threaded);
   bool want_threaded    = task_threaded_enable;

   if (want_threaded != current_threaded)
      task_queue_deinit();

   if (!impl_current)
      task_queue_init(want_threaded, msg_push_bak);
#endif

   impl_current->gather();
}

bool task_queue_push(ks_task_t *task)
{
   /* Ignore this task if a related one is already running */
   if (task->type == TASK_TYPE_BLOCKING)
   {
      ks_task_t *running = NULL;
      bool            found = false;

      SLOCK_LOCK(queue_lock);
      running = tasks_running.front;

      for (; running; running = running->next)
      {
         if (running->type == TASK_TYPE_BLOCKING)
         {
            found = true;
            break;
         }
      }

      SLOCK_UNLOCK(queue_lock);

      /* skip this task, user must try again later */
      if (found)
         return false;
   }

   /* The lack of NULL checks in the following functions
    * is proposital to ensure correct control flow by the users. */
   impl_current->push_running(task);

   return true;
}

void task_queue_wait(ks_task_condition_fn_t cond, void* data)
{
   impl_current->wait(cond, data);
}

void task_queue_reset(void)
{
   impl_current->reset();
}

/**
 * Signals a task to end without waiting for
 * it to complete. */
void task_queue_cancel_task(void *task)
{
   impl_current->cancel(task);
}

void *task_queue_retriever_info_next(task_retriever_info_t **link)
{
   void *data = NULL;

   /* Grab data and move to next link */
   if (*link)
   {
      data  = (*link)->data;
      *link = (*link)->next;
   }

   return data;
}

void task_queue_retriever_info_free(task_retriever_info_t *list)
{
   task_retriever_info_t *info;

   /* Free links including retriever-specific data */
   while (list)
   {
      info = list->next;
      free(list->data);
      free(list);
      list = info;
   }
}

void task_set_finished(ks_task_t *task, bool finished)
{
   SLOCK_LOCK(property_lock);
   task->finished = finished;
   SLOCK_UNLOCK(property_lock);
}

void task_set_mute(ks_task_t *task, bool mute)
{
   SLOCK_LOCK(property_lock);
   task->mute = mute;
   SLOCK_UNLOCK(property_lock);
}

void task_set_error(ks_task_t *task, char *error)
{
   SLOCK_LOCK(property_lock);
   task->error = error;
   SLOCK_UNLOCK(property_lock);
}

void task_set_progress(ks_task_t *task, int8_t progress)
{
   SLOCK_LOCK(property_lock);
   task->progress = progress;
   SLOCK_UNLOCK(property_lock);
}

void task_set_title(ks_task_t *task, char *title)
{
   SLOCK_LOCK(property_lock);
   task->title = title;
   SLOCK_UNLOCK(property_lock);
}

void task_set_data(ks_task_t *task, void *data)
{
   SLOCK_LOCK(running_lock);
   task->task_data = data;
   SLOCK_UNLOCK(running_lock);
}

void task_set_cancelled(ks_task_t *task, bool cancelled)
{
   SLOCK_LOCK(running_lock);
   task->cancelled = cancelled;
   SLOCK_UNLOCK(running_lock);
}

void task_free_title(ks_task_t *task)
{
   SLOCK_LOCK(property_lock);
   if (task->title)
      free(task->title);
   task->title = NULL;
   SLOCK_UNLOCK(property_lock);
}

void* task_get_data(ks_task_t *task)
{
   void *data = NULL;

   SLOCK_LOCK(running_lock);
   data = task->task_data;
   SLOCK_UNLOCK(running_lock);

   return data;
}

bool task_get_cancelled(ks_task_t *task)
{
   bool cancelled = false;

   SLOCK_LOCK(running_lock);
   cancelled = task->cancelled;
   SLOCK_UNLOCK(running_lock);

   return cancelled;
}

bool task_get_finished(ks_task_t *task)
{
   bool finished = false;

   SLOCK_LOCK(property_lock);
   finished = task->finished;
   SLOCK_UNLOCK(property_lock);

   return finished;
}

bool task_get_mute(ks_task_t *task)
{
   bool mute = false;

   SLOCK_LOCK(property_lock);
   mute = task->mute;
   SLOCK_UNLOCK(property_lock);

   return mute;
}

char* task_get_error(ks_task_t *task)
{
   char *error = NULL;

   SLOCK_LOCK(property_lock);
   error = task->error;
   SLOCK_UNLOCK(property_lock);

   return error;
}

int8_t task_get_progress(ks_task_t *task)
{
   int8_t progress = 0;

   SLOCK_LOCK(property_lock);
   progress = task->progress;
   SLOCK_UNLOCK(property_lock);

   return progress;
}

char* task_get_title(ks_task_t *task)
{
   char *title = NULL;

   SLOCK_LOCK(property_lock);
   title = task->title;
   SLOCK_UNLOCK(property_lock);

   return title;
}

ks_task_t *task_init(void)
{
   /* TODO/FIXME - static local global */
   static uint32_t task_count = 0;
   ks_task_t *task         = (ks_task_t*)malloc(sizeof(*task));

   if (!task)
      return NULL;

   task->handler           = NULL;
   task->callback          = NULL;
   task->cleanup           = NULL;
   task->finished          = false;
   task->cancelled         = false;
   task->mute              = false;
   task->task_data         = NULL;
   task->user_data         = NULL;
   task->state             = NULL;
   task->error             = NULL;
   task->progress          = 0;
   task->progress_cb       = NULL;
   task->title             = NULL;
   task->type              = TASK_TYPE_NONE;
   task->ident             = task_count++;
   task->frontend_userdata = NULL;
   task->alternative_look  = false;
   task->next              = NULL;
   task->when              = 0;

   return task;
}