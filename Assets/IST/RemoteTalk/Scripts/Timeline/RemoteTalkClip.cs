#if UNITY_2017_1_OR_NEWER
using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkClip : PlayableAsset, ITimelineClipAsset
    {
        // note: non-public template means unrecordable
        RemoteTalkBehaviour template = new RemoteTalkBehaviour();

        public Talk talk = new Talk();
        public ExposedReference<AudioClip> audioClip;


        public ClipCaps clipCaps
        {
            get { return ClipCaps.None; }
        }

        public override Playable CreatePlayable(PlayableGraph graph, GameObject owner)
        {
            talk.ValidateParams();
            return ScriptPlayable<RemoteTalkBehaviour>.Create(graph, template);
        }


        public bool UpdateCachedClip()
        {
            var provider = talk.provider;
            if (provider != null)
            {
                var ac = provider.FindClip(talk);
                if (audioClip.defaultValue != ac)
                {
                    audioClip.defaultValue = ac;
                    return ac != null;
                }
            }
            return false;
        }

        public override double duration
        {
            get
            {
                var ac = audioClip.defaultValue as AudioClip;
                if (audioClip.defaultValue == null)
                    return base.duration;
                return (double)ac.samples / (double)ac.frequency;
            }
        }
    }
}
#endif
