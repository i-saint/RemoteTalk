using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [TrackColor(0.5179334f, 0.7978405f, 0.9716981f)]
    [TrackClipType(typeof(RemoteTalkClip))]
    public class RemoteTalkTrack : TrackAsset
    {
        public bool autoAdjustDuration = true;

        public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount)
        {
#if UNITY_EDITOR
            foreach (var c in GetClips())
            {
                var clip = (RemoteTalkClip)c.asset;
                c.displayName = clip.talk.text + "_" + clip.talk.castName;

                if(clip.UpdateCachedAsset())
                {
                    if (autoAdjustDuration)
                        c.duration = clip.duration;
                }
            }
#endif
            var ret = ScriptPlayable<RemoteTalkMixerBehaviour>.Create(graph, inputCount);
            var mixer = ret.GetBehaviour();
            mixer.director = go.GetComponent<PlayableDirector>();
            mixer.clips = GetClips();
            mixer.track = this;
            return ret;
        }

        protected override Playable CreatePlayable(PlayableGraph graph, GameObject go, TimelineClip clip)
        {
            var ret = base.CreatePlayable(graph, go, clip);
            var playable = (ScriptPlayable<RemoteTalkBehaviour>)ret;
            var behaviour = playable.GetBehaviour();
            behaviour.track = this;
            return ret;
        }
    }
}
