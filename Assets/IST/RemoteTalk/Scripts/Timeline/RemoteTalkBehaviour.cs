using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkBehaviour : PlayableBehaviour
    {
        public RemoteTalkTrack track;
        public Talk talk = new Talk();
        public AudioSource audioSource;
        public AudioClip audioClip;

        public override void OnBehaviourPlay(Playable playable, FrameData info)
        {
            talk.Play();
        }

        public override void OnBehaviourPause(Playable playable, FrameData info)
        {
            //talk.Stop();
        }
    }
}
