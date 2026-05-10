// Copyright 2026 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string>

#include <gtest/gtest.h>

#include "Common/FileUtil.h"
#include "UICommon/GameFile.h"

class GameFileTest : public testing::Test
{
protected:
  GameFileTest()
      : m_parent_directory(File::CreateTempDir()), m_disc_path(m_parent_directory + "/disc1.dol"),
        m_playlist_path(m_parent_directory + "/playlist.m3u")
  {
  }

  ~GameFileTest() override
  {
    if (!m_parent_directory.empty())
      File::DeleteDirRecursively(m_parent_directory);
  }

  void SetUp() override
  {
    ASSERT_FALSE(m_parent_directory.empty());
    ASSERT_TRUE(File::CreateEmptyFile(m_disc_path));
  }

  bool WritePlaylist(const std::string& body) const
  {
    return File::WriteStringToFile(m_playlist_path, body);
  }

  const std::string m_parent_directory;
  const std::string m_disc_path;
  const std::string m_playlist_path;
};

TEST_F(GameFileTest, M3UProxyUsesFirstExistingEntry)
{
  ASSERT_TRUE(WritePlaylist("#EXTM3U\nmissing.dol\ndisc1.dol\n"));

  const UICommon::GameFile disc_game(m_disc_path);
  ASSERT_TRUE(disc_game.IsValid());

  const UICommon::GameFile playlist_game(m_playlist_path);

#ifdef ANDROID
  EXPECT_TRUE(playlist_game.IsValid());
  EXPECT_EQ(playlist_game.GetFilePath(), m_playlist_path);
  EXPECT_EQ(playlist_game.GetGameID(), disc_game.GetGameID());
#else
  EXPECT_FALSE(playlist_game.IsValid());
#endif
}

TEST_F(GameFileTest, M3UProxySupportsUtf8BomAndComments)
{
  // UTF-8 BOM + comments should be ignored when selecting the proxy entry.
  ASSERT_TRUE(WritePlaylist("\xEF\xBB\xBF#EXTM3U\n#COMMENT\n./disc1.dol\n"));

  const UICommon::GameFile disc_game(m_disc_path);
  ASSERT_TRUE(disc_game.IsValid());

  const UICommon::GameFile playlist_game(m_playlist_path);

#ifdef ANDROID
  EXPECT_TRUE(playlist_game.IsValid());
  EXPECT_EQ(playlist_game.GetFilePath(), m_playlist_path);
  EXPECT_EQ(playlist_game.GetGameID(), disc_game.GetGameID());
#else
  EXPECT_FALSE(playlist_game.IsValid());
#endif
}

TEST_F(GameFileTest, M3UProxyStaysInvalidWithoutAnyExistingEntries)
{
  ASSERT_TRUE(WritePlaylist("#EXTM3U\nmissing_1.dol\nmissing_2.dol\n"));

  const UICommon::GameFile playlist_game(m_playlist_path);
  EXPECT_FALSE(playlist_game.IsValid());
}
