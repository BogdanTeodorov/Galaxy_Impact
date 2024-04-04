
#include "MusicPlayer.h"
#include <stdexcept>


MusicPlayer::MusicPlayer() {
    m_filenames["menuTheme"] = "../assets/Music/main-menu-song.mp3";
    m_filenames["levelTheme"] = "../assets/Music/Level_music.ogg";
    m_filenames["bossTheme"] = "../assets/Music/Boss_Battle.ogg";
    m_filenames["victory_song"] = "../assets/Music/Victory_Tune.ogg";
    m_filenames["defeat_song"] = "../assets/Music/Defeat_Tune.ogg";
}

void MusicPlayer::addSong(const std::string&name, const std::string&path) {
    m_filenames[name] = path;
}

MusicPlayer& MusicPlayer::getInstance() {
    static MusicPlayer instance; // Meyers Singleton implementation
    return instance;
}


void MusicPlayer::play(String theme) {
    if (!m_music.openFromFile(m_filenames[theme]))
        throw std::runtime_error("Music could not open file");

    m_music.setVolume(m_volume);
    m_music.setLoop(true);
    m_music.play();
}


void MusicPlayer::stop() {
    m_music.stop();
    
}


void MusicPlayer::setPaused(bool paused) {
    if (paused)
        m_music.pause();
    else
        m_music.play();
}


void MusicPlayer::setVolume(float volume) {
    m_volume = volume;
    m_music.setVolume(m_volume);
}

void MusicPlayer::setLoop(bool inLoop)
{
    if (inLoop) {
        m_music.setLoop(true);
    }
    else {
        m_music.setLoop(false);
    }
}
