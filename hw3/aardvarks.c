#include <semaphore.h>
#include <pthread.h>
#include "/comp/111/assignments/aardvarks/anthills.h"

#define ANTHILLS 3

// Representation of an anthill, with a semaphore and a mutex to control the
// threads accessing it.
typedef struct
{
    sem_t sem;
    pthread_mutex_t mutex;
    int ants_left;
} Anthill;

static Anthill hills[ANTHILLS];
static int initialized = 0;

// Semaphore with a set time to release it.
typedef struct
{
    sem_t *sem;
    double release_time;
} SemAndRelease;

/* sem_releaser
 * Description: handles releasing the semaphore associated with an anthill,
                will be passed to a thread as the function call for the thread.
 * Input: SemAndRelease (void * arg) - a semaphore and a release time
 * Output: none
 */
void *sem_releaser(void *arg)
{
    SemAndRelease *args = (SemAndRelease *)arg;
    double now = elapsed();

    if (now < args->release_time)
    {
        usleep((useconds_t)((args->release_time - now) * 1000000));
    }

    sem_post(args->sem);
    free(args);
    return NULL;
}

/* my_slurp
 * Description: Wrapper for the slurp function to handle releasing the anthill
 *              semaphore before the slurp function returns.
 * Parameters: char name -- name of the ardvark at the anthill
 *             int idx -- index of the anthill
 * Returns: int indicating the return result of the slurp function (1 if
 *          success and 0 if failure)
 */
int threaded_slurp(char name, int idx)
{
    // Start a timer
    double start_time = elapsed();

    // Initialize a new thread
    pthread_t helper;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // In detached state, so we don't need to join
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Initialize helper args, with sem of hill and time we can release the
    // semaphore.
    SemAndRelease *args = malloc(sizeof(SemAndRelease));
    args->sem = &hills[idx].sem;
    args->release_time = start_time + 1.05;

    // Create thread with the releaser function, to release the semaphore 1
    // second from right now
    pthread_create(&helper, &attr, sem_releaser, args);

    // Begin the slurp
    int result = slurp(name, idx);

    // Return when done with slurp
    return result;
}

void *aardvark(void *input)
{
    char name = *(char *)input;

    pthread_mutex_lock(&init_lock);
    if (!initialized++)
    {
        for (int i = 0; i < ANTHILLS; i++)
        {
            sem_init(&hills[i].sem, 0, AARDVARKS_PER_HILL);
            pthread_mutex_init(&hills[i].mutex, NULL);
            hills[i].ants_left = ANTS_PER_HILL;
        }
    }
    pthread_mutex_unlock(&init_lock);

    int start = rand() % ANTHILLS;

    while (chow_time())
    {
        for (int offset = 0; offset < ANTHILLS; offset++)
        {
            int idx = (start + offset) % ANTHILLS;

            pthread_mutex_lock(&hills[idx].mutex);
            int available = hills[idx].ants_left > 0;
            pthread_mutex_unlock(&hills[idx].mutex);

            // If hill not available skip to next iteration
            if (!available)
                continue;

            // If hill available, try to get the semaphore
            if (sem_trywait(&hills[idx].sem) != 0)
                continue;

            // If we could get the semaphore, lock the hill and
            pthread_mutex_lock(&hills[idx].mutex);
            // Check if there are no ants left
            if (hills[idx].ants_left <= 0)
            {
                pthread_mutex_unlock(&hills[idx].mutex);
                sem_post(&hills[idx].sem);
                continue;
            }

            // decrease ants and unlock mutex for hill
            hills[idx].ants_left--;
            pthread_mutex_unlock(&hills[idx].mutex);

            // Call my_slurp function
            int result = threaded_slurp(name, idx);
            // If it fails, put ant back on hill
            if (!result)
            {
                pthread_mutex_lock(&hills[idx].mutex);
                hills[idx].ants_left++;
                pthread_mutex_unlock(&hills[idx].mutex);
            }
        }
    }
    return NULL;
}