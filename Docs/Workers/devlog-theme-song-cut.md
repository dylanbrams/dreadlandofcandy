# Handoff to GAME: cut the theme song (Dylan, Nov 15)

Dylan has cut the theme song from the game entirely; he wants more time with it. Its guitar is the 1928 McClintock recording (US copyright until 2029), so it must not ship.

Game changes (GAME owns these; the editor was open when this was written):
- `Source/MegaJam/Private/UI/JamMusic.cpp:9`: Credits plays `credits`, not `theme_song`. Remove `theme_song` from the cue list (line 28) and its special cases (lines 34, 39).
- `Source/MegaJam/Private/UI/JamScreens.cpp:262`: remove the credits line "Theme-song vocals: Dylan".
- `Source/MegaJam/Private/Tests/UiScreensTests.cpp:115-117`: the credits cue is now `credits`; drop the theme_song take/fallback checks.
- Delete `/Game/Audio/Music/M_theme_song_take1` in the editor (Content/Audio/Music is cooked, so an unused asset would still ship).
- `Scripts/import_audio.py`: drop `theme-song.mp3` from MUSIC.
- `Docs/Declared.md` 2026-11-13 theme song entry: note it was cut on Nov 15 and is not in the build.
- Then build, run the Jam tests, package, smoke and re-zip; check that the packaged credits play `credits`.

Devlog and trailer docs (`Docs/Devlog/`) are already updated: no theme song anywhere.
