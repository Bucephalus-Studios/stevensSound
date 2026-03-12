/**
 *  An object used in the stevensSound library which represents music and its associated data
 *  within code. Music uses Mix_Music for streaming playback.
 */

class Music
{
    public:
        /*** Member variables ***/
        std::string  name;       //The name of the music track
        std::string  type;       //The type of music (e.g., "music", "battle music")
        std::string controllerId; //The id of the sound controller of this music

        Mix_MusicData musicData;  //The music data with file path and Mix_Music handle


        /*** Constructors ***/
        Music()
        {
            name = "default";
            type = "default";
            controllerId = "default";
            musicData = Mix_MusicData();
        }


        Music(  std::string nameParam,
                std::string typeParam,
                std::string controllerIdParam   )
        {
            name = nameParam;
            type = typeParam;
            controllerId = controllerIdParam;
            musicData = Mix_MusicData();
        }


        /*** Methods ***/
        /**
         * @brief Validates that this Music object's Mix_Music handle is safe to use
         *
         * Checks that the Mix_Music pointer is not null and is valid for playback.
         * If validation fails, logs an error with the music's name for debugging.
         *
         * @return true if the music is valid and safe to play, false otherwise
         */
        bool isValid() const
        {
            if (musicData.music == nullptr)
            {
                ErrorHandler::setError(ErrorLevel::ERROR,
                    "Music handle is null for: " + name + " (" + musicData.filePath + ")",
                    "Music::isValid");
                return false;
            }
            return true;
        }

        /**
         * @brief Free the Mix_Music handle owned by this music data object
         *
         * WARNING: Should ONLY be called during program shutdown in cleanUp().
         * Do NOT call this during normal playback or playlist switching!
         */
        void freeMusic()
        {
            musicData.free();
        }


    private:
};
