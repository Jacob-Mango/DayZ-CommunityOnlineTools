# `VideoWidget`

Video playback in the UI. Supports play / pause / stop / seek / looping
and subtitle rendering.

## Class

```c
class VideoWidget extends Widget
{
    proto native bool Load(string name, bool looping = false, int startTime = 0);
    proto native void Unload();

    proto native bool Play();
    proto native bool Pause();
    proto native bool Stop();

    proto native bool SetTime(int time, bool preload);
    proto native int  GetTime();
    proto native int  GetTotalTime();

    proto native void SetLooping(bool looping);
    proto native bool IsLooping();

    proto native bool IsPlaying();
    proto native VideoState GetState();

    proto native void DisableSubtitles(bool disable);
    proto native bool IsSubtitlesDisabled();

    proto void SetCallback(VideoCallback cb, func fn);

    // Legacy API:
    int Play(VideoCommand cmd);
    bool LoadVideo(string name, int soundScene);
};

enum VideoState   { NONE, PLAYING, PAUSED, STOPPED, FINISHED };
enum VideoCallback {
    ON_PLAY, ON_PAUSE, ON_STOP, ON_END, ON_LOAD,
    ON_SEEK, ON_BUFFERING_START, ON_BUFFERING_END, ON_ERROR
};
```

## Layout

```
VideoWidgetClass intro_video {
 position 0 0
 size 1 1
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 font "gui/fonts/MetronLight14"   // font for subtitles
}
```

The `font` property is required if subtitles are to be displayed.

## Loading and playing

```c
m_Video.Load("dta/videos/intro.bik");
m_Video.Play();
```

Times are in milliseconds (`GetTime`, `SetTime`, `GetTotalTime`).

## Looping

```c
m_Video.SetLooping(true);   // restart from beginning on reach end
```

Or pass `looping = true` to `Load`.

## Subtitle file format

Subtitles are `.srt` files named `<videoname>_<lang>.srt` alongside the
video file:

- `intro.bik` — video
- `intro.srt` — English subtitles (default, no language code)
- `intro_de.srt` — German
- `intro_fr.srt` — French
- `intro_ja.srt` — Japanese

Engine picks based on player's language setting.

## Callbacks

```c
m_Video.SetCallback(VideoCallback.ON_END, this.OnVideoEnded);

void OnVideoEnded()
{
    // jump to main menu, etc.
}
```

## Supported formats

DayZ uses BIK (Bink) video. Convert with the RAD Game Tools encoders.
H.264 and other modern formats are **not** supported.

## Use cases in DayZ

- Loading screens (intro animations)
- Cinematic cutscenes (Namalsk events)
- HUD video feeds (rare; usually rendered via `RenderTargetWidget`)

COT doesn't use `VideoWidget`.

## Engine truth

> Defined at lines 543–632 of `scripts/1_core/proto/enwidgets.c`. The
> legacy `Play(VideoCommand)` API at the bottom of the class is kept for
> backwards compatibility — use the new explicit methods instead.
