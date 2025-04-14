/**
 *  An object used in the stevensSound library which represents a specific sound and its associated data 
 *  within code.  
 */


class s_soundData
{
    public:
        /*** Member variables ***/
        std::string  name;       //The name of the sound - Will also be used as the key to reference this object in the sounds map
        const char * filePath;   //The path to the sound file from the main executable file's location
        std::string  type;       //The type of sound. This controls the location the sound will be stored in the sounds map.
        std::string controllerId; //The id of the sound controller of this sound.


        /*** Constructors ***/
        s_soundData()
        {
            name = "default";
            filePath = "no filepath defined for this s_soundData object";
            type = "default";
            controllerId = "default";
        }


        s_soundData(    std::string nameParam,
                        const char * filePathParam,
                        std::string typeParam,
                        std::string controllerIdParam   )
        {
            name = nameParam;
            filePath = filePathParam;
            type = typeParam;
            controllerId = controllerIdParam;
        }


        /*** Methods ***/


    private:
};