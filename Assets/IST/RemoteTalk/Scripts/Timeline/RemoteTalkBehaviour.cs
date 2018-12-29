#if UNITY_2017_1_OR_NEWER
using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkBehaviour : PlayableBehaviour
    {
        public PlayableDirector director;
        public RemoteTalkTrack track;
        public RemoteTalkMixerBehaviour mixer;
        public TimelineClip clip;
        public int clipHash;

        public Talk talk = new Talk();
        public AudioSource audioSource;
        public AudioClip audioClip;
        public RemoteTalkProvider remoteTalk;

        const double NullTime = -1;
        bool m_pending;
        bool m_exporting;
        double m_timePause = NullTime;


        public override void OnBehaviourPlay(Playable playable, FrameData info)
        {
            if (mixer == null)
            {
                mixer = Misc.FindOutput<RemoteTalkMixerBehaviour>(playable);
                director = mixer.director;
                track = mixer.track;
                clip = mixer.FindClip(clipHash);
            }

            if (clip != null)
            {
                var rtc = (RemoteTalkClip)clip.asset;
                bool audipClipUpdated = rtc.UpdateCachedClip(true);
                if (audipClipUpdated)
                {
#if UNITY_EDITOR
                    Undo.RecordObject(track, "RemoteTalk");
                    clip.displayName = rtc.GetDisplayName();
#endif
                    rtc.UpdateCachedClip();
                }
                audioClip = rtc.GetAudioClip();
                if (audipClipUpdated)
                    OnAudioClipUpdated();
            }

            m_pending = true;
        }

        public override void OnBehaviourPause(Playable playable, FrameData info)
        {
            m_pending = false;
            if (track == null)
                return;

            var output = track.audioSource;
            if (output != null)
            {
                if (audioClip != null)
                    output.Stop();
                //else
                //    talk.Stop();
            }
        }

        public override void ProcessFrame(Playable playable, FrameData info, object playerData)
        {
            if (!m_pending)
                return;

            var output = playerData as AudioSource;
            if (output == null || clip == null)
            {
                m_pending = false;
                return;
            }

            if (audioClip != null)
            {
                output.PlayOneShot(audioClip);
                m_pending = false;
            }
            else
            {
                var provider = talk.provider;
                if (provider != null)
                {
                    provider.output = output;
                    if (provider.Play(talk))
                    {
                        var rtc = (RemoteTalkClip)clip.asset;
                        rtc.audioClip.defaultValue = null;

                        m_pending = false;
#if UNITY_EDITOR
                        if (provider.exportAudio)
                        {
                            m_exporting = true;
                            if (RemoteTalkTrack.pauseWhenExport && info.evaluationType == FrameData.EvaluationType.Playback)
                            {
                                // note:
                                // on 2017.x, PlayableDirector.Pause() seems don't pause playback if not playing in editor.
                                // so, emulate pause by updating PlayableDirector.time.
                                m_timePause = director.time;
                                director.Pause();
                            }
                        }
#endif
                    }
                }
            }

#if UNITY_EDITOR
            if (m_exporting && m_timePause > NullTime)
            {
                if (info.evaluationType != FrameData.EvaluationType.Playback)
                    m_timePause = NullTime;
                if (m_timePause > NullTime)
                {
                    // see above note
                    director.time = m_timePause;
                }
            }
#endif
        }


#if UNITY_EDITOR
        public void OnTalkFinished(Talk t, bool succeeded)
        {
            if (!m_exporting || m_timePause == NullTime || !talk.Equals(t))
                return;

            if (!succeeded)
            {
                m_exporting = false;
                m_timePause = NullTime;
                director.Resume();
            }
        }

        public void OnAudioClipImport(Talk t, AudioClip ac)
        {
            if (!m_exporting || m_timePause == NullTime || !talk.Equals(t))
                return;

            m_exporting = false;
            var rtc = (RemoteTalkClip)clip.asset;
            if (rtc.UpdateCachedClip())
            {
                OnAudioClipUpdated();
            }

            // on 2017.x, director.time can be advanced by delta time
            const double MaxAllowdDelta = 0.2;
            if (Math.Abs(director.time - m_timePause) < MaxAllowdDelta)
            {
                m_timePause = NullTime;
                director.time = clip.end;
                director.Resume();
            }
        }
#endif

        public void OnAudioClipUpdated()
        {
            var rtc = (RemoteTalkClip)clip.asset;
            double prev = clip.duration;
            double duration = rtc.duration;
            double gap = duration - prev;

            if (RemoteTalkTrack.fitDuration && !double.IsInfinity(prev))
            {
#if UNITY_EDITOR
                Undo.RecordObject(track, "RemoteTalk");
#endif
                clip.duration = duration;
                track.ArrangeClips(clip.start, gap);
            }
        }

#if UNITY_EDITOR
        public override void OnPlayableCreate(Playable playable)
        {
            RemoteTalkProvider.onTalkFinish += OnTalkFinished;
            RemoteTalkProvider.onAudioClipImport += OnAudioClipImport;
        }

        public override void OnPlayableDestroy(Playable playable)
        {
            RemoteTalkProvider.onTalkFinish -= OnTalkFinished;
            RemoteTalkProvider.onAudioClipImport -= OnAudioClipImport;
        }
#endif
    }
}
#endif
