/**
 * Defines the s_soundPlaylist object, an object used to hold ordered lists of keys to soundData objects in the 
 * stevensSound library. 
 * 
 * Playlists are used for the playPlaylist() function in the stevensSound library, where a playlist can be played.
*/


class s_soundPlaylist
{
    public:
        /*** Member variables ***/
		std::string name; //The identifying name of the playlist
        std::vector< tuple<std::string,std::string> > sounds; //The keys of the sounds (tuple<categoryName,soundName>) and their order in the playlist.
        int index; //Index of the currently playing song;
        std::string controllerId; //The sound controller which this playlist is assigned to.
		std::string status; //The current status of the playlist. Possible values are: "stopped", "paused", and "playing"
		std::unordered_map<int, int> preTrackDelays; //The amount of time in milliseconds that we will wait in a delay before a track is played
		std::unordered_map<int, int> postTrackDelays; //The amount of time in milliseconds that we will wait in a delay after a track is played

        /*** Constructors ***/
        //Default
        s_soundPlaylist()
        {
			name = "unnamed playlist";
            sounds = {};
            index = 0;
            controllerId = "default";
			status = "stopped";
			preTrackDelays = {};
			postTrackDelays = {};
        }


        //Parametric
        /*
         * Given categories of sounds, and an order of tracks by name, create a vector of ordered s_soundData
		 * objects to represent the playlist you requested.
		 * 
		 * N.B. If you have two sounds with the same name in different categories, the sound in the category that
		 * 		is ordered first in soundCategories will be added to your playlist instead of the other instance.
		 * 
		 * Parameters:
         *  unordered_map<std::string, unordered_map<std::string, s_soundData> > & sourceSounds - The map of all sounds from the stevensSound library
         *                                                                                        we a creating a playlist from.
		 * 	std::vector<std::string> soundCategories - A vector of the sound categories you would like to pull tracks
		 * 											   from to create a playlist.
		 * 	std::vector<std::string> trackOrder - The names of the tracks from the given sound categories that we would
		 * 										  like to construct our playlist of.
		 * 	bool shuffleFill - If true, fills the the playlist after the specified track order with all sounds from
		 * 					   the given soundCategories in a random order.
         * 
         * Returns:
         *  A newly created playlist object
         */
        s_soundPlaylist(    const std::string & nameParam,
							unordered_map<std::string, unordered_map<std::string, s_soundData> > & sourceSounds,
                            std::vector<std::string> soundCategories,
                            std::vector<std::string> trackOrder,
                            std::string controllerIdParam,
							bool shuffleFill = false,
							std::unordered_map<int, int> preTrackDelaysParam = {},
							std::unordered_map<int, int> postTrackDelaysParam = {}   )
        {
            /*** Create the playlist ***/
			//Name the playlist
			name = nameParam;

            //Creates a new unordered map for our sounds we'd like to use  so we can erase pairs
			//for the purpose of tracking which sounds have already been used
			unordered_map<std::string, unordered_map<std::string,s_soundData> > soundsToAdd = {};
			for(int i = 0; i < soundCategories.size(); i++)
			{
				soundsToAdd[soundCategories[i]] = sourceSounds[soundCategories[i]];
			}

			//For every sound named in trackOrder, we look through our sound categories to find it
			bool foundTrack = false;
			for(int i = 0; i < trackOrder.size(); i++)
			{
				for(int k = 0; k < soundCategories.size(); k++)
				{
					//Can we find the requested track in this sound category?
					if(soundsToAdd[soundCategories[k]].contains(trackOrder[i]))
					{
						//Yes, we add it to the playlist
						sounds.push_back(make_tuple(soundCategories[k],trackOrder[i]));
						//Erase it from the soundsToAdd map
						soundsToAdd[soundCategories[k]].erase(trackOrder[i]);
						foundTrack = true;
						break;
					}
				}
				//Check to see if we found the track we were looking for. If not, we send an error to cerr.
				if(!foundTrack)
				{
					cerr << "stevensSound library error: createPlaylist() : Unable to find requested track '" + trackOrder[i] + "' sounds map.\n"; 
				}
				else
				{
					foundTrack = false;
				}
			}
            auto category_it = soundsToAdd.begin();
			//Now we check to see if we'd like to shuffle fill the rest of the playlist
			if(shuffleFill)
			{
				//Until we have no more sounds left to be added to the playlist, pick a random sound category and a random sound
				while(!soundsToAdd.empty())
				{
					//Get the random category of sound we'd like to use
					category_it = soundsToAdd.begin();
					std::advance(category_it, rand() % soundsToAdd.size());
					std::string random_category = category_it->first;
					//Get a random sound from the category we selected
					auto sound_it = soundsToAdd[random_category].begin();
					std::advance(sound_it, rand() % soundsToAdd[random_category].size());
					std::string random_sound = sound_it->first;
					//Add the sound to the playlist
					sounds.push_back(make_tuple(random_category,random_sound));
					//Erase the sound from the category
					soundsToAdd[random_category].erase(random_sound);
					//If the category is empty, erase it from the soundsToAdd unordered map
					if(soundsToAdd[random_category].empty())
					{
						soundsToAdd.erase(random_category);
					}
				}
			}

			//Assign the pre and post track delays
			preTrackDelays = preTrackDelaysParam;
			postTrackDelays = postTrackDelaysParam;

            /*** Initialize the index, assign the sound controller, and set the status to be "stopped" ***/
            index = 0;
            controllerId = controllerIdParam;
			status = "stopped";
        }


		/*** Operators ***/
		// // Thread-safe copy assignment (copies everything except mutex/cv)
		// s_soundPlaylist& operator=(const s_soundPlaylist& other)
		// {
		// 	if (this != &other)
		// 	{
		// 		// Copy to a temporary first (no locking needed)
		// 		s_soundPlaylist temp;
		// 		temp.status = other.status;
		// 		temp.name = other.name;
		// 		temp.sounds = other.sounds;
		// 		temp.index = other.index;
		// 		temp.controllerId = other.controllerId;
    	// 		temp.forceExit = other.forceExit;
		// 		temp.preTrackDelays = other.preTrackDelays;
		// 		temp.postTrackDelays = other.postTrackDelays;
				
		// 		// Then lock and move to destination
		// 		std::lock_guard<std::mutex> lock(this->mutex);
		// 		// Copy all safe members
		// 		this->status = temp.status;
		// 		this->name = temp.name;
		// 		this->sounds = temp.sounds;
		// 		this->index = temp.index;
		// 		this->controllerId = temp.controllerId;
    	// 		this->forceExit = temp.forceExit;
		// 		this->preTrackDelays = temp.preTrackDelays;
		// 		this->postTrackDelays = temp.postTrackDelays;
		// 		// Explicitly skip:
		// 		// this->mutex (keep original)
		// 		// this->cv (keep original)
		// 	}
		// 	return *this;
		// }


        /*** Methods ***/
        /**
         * Use the std::shuffle function from the algorithm header to shuffle the order
         * of sounds in a playlist.
         * 
         * Parameters:
         *  None
         * 
         * Returns:
         *  None, but shuffles the order of sounds in this playlist.
        */
        void shuffle()
        {
            std::shuffle(sounds.begin(), sounds.end(), default_random_engine());
        }


		/**
		 * @brief Get the name of this playlist
		 */
		std::string getName() const
		{
			return name;
		}


    private:
};