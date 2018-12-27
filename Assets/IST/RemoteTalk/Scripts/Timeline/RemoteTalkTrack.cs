#if UNITY_2017_1_OR_NEWER
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.SceneManagement;
#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif

namespace IST.RemoteTalk
{
    [TrackColor(0.5179334f, 0.7978405f, 0.9716981f)]
    [TrackBindingType(typeof(AudioSource))]
    [TrackClipType(typeof(RemoteTalkClip))]
    public class RemoteTalkTrack : TrackAsset
    {
        public enum ArrangeTarget
        {
            None,
            OnlySelf,
            AllRemoteTalkTracks,
            AllTracks,
        }

        public bool fitDuration = true;
        public ArrangeTarget arrangeTarget = ArrangeTarget.AllTracks;
        public bool pauseWhenExport = true;
        bool m_resumeRequested;

        public PlayableDirector director { get; set; }

        public AudioSource audioSource
        {
            get
            {
                if (director != null)
                {
                    foreach (var output in outputs)
                    {
                        var ret = director.GetGenericBinding(output.sourceObject) as AudioSource;
                        if (ret != null)
                            return ret;
                    }
                }
                return null;
            }
            set
            {
                if (director != null)
                    director.SetGenericBinding(this, value);
            }
        }

        public TimelineClip AddClip(Talk talk)
        {
            var ret = CreateDefaultClip();
            ret.displayName = talk.text + "_" + talk.castName;
            var asset = ret.asset as RemoteTalkClip;
            asset.talk = talk;
            asset.audioClip.defaultValue = talk.audioClip;
            asset.UpdateCachedClip();
            if (!double.IsInfinity(asset.duration))
                ret.duration = asset.duration;
            return ret;
        }

        public List<TimelineClip> AddClips(IEnumerable<Talk> talks)
        {
            var ret = new List<TimelineClip>();
            foreach(var talk in talks)
                ret.Add(AddClip(talk));
            return ret;
        }

        public List<Talk> GetTalks()
        {
            var ret = new List<Talk>();
            foreach (var srcClip in GetClips())
                ret.Add(((RemoteTalkClip)srcClip.asset).talk);
            return ret;
        }

        public AudioTrack ConvertToAudioTrack()
        {
            var timeline = timelineAsset;
            var audioTrack = timeline.CreateTrack<AudioTrack>(null, name);

            var output = audioSource;
            if (output != null)
                director.SetGenericBinding(audioTrack, output);

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
            return audioTrack;
        }

        public void OnTalk(RemoteTalkBehaviour behaviour, FrameData info)
        {
            if (pauseWhenExport && info.evaluationType == FrameData.EvaluationType.Playback)
            {
                behaviour.director.Pause();
                m_resumeRequested = true;
            }
        }

        public void OnAudioClipUpdated(RemoteTalkBehaviour behaviour)
        {
            var clip = behaviour.clip;
            var rtc = (RemoteTalkClip)clip.asset;
            double prev = clip.duration;
            double duration = rtc.duration;
            double gap = duration - prev;

            if (fitDuration)
            {
                clip.duration = duration;
                ArrangeClips(clip.start, gap);
            }
            if (pauseWhenExport && m_resumeRequested)
            {
                director.time = clip.end;
                director.Resume();
                m_resumeRequested = false;
            }
        }

        public void ArrangeClips(double time, double gap)
        {
            if (arrangeTarget == ArrangeTarget.None)
                return;

            var tracks = new List<TrackAsset>();
            switch (arrangeTarget)
            {
                case ArrangeTarget.OnlySelf:
                    tracks.Add(this);
                    break;
                case ArrangeTarget.AllRemoteTalkTracks:
                    Misc.EnumerateTracks(timelineAsset, track=> {
                        var rtt = track as RemoteTalkTrack;
                        if (rtt != null)
                            tracks.Add(rtt);
                    });
                    break;
                case ArrangeTarget.AllTracks:
                    Misc.EnumerateTracks(timelineAsset, track => tracks.Add(track));
                    break;
                default:
                    break;
            }

            foreach(var track in tracks)
            {
                foreach (var clip in track.GetClips())
                {
                    if (clip.start > time)
                        clip.start = clip.start + gap;
                }
            }
        }


