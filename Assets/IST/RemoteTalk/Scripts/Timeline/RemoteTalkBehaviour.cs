#if UNITY_2017_1_OR_NEWER
using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkBehaviour : PlayableBehaviour
    {
        public PlayableDirector director;
        public RemoteTalkTrack track;
        public TimelineClip clip;

        public Talk talk = new Talk();
        public AudioSource audioSource;
        public AudioClip audioClip;
        public RemoteTalkProvider remoteTalk;

        bool m_pending;
        bool m_exporting;


        public override void OnBehaviourPlay(Playable playable, FrameData info)
        {
            m_pending = true;
        }

        public override void OnBehaviourPause(Playable playable, FrameData info)
        {
            m_pending = false;

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
            if (output == null)
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
                        if (provider.exportAudio)
                        {
                            m_exporting = true;
                            track.OnTalk(this, info);
                        }
                    }
                }
            }
        }


        public void OnAudioClipImport(Talk t, AudioClip ac)
        {
            if (!m_exporting)
                return;
            m_exporting = false;

            var rtc = (RemoteTalkClip)clip.asset;
            if (rtc.UpdateCachedClip())
            {
                audioClip = rtc.audioClip.defaultValue as AudioClip;
                track.OnAudioClipUpdated(this);
            }
        }

#if UNITY_EDITOR
        public override void OnPlayableCreate(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport += OnAudioClipImport;
        }

        public override void OnPlayableDestroy(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport -= OnAudioClipImport;
        }
#endif
    }
}
#endif
