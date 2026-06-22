#pragma once

namespace stevensSound
{

/**
 * Options passed to switchMusicPlaylist() to control how the incoming playlist transitions in.
 */
struct PlaylistSwitchOptions
{
    int fadeInMs = 0; //If > 0, the incoming playlist's first track fades in over this many milliseconds
};

} // namespace stevensSound