        public override Playable CreateTrackMixer(PlayableGraph graph, GameObject go, int inputCount)
        {
            director = go.GetComponent<PlayableDirector>();

            var playable = ScriptPlayable<RemoteTalkMixerBehaviour>.Create(graph, inputCount);
            var mixer = playable.GetBehaviour();
            return playable;
        }

        protected override Playable CreatePlayable(PlayableGraph graph, GameObject go, TimelineClip clip)
        {
            var rtc = (RemoteTalkClip)clip.asset;
            clip.displayName = rtc.talk.text + "_" + rtc.talk.castName;
            bool audipClipUpdated = rtc.UpdateCachedClip();

            var ret = base.CreatePlayable(graph, go, clip);
            var playable = (ScriptPlayable<RemoteTalkBehaviour>)ret;
            var behaviour = playable.GetBehaviour();
            behaviour.director = go.GetComponent<PlayableDirector>();
            behaviour.track = this;
            behaviour.clip = clip;
            behaviour.talk = rtc.talk;
            behaviour.audioClip = rtc.audioClip.Resolve(graph.GetResolver());
            if (audipClipUpdated)
                OnAudioClipUpdated(behaviour);
            return ret;
        }


        public class TextImportOptions
        {
            public bool parCastTrack = true;
            public double startTime = 0.5;
            public double interval = 0.5;
        }

#if UNITY_EDITOR
        public static bool ImportText(string path, TextImportOptions opt)
        {
            var timeline = TimelineEditor.inspectedAsset;
            var director = TimelineEditor.inspectedDirector;
            if (timeline == null || director == null)
            {
                timeline = ScriptableObject.CreateInstance<TimelineAsset>();
                AssetDatabase.CreateAsset(timeline, "Assets/RemoteTalkTimeline.asset");
                var go = new GameObject();
                go.name = "RemoteTalkTimeline";
                director = go.AddComponent<PlayableDirector>();
                director.playableAsset = timeline;

                Selection.activeGameObject = go;
            }
            return ImportText(path, opt, timeline, director);
        }

#endif
        public static bool ImportText(string path, TextImportOptions opt, TimelineAsset timeline, PlayableDirector director)
        {
            if (timeline == null || director == null)
                return false;
            var talks = RemoteTalkScript.TextFileToTalks(path);
            if (talks == null)
                return false;

            double time = opt.startTime;
            if (opt.parCastTrack)
            {
                var tracks = new Dictionary<string, RemoteTalkTrack>();
                foreach (var talk in talks)
                {
                    RemoteTalkTrack track = null;
                    if (!tracks.TryGetValue(talk.castName, out track))
                    {
                        track = timeline.CreateTrack<RemoteTalkTrack>(null, "RemoteTalk");
                        track.director = director;
                        track.name = talk.castName;
                        tracks[talk.castName] = track;

                        var audio = Misc.FindOrCreateGameObject(talk.castName + "_AudioSource");
                        track.audioSource = Misc.GetOrAddComponent<AudioSource>(audio);
                    }
                    var clip = track.AddClip(talk);
                    clip.start = time;
                    time += clip.duration + opt.interval;
                }
            }
            else
            {
                var track = timeline.CreateTrack<RemoteTalkTrack>(null, "RemoteTalk");
                track.director = director;
                track.name = "RemoteTalkTrack";

                var audio = Misc.FindOrCreateGameObject("RemoteTalkAudioSource");
                track.audioSource = Misc.GetOrAddComponent<AudioSource>(audio);

                foreach (var talk in talks)
                {
                    var clip = track.AddClip(talk);
                    clip.start = time;
                    time += clip.duration + opt.interval;
                }
            }

            Misc.RefreshTimelineWindow();
            return false;
        }


#if UNITY_EDITOR
        public static bool ExportText(string path)
        {
            return ExportText(path, TimelineEditor.inspectedAsset);
        }
#endif

        public static bool ExportText(string path, TimelineAsset timeline)
        {
            if (!timeline)
                return false;

            var talks = new List<Talk>();
            foreach (var clip in Misc.GetRemoteTalkClips(timeline).OrderBy(a => a.start))
                talks.Add(((RemoteTalkClip)clip.asset).talk);

            return RemoteTalkScript.TalksToTextFile(path, talks);
        }

    }
}
#endif
