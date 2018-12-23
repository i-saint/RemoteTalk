#if UNITY_2017_1_OR_NEWER
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [TrackColor(0.5179334f, 0.7978405f, 0.9716981f)]
    [TrackBindingType(typeof(AudioSource))]
    [TrackClipType(typeof(RemoteTalkClip))]
    public class RemoteTalkTrack : TrackAsset
    {
        public bool fitDuration = true;
        public bool rearrange = true;
        public bool pauseWhenExport = true;


        public TimelineClip AddClip(Talk talk)
        {
            var ret = CreateDefaultClip();
            var asset = ret.asset as RemoteTalkClip;
            asset.talk = talk;
            asset.audioClip.defaultValue = talk.audioClip;
            return ret;
        }

        public List<TimelineClip> AddClips(IEnumerable<Talk> talks)
        {
            var ret = new List<TimelineClip>();
            foreach(var talk in talks)
                ret.Add(AddClip(talk));
            return ret;
        }

        public void ConvertToAudioTrack()
        {
            var timeline = timelineAsset;
            var audioTrack = timeline.CreateTrack<AudioTrack>(null, name);
            foreach (var srcClip in GetClips())
            {
                var srcAsset = (RemoteTalkClip)srcClip.asset;
                var ac = srcAsset.audioClip.defaultValue;
                if (ac == null)
                    continue;

                var dstClip = audioTrack.CreateClip((AudioClip)ac);
                dstClip.displayName = srcClip.displayName;
                dstClip.start = srcClip.start;
                dstClip.duration = srcClip.duration;
            }
        }

        public void OnTalk(RemoteTalkBehaviour behaviour)
        {
            if (pauseWhenExport)
                behaviour.director.Pause();
        }

        public void OnAudioClipUpdated(RemoteTalkBehaviour behaviour)
        {
            if (fitDuration)
            {
                var clip = behaviour.clip;
                var rtc = (RemoteTalkClip)clip.asset;
                double prev = clip.duration;
                clip.duration = rtc.duration;
                double gap = clip.duration - prev;

                if (rearrange)
                {
                    bool doArrange = false;
                    foreach(var c in GetClips())
                    {
                        if (doArrange)
                            c.start = c.start + gap;
                        else
                            doArrange = c == clip;
                    }
                }
            }
            if (pauseWhenExport)
                behaviour.director.Resume();
        }
        public bool ImportText(string path)
        {
            var talks = RemoteTalkScript.TextFileToTalks(path);
            if (talks != null)
            {
                AddClips(talks);
                return true;
            }
            return false;
        }

        public bool ExportText(string path)
        {
            var talks = new List<Talk>();
            foreach (var c in GetClips())
            {
                var talk = ((RemoteTalkClip)c.asset).talk;
                if (talks != null)
                    talks.Add(talk);
            }
            return RemoteTalkScript.TalksToTextFile(path, talks);
        }


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
            rtc.UpdateCachedClip();

            return ret;
        }
    }
}
#endif
