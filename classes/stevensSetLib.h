/**
 * stevensSetLib.h
 * 
 * Defines stevensSetLib.h, a library used for the purpose of expanding the ways you can interact with the standard set container!
 */

#pragma once

#include <set>


namespace stevensSetLib
{
	template<typename T> 
	T getRandomElement(	std::set<T> & mySet,
						bool removeSelectedElement = false	)
	/*
	** Randomly picks an item from a set to return, then removes that item from the set if directed.
	**
	** REQUIRES:
	** set
	*/
	{
		typename std::set<T>::iterator it; //allows us to iterate through the set
		int iterateAmount = rand()%mySet.size(); //the amount of times we want to iterate through the set
		int iterationCurrent = 0; //the current amount of iterations we have gone through
		for (it = mySet.begin(); iterationCurrent != iterateAmount; ++it) //iterate through the set until our iterator reaches the desired iteration count
		{
			iterationCurrent++;
		}

		T elementToReturn = *it;
		if(removeSelectedElement)
		{
			mySet.erase(it);
		}

		return elementToReturn; //once we are done with iterating through the set, we return the element that we selected
	}
};

