using UnityEngine;
using System.Linq;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [TrackColor(0.5179334f, 0.7978405f, 0.9716981f)]
    [TrackClipType(typeof(RemoteTalkClip))]
    public class RemoteTalkTrack : TrackAsset
    {
        public bool fitDuration = true;

        public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount)
        {
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
            behaviour.director = go.GetComponent<PlayableDirector>();
            behaviour.track = this;
            behaviour.clip = clip;

            var rtc = (RemoteTalkClip)clip.asset;
            clip.displayName = rtc.talk.text + "_" + rtc.talk.castName;
            rtc.UpdateCachedAsset();

            return ret;
        }
    }
}
