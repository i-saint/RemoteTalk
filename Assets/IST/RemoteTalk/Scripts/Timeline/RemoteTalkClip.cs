using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkClip : PlayableAsset, ITimelineClipAsset
    {
        public RemoteTalkBehaviour template = new RemoteTalkBehaviour();
        public Talk talk = new Talk();
        public ExposedReference<AudioSource> audioSource;
        public ExposedReference<AudioClip> audioClip;
        public ExposedReference<RemoteTalkProvider> remoteTalk;


        public ClipCaps clipCaps
        {
            get { return ClipCaps.None; }
        }

        public override Playable CreatePlayable(PlayableGraph graph, GameObject owner)
        {
            var playable = ScriptPlayable<RemoteTalkBehaviour>.Create(graph, template);
            var clone = playable.GetBehaviour();
            clone.clip = this;
            clone.talk = talk;
            clone.audioSource = audioSource.Resolve(graph.GetResolver());
            clone.audioClip = audioClip.Resolve(graph.GetResolver());
            clone.remoteTalk = remoteTalk.Resolve(graph.GetResolver());
            return playable;
        }


        public bool UpdateCachedAsset()
        {
            var provider = talk.provider;
            if (provider != null)
            {
                var ac = provider.FindClip(talk);
                audioClip.defaultValue = ac;
                remoteTalk.defaultValue = provider;
                return ac != null;
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
