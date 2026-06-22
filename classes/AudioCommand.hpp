#pragma once

#include <string>
#include "PlaylistSwitchOptions.hpp"

namespace stevensSound
{

enum class AudioCommandType
{
    SwitchPlaylist,
    Stop,
    SetMusicVolume,
};

struct AudioCommand
{
    AudioCommandType      type;
    std::string           playlistName; // SwitchPlaylist
    PlaylistSwitchOptions options;      // SwitchPlaylist
};

} // namespace stevensSound
