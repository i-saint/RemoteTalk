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


        public ClipCaps clipCaps
        {
            get { return ClipCaps.None; }
        }

        public override Playable CreatePlayable(PlayableGraph graph, GameObject owner)
        {
            var playable = ScriptPlayable<RemoteTalkBehaviour>.Create(graph, template);
            var clone = playable.GetBehaviour();
            clone.talk = talk;
            clone.audioSource = audioSource.Resolve(graph.GetResolver());
            clone.audioClip = audioClip.Resolve(graph.GetResolver());
            return playable;
        }
    }
}
