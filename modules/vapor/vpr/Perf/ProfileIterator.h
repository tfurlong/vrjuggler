#ifndef VPR_PROFILE_ITERATOR_H
#define VPR_PROFILE_ITERATOR_H

/**
 * mostly taken from:
 * Real-Time Hierarchical Profiling for Game Programming Gems 3
 * by Greg Hjelstrom & Byon Garrabrant
 */

#include <vpr/Perf/ProfileNode.h>

namespace vpr
{
   
/**
 * An iterator to navigate through the Profile tree
 */
class ProfileIterator
{
public:
	/**
    * Access the first child of the parent.
    */
	void				first(void);

   /**
    * Access the next child of the parent.
    */
	void				next(void);

   /**
    * Tells if the there are no more children to iterate through.
    *
    * @return true is returned if the next child is NULL.
    *         false is returned if there is a valid next child.
    */
	bool				isDone(void);

   /**
    * Make the given child the new parent.
    */
	void				enterChild( int index );

   /**
    * Make the largest child the new parent.
    */
	void				enterLargestChild( void );

   /**
    * Make the current parent's parent the new parent.
    */
	void				enterParent( void );	

   /**
    * @return Current childs name is returned.
    */
	const char*	   getCurrentName( void )			{ return mCurrentChild->getName(); }

   /**
    * @return Current childs number of total calls is returned.
    */
	int				getCurrentTotalCalls( void )	{ return mCurrentChild->getTotalCalls(); }

   /**
    * @return Current childs total exectuion time is returned.
    */
	float				getCurrentTotalTime( void )	{ return mCurrentChild->getTotalTime(); }

   /**
    * @return Current child's parent name is returned.
    */
	const char*	getCurrentParentName( void )			{ return mCurrentParent->getName(); }

   /**
    * @return Current child's parent number of total calls is returned.
    */
	int				getCurrentParentTotalCalls( void )	{ return mCurrentParent->getTotalCalls(); }

   /**
    * @return Current child's parent total execution time is returned.
    */
	float				getCurrentParentTotalTime( void )	{ return mCurrentParent->getTotalTime(); }

protected:

	ProfileNode*	mCurrentParent;
	ProfileNode*	mCurrentChild;

	ProfileIterator( ProfileNode* start );
	friend	class		ProfileManager;
};

/**
 * Overloaded output operator for outputting the current statistics
 */
VPR_API(std::ostream&) operator<< (std::ostream& out, ProfileIterator& iter);

} // end vpr namespace

#endif
