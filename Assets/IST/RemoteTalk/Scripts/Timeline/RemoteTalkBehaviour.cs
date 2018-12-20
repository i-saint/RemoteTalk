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

        public override void OnBehaviourPlay(Playable playable, FrameData info)
        {
            if (remoteTalk != null && audioClip != null)
                remoteTalk.Play(audioClip);
            else
                talk.Play();
        }

        public override void OnBehaviourPause(Playable playable, FrameData info)
        {
            if (remoteTalk != null && audioClip != null)
                remoteTalk.Stop();
            //else
            //    talk.Stop();
        }

        public void OnAudioClipImport(Talk t, AudioClip ac)
        {
            var rtc = (RemoteTalkClip)clip.asset;
            if (rtc.UpdateCachedAsset())
            {
                audioClip = rtc.audioClip.defaultValue as AudioClip;
                if (track.fitDuration)
                    clip.duration = rtc.duration;
            }
        }

        public override void OnPlayableCreate(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport += OnAudioClipImport;
        }

        public override void OnPlayableDestroy(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport -= OnAudioClipImport;
        }
    }
}
