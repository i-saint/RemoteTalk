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
#if UNITY_2018_1_OR_NEWER
using UnityEditor.Timeline;
#endif
#endif

namespace IST.RemoteTalk
{
    public static partial class Misc
    {
#if UNITY_EDITOR
        public static TimelineAsset currentTimeline
        {
            get
            {
#if UNITY_2018_1_OR_NEWER
                return TimelineEditor.inspectedAsset;
#else
                var director = currentDirector;
                if (director != null)
                    return director.playableAsset as TimelineAsset;
                return null;
#endif
            }
        }
        public static PlayableDirector currentDirector
        {
            get
            {
#if UNITY_2018_1_OR_NEWER
                return TimelineEditor.inspectedDirector;
#else
                var go = Selection.activeGameObject;
                if (go != null)
                    return go.GetComponent<PlayableDirector>();
                return null;
#endif
            }
        }
#endif

        public static T FindOutput<T>(Playable h, bool recursive = false) where T : PlayableBehaviour, new()
        {
            if (!h.IsValid())
                return null;

            for (int port = 0; port < h.GetOutputCount(); ++port)
            {
                var playable = h.GetOutput(port);
                if (playable.IsValid())
                {
                    var type = playable.GetPlayableType();
                    if (type == typeof(T))
                        return ((ScriptPlayable<T>)playable).GetBehaviour();
                    else if (recursive)
                        return FindOutput<T>(playable, recursive);
                }
            }
            return null;
        }
        public static T FindOutput<T>(PlayableGraph graph, bool recursive = false) where T : PlayableBehaviour, new()
        {
            int outputs = graph.GetRootPlayableCount();
            for (int i = 0; i < outputs; i++)
            {
                var found = FindOutput<T>(graph.GetRootPlayable(i), recursive);
                if (found != null)
                    return found;
            }
            return null;
        }

        public static T FindInput<T>(Playable h, bool recursive = false) where T : PlayableBehaviour, new()
        {
            if (!h.IsValid())
                return null;

            for (int port = 0; port < h.GetInputCount(); ++port)
            {
                var playable = h.GetInput(port);
                if (playable.IsValid())
                {
                    var type = playable.GetPlayableType();
                    if (type == typeof(T))
                        return ((ScriptPlayable<T>)playable).GetBehaviour();
                    else if (recursive)
                        return FindInput<T>(playable, recursive);
                }
            }
            return null;
        }
        public static T FindInput<T>(PlayableGraph graph, bool recursive = false) where T : PlayableBehaviour, new()
        {
            int outputs = graph.GetRootPlayableCount();
            for (int i = 0; i < outputs; i++)
            {
                var found = FindInput<T>(graph.GetRootPlayable(i), recursive);
                if (found != null)
                    return found;
            }
            return null;
        }


        public static void EnumerateTracks(TrackAsset track, Action<TrackAsset> act)
        {
            act(track);
            foreach (var child in track.GetChildTracks())
                EnumerateTracks(child, act);
        }

        public static void EnumerateTracks(TimelineAsset timeline, Action<TrackAsset> act)
        {
            foreach (var track in timeline.GetRootTracks())
                EnumerateTracks(track, act);
        }

        public static void EnumerateRemoteTalkTracks(TrackAsset track, Action<RemoteTalkTrack> act)
        {
            EnumerateTracks(track, t =>
            {
                var rtt = t as RemoteTalkTrack;
                if (rtt != null)
                    act(rtt);
            });
        }

        public static void EnumerateRemoteTalkTracks(TimelineAsset timeline, Action<RemoteTalkTrack> act)
        {
            EnumerateTracks(timeline, t => {
                var rtt = t as RemoteTalkTrack;
                if (rtt != null)
                    act(rtt);
            });
        }

        public static IEnumerable<TimelineClip> GetRemoteTalkClips(TimelineAsset timeline)
        {
            IEnumerable<TimelineClip> ret = null;
            Misc.EnumerateTracks(timeline, track =>
            {
                var rtt = track as RemoteTalkTrack;
                if (rtt != null)
                {
                    if (ret == null)
                        ret = rtt.GetClips();
                    else
                        ret = ret.Concat(rtt.GetClips());
                }
            });
            if (ret == null)
                ret = new List<TimelineClip>();
            return ret;
        }

        public static void RefreshTimelineWindow()
        {
#if UNITY_EDITOR
#if UNITY_2018_3_OR_NEWER
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);
#else
            // select GO that doesn't have PlayableDirector to clear timeline window
            var roots = SceneManager.GetActiveScene().GetRootGameObjects();
            foreach(var go in roots)
            {
                var director = go.GetComponent<PlayableDirector>();
                if (director == null)
                {
                    Selection.activeGameObject = go;
                    break;
                }
            }
#endif
#endif
        }

    }
}
#endif
