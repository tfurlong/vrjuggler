/*
 * --------------------------------------------------------------------------
 * vjCondPosix.h
 * $Revision$
 * $Date$
 *
 * Author:
 *   Patrick Hartling (based on CondSGI by Allen Bierbaum).
 * --------------------------------------------------------------------------
 * NOTES:
 *    - This file (vjCondPosix.h) must be included by vjCond.h, not the
 *      other way around.
 * --------------------------------------------------------------------------
 */

#ifndef _COND_POSIX_H_
#define _COND_POSIX_H_


#include <vjConfig.h>
#include <iostream.h>
#include <pthread.h>
#include <Sync/vjMutexPosix.h>


//: Condition variable wrapper for POSIX-compliant systems using pthreads
//+ condition variables for the implementation.

class vjCondPosix {
public:
    // -----------------------------------------------------------------------
    //: Constructor for vjCondPosix class.
    //
    //! PRE: None.
    //! POST: The condition variable is intialized, and the mutex variable
    //+       associated with it is defined.  These two steps must be done
    //+       before any other member functions can use them.
    //
    //! ARGS: mutex - Pointer to a vjMutexPosix variable that is used in
    //+               association with the condition variable in this class
    //+               (optional).
    // -----------------------------------------------------------------------
    vjCondPosix (vjMutexPosix* mutex = NULL) {
        // Initialize the condition variable.
#ifdef _PTHREADS_DRAFT_4
        pthread_cond_init(&cond_var, pthread_condattr_default);
#else
        pthread_cond_init(&cond_var, NULL);
#endif

        // If the caller did not specify a mutex variable to use with
        // the condition variable, use defaultMutex.
        if ( mutex == NULL ) {
            mutex = &defaultMutex;
        }

        condMutex = mutex;
    }

    // -----------------------------------------------------------------------
    //: Destructor for vjCondPosix class.
    //
    //! PRE: None.
    //! POST: The condition variable is destroyed.
    // -----------------------------------------------------------------------
    ~vjCondPosix (void) {
        pthread_cond_destroy(&cond_var);
    }

    // -----------------------------------------------------------------------
    //: Block on a condition.
    //
    //! PRE: The mutex variable associated with the condition variable must
    //+      be locked.
    //! POST: The condition variable is locked.  If it was previously
    //+       locked, the caller blocks until signaled.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    int
    wait (void) {
        // ASSERT:  We have been locked

        // If not locked ...
        if ( condMutex->test() == 0 ) {
            cerr << "vjCondPosix::wait: INCORRECT USAGE: Mutex was not "
                 << "locked when wait invoked!!!\n";

            return -1;
        }

        // The mutex variable must be locked when passed to
        // pthread_cond_wait().
        return pthread_cond_wait(&cond_var, condMutex->mutex);
    }

    // -----------------------------------------------------------------------
    //: Signal a thread waiting on the condition variable.
    //
    //! PRE: The condition variable must be locked.
    //! POST: The condition variable is unlocked, and a signal is sent to a
    //+       thread waiting on it.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    signal (void) {
        // ASSERT:  We have been locked
        return pthread_cond_signal(&cond_var);
    }

    // -----------------------------------------------------------------------
    //: Signal all waiting threads.
    //
    //! PRE: The mutex variable associated with teh condition variable
    //+      should be locked.
    //! POST: The condition variable is unlocked, and all waiting threads
    //+       are signaled of this event.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    broadcast (void) {
        // ASSERT:  We have been locked

        // If not locked ...
        if ( condMutex->test() == 0 ) {
            cerr << "vjCondPosix::broadcast: Mutex was not locked when "
                 << "broadcast called!!!\n";
        }

        return pthread_cond_broadcast(&cond_var);
    }

    // -----------------------------------------------------------------------
    //: Acquire a lock on the mutex variable associated with the condition
    //+ variable.
    //
    //! PRE: None.
    //! POST: A lock is acquired on the mutex variable associated with the
    //+      condition variable.  If a lock is acquired, the caller controls
    //+      the mutex variable.  If it was previously locked, the caller
    //+      blocks until it is unlocked.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    acquire (void) {
        return condMutex->acquire();
    }

    // -----------------------------------------------------------------------
    //: Try to aquire a lock on the mutex variable associated with the
    //+ condition variable.
    //
    //! PRE: None.
    //! POST: If the mutex variable is not already locked, the caller
    //+       obtains a lock on it.  If it is already locked, the routine
    //+       returns immediately to the caller.
    //
    //! RETURNS:  0 - Successful completion
    //! RETURNS: -1 - Error
    // -----------------------------------------------------------------------
    inline int
    tryAcquire (void) {
        return condMutex->tryAcquire();
    }

    // -----------------------------------------------------------------------
    //: Release the lock on the mutex variable associated with the condition
    //+ variable.
    //
    //! PRE: None.
    //! POST: The lock held by the caller on the mutex variable is released.
    // -----------------------------------------------------------------------
    inline int
    release (void) {
        return condMutex->release();
    }

    // -----------------------------------------------------------------------
    //: Change the condition variable mutex to be the specifiec mutex
    //+ variable.
    //
    //! PRE: The specified mutex variable must be initialized.
    //! POST: The condition variable associated with the mutex variable is
    //+       reset to the specified variable.
    //
    //! ARGS: mutex - Pointer to a vjMutexPosix variable that is used in
    //+               association with the condition variable in this class.
    //
    //! NOTE: NEVER call except to initialize explicitly.
    // -----------------------------------------------------------------------
    inline void
    setMutex (vjMutexPosix* mutex) {
        // NOT exactly correct, but just make sure not to leave it locked
        mutex->release();
        condMutex = mutex;
    }

    // -----------------------------------------------------------------------
    //: Print out information about the condition variable to stderr.
    //
    //! PRE: None.
    //! POST: All important data and debugging information related to the
    //+       condition variable and its mutex are dumped to stderr.
    // -----------------------------------------------------------------------
    void
    dump (void) const {
        cerr << "------------- vjCondPosix::Dump ---------\n"
             << "Not Implemented yet.\n";
    }

private:
    pthread_cond_t	cond_var;	//: Condition variable
    vjMutexPosix*	condMutex;	//: Mutex for the condition variable
    vjMutexPosix	defaultMutex;	//: A default mutex variable

    // = Prevent assignment and initialization.
    void operator= (const vjCondPosix&) {}
    vjCondPosix (const vjCondPosix &c) {}
};

#endif	/* _COND_POSIX_H_ */
