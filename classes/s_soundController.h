class s_soundController
{
    public:
        /*** Member variables ***/
        std::string id; //The identifying string of the sound controller
        float volume; //The volume at which to play sounds associated with this controller. 1 is 100% volume, while 0 is 0% volume.


        /*** Constructors ***/
        //Default
        s_soundController()
        {
            id = "default";
            volume = 1;
        }


        //Parametric
        s_soundController(  std::string idParam,
                            float volumeParam    )
        {
            id = idParam;
            volume = volumeParam;
        }


        /*** Methods ***/

    
    private:
};