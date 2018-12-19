using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkBehaviour : PlayableBehaviour
    {
        public RemoteTalkClip clip;
        public RemoteTalkTrack track;
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
            else
            {
                talk.Stop();
                clip.UpdateCachedAsset();
            }
        }
    }
}
